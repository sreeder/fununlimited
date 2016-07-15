<?php
/**
 * Fix ethnicity/eye color/hair color values to match the state pawn values
 * Created: 10/08/2012
 * Revised: 10/08/2012
 */

include('../../include/include.inc');

$pg = new admin_page();
$cust = new customers($pg);

$replace = array(
	'ethnicity' => array(
		"'" => 'U',
		'+++++++++++++++' => 'U',
		'-' => 'U',
		'--' => 'U',
		'.' => 'U',
		'..' => 'U',
		'...' => 'U',
		'....' => 'U',
		'/' => 'U',
		'//' => 'U',
		'///' => 'U',
		'0' => 'U',
		'00' => 'U',
		'1' => 'U',
		'11' => 'U',
		'111' => 'U',
		'1`' => 'U',
		'2' => 'U',
		'6' => 'U',
		'?' => 'U',
		'?:' => 'U',
		'??' => 'U',
		'???' => 'U',
		'?Dark..er' => 'U',
		'Asian?' => 'U',
		'Asian??' => 'U',
		'Asian???' => 'U',
		'AWSOME!' => 'U',
		'Beige' => 'U',
		'bl/w just because' => 'U',
		'bl? hisp?' => 'U',
		'Black/Caucasion' => 'U',
		'Black??' => 'U',
		'Blue' => 'U',
		'cfghfgh' => 'U',
		'Chamorro' => 'U',
		'clear' => 'U',
		'colorful' => 'U',
		'crazy' => 'U',
		'D' => 'U',
		'D-Bag' => 'U',
		'dark' => 'U',
		'dark caucasian?' => 'U',
		'darker than me' => 'U',
		'Darkish' => 'U',
		'darkish caucasian' => 'U',
		'DB' => 'U',
		'Deuce-Bags' => 'U',
		'Douchebag' => 'U',
		'Female Dog' => 'U',
		'f' => 'U',
		'fh' => 'U',
		'h 55' => 'U',
		'Hisp/Blk?' => 'U',
		'Hisp?' => 'U',
		'hisp? asian? fiik' => 'U',
		'i un know' => 'U',
		'Indian?' => 'U',
		'kjh' => 'U',
		'Mexican?' => 'U',
		'MHL' => 'U',
		'MILITARY ID' => 'U',
		'Mixed' => 'U',
		'not sure' => 'U',
		'NW' => 'U',
		'O' => 'U',
		'or' => 'U',
		'Other' => 'U',
		'Pac. Islander? Hisp? wtf?' => 'U',
		'Polynesian???' => 'U',
		'purple' => 'U',
		'q' => 'U',
		'qq' => 'U',
		'refused' => 'U',
		'S' => 'U',
		'some dark skined race' => 'U',
		'Something, not White' => 'U',
		'transparent' => 'U',
		'tyri' => 'U',
		'w/m' => 'U',
		'w?' => 'U',
		'w? hisp?' => 'U',
		'we' => 'U',
		'white?' => 'U',
		'who the hell cares' => 'U',
		'WWWW' => 'U',
		'x' => 'U',
		'zz' => 'U',

		'Aa' => 'B',
		'af' => 'B',
		'Af. Am.' => 'B',
		'African' => 'B',
		'African A' => 'B',
		'aFRICAN AMERICAN' => 'B',
		'Alaskan Native' => 'I',
		'Arab' => 'W',
		'Arab (O noes! He\'s got a' => 'W',
		'Arabic' => 'W',
		'as' => 'A',
		'Asain' => 'A',
		'Asia/Pacific Islander?' => 'U',
		'Asian/white' => 'A',
		'asin' => 'A',
		'bl' => 'B',
		'Black Nigga' => 'B',
		'Blackinja' => 'B',
		'Blk' => 'B',
		'bro' => 'H',
		'brown' => 'H',
		'c' => 'W',
		'C aucasion' => 'W',
		'ca' => 'W',
		'cacaisain' => 'W',
		'Cacaucasion' => 'W',
		'Cacausion' => 'W',
		'cacu' => 'W',
		'Cambodian' => 'A',
		'Cauc' => 'W',
		'Caucaian' => 'W',
		'caucaisin' => 'W',
		'Caucasian' => 'W',
		'caucasin' => 'W',
		'Caucasiom' => 'W',
		'Caucasion' => 'W',
		'Caucasion/Hispanic' => 'H',
		'Caucasoin' => 'W',
		'Caucsaion' => 'W',
		'Causasion' => 'W',
		'Causcasion' => 'W',
		'Chinese' => 'A',
		'colored' => 'B',
		'colorful-indianish' => 'I',
		'Cuacasian' => 'W',
		'Cuacasion' => 'W',
		'Dominican' => 'U',
		'Ehite' => 'W',
		'Hawaiian' => 'P',
		'Hespanic' => 'H',
		'hipanic' => 'H',
		'his' => 'H',
		'Hisp' => 'H',
		'hisp/w' => 'H',
		'Hispan ICK' => 'H',
		'HisPANIC!' => 'H',
		'Hispanic/White' => 'H',
		'Hispanickkkkkikikkiccc!!!' => 'H',
		'hist' => 'H',
		'Ind' => 'I',
		'Indian' => 'I',
		'Indian/Caucasion' => 'I',
		'j' => 'A',
		'Jap' => 'A',
		'Japanese' => 'A',
		'Japanese/Caucasion' => 'A',
		'Latin' => 'H',
		'Latino' => 'H',
		'M' => 'H',
		'mex' => 'H',
		'Mexican' => 'H',
		'Middle East' => 'W',
		'Middle Eastern' => 'W',
		'Middle-Eastern' => 'W',
		'Milato' => 'B',
		'n' => 'I',
		'na' => 'I',
		'Native America' => 'I',
		'Native American' => 'I',
		'Navajo' => 'I',
		'Of Color' => 'B',
		'Pacific Island' => 'P',
		'Persian' => 'A',
		'Philipino' => 'A',
		'phillipino' => 'A',
		'PI' => 'P',
		'Polynesian' => 'P',
		'puerto rico' => 'H',
		'redneck' => 'W',
		'samoan' => 'P',
		'sOMONA' => 'P',
		'sp' => 'H',
		'Spanish' => 'H',
		'Stupid (White)' => 'W',
		'Tongan' => 'P',
		'W Douche' => 'W',
		'Whie' => 'W',
		'White (D-Bag)' => 'W',
		'White (Fat)' => 'W',
		'White (Old)' => 'W',
		'white q' => 'W',
		'white/darker' => 'W',
		'White/Hisp' => 'H',
		'white/hispanic' => 'H',
		'white/Phillipino' => 'A',
		'whiter' => 'W',
		'whitish' => 'W',
		'Whitwe' => 'W',
		'Wht' => 'W',
		'Whte' => 'W',
		'Whtie' => 'W',
		'wt' => 'W',
                'black' => 'B',
                'blk? hisp? wtf' => 'U',
                'blue' => 'U',
                'cocaine' => 'U',
                'jamaholapican' => 'U',
                'white' => 'W',
	),
	'hair_color' => array(
		'blonde' => 'BLN',
                '-' => 'XXX',
                '1' => 'XXX',
                '55' => 'XXX',
                '?' => 'XXX',
                'B' => 'BLN',
                'BLD' => 'BLN',
                'BLO' => 'BLN',
                'BLOND' => 'BLN',
                'BRN' => 'BRO',
                'BROWM' => 'BRO',
                'BRUNETTE' => 'BRO',
                'GREY' => 'XXX',
                'MOO' => 'XXX',
                'NONE' => 'XXX',
                'MUL' => 'XXX',
	),
	'eye_color' => array(
                '-' => 'XXX',
                '1' => 'XXX',
                'B' => 'BLU',
                'BLUE' => 'BLU',
                'BRN' => 'BRO',
                'COW' => 'XXX',
                'GREEN' => 'GRN',
                'GREN' => 'GRN',
                'POO' => 'XXX',
                'STUPID' => 'XXX',
	),
);

