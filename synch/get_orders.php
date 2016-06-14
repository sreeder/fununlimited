<?php
/*
* Fun Unlimited database synchronization utility
* Runs every morning at 2am
*
* This file handles the exporting of the latest online orders
*/
$force_production = true;
include('synch_settings.inc');

if (0)
{
	$read = file_get_contents('neworders.txt');
	echo $read;
	die();
}

$last_time = @$_GET['last_time'];
if (!$last_time) { $last_time = 0; }
//$last_time = strtotime('-1 months -11 days');
$locale = ONLINE; // ONLINE

// grab all online orders from the last_time
$sql = "SELECT * FROM invoices WHERE inv_locale={$locale} AND inv_completedtime>=$last_time";
$result = mysql_query($sql,$db);

if (mysql_num_rows($result))
{
	// output each invoice and its items
	// the table structure needs to be output before the data does!!!
	// the field delimeter is set in $delimeter
	$item_structure_shown = NO;

	// first, output the table structure
	echo getTableStructure('invoices',$result);

	// then, the info for each invoice and its items
	while ($row = mysql_fetch_assoc($result))
	{
		// output the data line
		echo getDataLine('invoices',$row,'inv_invoiceID','CREATEID');

		// get the items
		$isql = "SELECT * FROM invoice_items WHERE ini_invoiceID={$row['inv_invoiceID']}";
		$iresult = mysql_query($isql,$db);

		if (mysql_num_rows($iresult))
		{
			if (!$item_structure_shown)
			{
				echo getTableStructure('invoice_items',$iresult);
				$item_structure_shown = YES;
			}

			while ($irow = mysql_fetch_assoc($iresult))
			{
				echo getDataLine('invoice_items',$irow,'ini_invoiceID','LASTID');
			}
		}
	}
}

/**
* Output the structure for the given table and result set
* @param string $table
* @param object &$result
* @return string
* @access public
*/
function getTableStructure($table,&$result)
{
	global $delimeter;

	$structure = array('columns',$table);
	for ($i=0; $name=@mysql_field_name($result,$i); $i++)
	{
		$structure[] = $name;
	}

	return implode($delimeter,$structure)."<br />";
}

/**
* Output a data line
* @param string $table
* @param object $row
* @param string $id_field IE: ini_invoiceID
* @param string $id_replace IE: LASTID
* @return string
* @access public
*/
function getDataLine($table,$row,$id_field,$id_replace)
{
	global $delimeter;

	$row[$id_field] = $id_replace;
	return implode($delimeter,array_merge(array('data',$table),$row))."<br />";
}
?>