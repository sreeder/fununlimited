<?php
/**
 * Utah Pawn file download - generate file
 * Created: 10/05/2012
 * Revised: 10/10/2012
 */

include('../../include/include.inc');

$cl = new check_login(STORE);

// generate a Utah Pawn upload file
$from = strtotime($_GET['fromdate']);
$to = strtotime($_GET['todate'] . ' 11:59:59pm');

$min = strtotime(date('m/d/Y', $to) . ' -6 months -1 day');

if ($from < $min)
{
	// no items found
	$pg = new admin_page();
	$pg->setTitle('Utah Pawn File Download');
	$pg->head('Utah Pawn File Download');

	echo '<p>ERROR: Do not select a date range larger than 6 months</p>';

	$pg->foot();
	die();
}

// get the trade items
$inv = new invoice($pg);
$items = $inv->getInvoiceItems(TRADE, $from, $to);

if (!$items)
{
	// no items found
	$pg = new admin_page();
	$pg->setTitle('Utah Pawn File Download');
	$pg->head('Utah Pawn File Download');

	echo '<p>ERROR: No trade records found in the selected date range</p>';

	$pg->foot();
	die();
}

// write the file
$f = fopen('trade_items.csv', 'w');

if (!$f)
{
	die('ERROR: Unable to open trade_items.csv! Please let Scott Carpenter know!');
}

foreach ($items as $arr)
{
	set_time_limit(30);

	// parse date of birth
	// it's sent through as either 00/00/0000 or MM/DD/YYYY if it's parseable
	$dob = '00/00/0000';

	if (preg_match('/([0-9]{1,2})\/([0-9]{1,2})\/([0-9]{2,4})/', $arr['cus_dob'], $matches))
	{
		$month = sprintf('%02d', $matches[1]);
		$day = sprintf('%02d', $matches[2]);
		$year = $matches[3];

		if (strlen($year) == 2)
		{
			if ($year <= date('y'))
			{
				$year = 2000 + $year;
			}
			else
			{
				$year = 1900 + $year;
			}
		}

		$dob = "$month/$day/$year";
	} // if MM/DD/YYYY match

	// parse height
	$height_feet = 0;
	$height_inches = 0;

	if (preg_match('/([0-9])\'([0-9]{1,2})/', $arr['cus_height'], $matches))
	{
		$height_feet = $matches[1];
		$height_inches = $matches[2];
	}

	// remove commas
	// they aren't allowed in the export
	foreach ($arr as $key => $value)
	{
		if (!is_array($value))
		{
			$arr[$key] = str_replace(',', ' ', $value);
			$arr[$key] = str_replace('  ', ' ', $arr[$key]);
		}
	}

	$line = array(
		// store info
		$_SESSION['store_info']['sto_name'],
		$_SESSION['store_info']['sto_address'],
		$_SESSION['store_info']['sto_city'],
		$_SESSION['store_info']['sto_state'],
		$_SESSION['store_info']['sto_zip'],

		// customer info
		trim($arr['emp_fname'] . ' ' . $arr['emp_lname'], ' '),
		$arr['cus_fname'],
		'', // customer middle name
		$arr['cus_lname'],
		$arr['cus_address'],
		$arr['cus_city'],
		$arr['cus_state'],
		$arr['cus_zip'],
		$arr['cus_phone'],
		$dob,
		($arr['cus_gender']==MALE ? 'M' : ($arr['cus_gender']==FEMALE ? 'F' : '?')),
		substr($arr['cus_ethnicity'], 0, 10),
		substr($arr['cus_hair_color'], 0, 10),
		substr($arr['cus_eye_color'], 0, 10),
		$height_feet,
		$height_inches,
		substr(intval($arr['cus_weight']), 0, 3),
		'', // SSN
		substr($arr['cus_idnumber'], 0, 20),
		'license',
		substr($arr['cus_idstate'], 0, 10),
		'', // employer
		'', // comments

		// ticket info
		$arr['inv_invoiceID'],
		'B', // 'B'uy 'L'oan 'S'ell 'R'enew
		date('m/d/Y', $arr['inv_completedtime']),
		'', // hold days
		'', // mature date
		'', // ticket amount

		// item info
		'', // NCIC code
		substr($arr['ini_platform_name'], 0, 100),
		substr($arr['itm_title'], 0, 100),
		substr(($arr['ini_serial_number'] ? $arr['ini_serial_number'] : 'Unknown'), 0, 100),
		substr($arr['itm_description'], 0, 2000),
		'', // owner ID
		'', // estimated value
		$arr['ini_qty'],
		$arr['ini_price'],

		// gun info
		'', // caliber
		'', // action
		'', // finish
		'', // engraving
		'', // gun type

		// jewelry info
		'', // karat
		'', // DWT
		'', // JPTS
		'', // stone type
		'', // stone count
		'', // metal type
		'', // stone cut
		'', // engraving
		'', // size length
		'', // description
	); // end $line

	fputcsv($f, $line);
} // foreach trade item

fclose($f);

// output the file for download
header('Content-Type: application/csv');
header('Content-Disposition: attachment; filename=pawn_upload_' . date('Ymd', $from) . '-' . date('Ymd', $to) . '.csv');
header('Pragma: no-cache');

readfile('trade_items.csv');

/* END OF FILE */
/* Location: ./admin/reports/pawn_download_generate.php */