$unknown = array();

if (!isset($_SESSION['fixed']))
{
	$_SESSION['fixed'] = array();
}

?>
<table>
	<tr>
		<th>Field</th>
		<th>From</th>
		<th>To</th>
		<th>Affected</th>
	</tr>
	<?php

	foreach ($cust->lookup_tables as $field => $values)
	{
		foreach ($values as $to => $from)
		{
			$replace[$field][$from] = $to;
		}

		foreach ($replace[$field] as $from => $to)
		{
			if (!strlen($to) || isset($_SESSION['fixed'][$from]))
			{
                                continue;
			}

			set_time_limit(60);

			$sql = "
				UPDATE
					customers
				SET
					cus_$field='" . mysql_real_escape_string($to) . "'
				WHERE
					cus_$field='" . mysql_real_escape_string($from) . "'
			";
			mysql_query($sql, $db);
			$affected = mysql_affected_rows($db);

			if (!$affected)
			{
				$_SESSION['fixed'][$from] = true;
			}

			?>
			<tr>
				<td><?php echo $field;?></td>
				<td><?php echo $from;?></td>
				<td><?php echo $to;?></td>
				<td align="right"><?php echo $affected;?></td>
			</tr>
			<?php
		} // foreach value

		// fix case
		$sql = "
			UPDATE
				customers
			SET
				cus_$field=UPPER(cus_$field)
			WHERE
				cus_$field!=''
		";
		mysql_query($sql, $db);

		// get unknown values
		$sql = "
			SELECT
				DISTINCT cus_$field
			FROM
				customers
			WHERE
				cus_$field!=''
				AND cus_$field NOT IN " . getIn(array_keys($cust->lookup_tables[$field])) . "
			ORDER BY
				cus_$field
		";
		$result = mysql_query($sql, $db);

		while ($row = mysql_fetch_assoc($result))
		{
			$unknown[$field][] = $row["cus_$field"];
		}
	} // foreach lookup table

	?>
</table>

<hr />

<p>
	<b>Unknown values:</b>
</p>
<?php

foreach ($unknown as $type => $values)
{
	echo "<p><b>$type</b></p>";

	$output = array();

	foreach ($values as $value)
	{
		$output[$value] = '';
	}

	echo '<pre>';
	var_export($output);
	echo '</pre>';
}

/* END OF FILE */
/* Location: ./admin/scott_utils/fix_pawn_values.php */
