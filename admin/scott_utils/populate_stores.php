<?php
include('../../include/include.inc');
check_scottc_loggedin();

/*
This file removes ALL settings for ALL stores EXCEPT Fun Unlimited of Logan
and then copies Logan's settings to each store
Run this with caution!

NOTE: this does not populate quantities - run putqtyrows.php to do that...
*/

//die('DO NOT RUN THIS!!!');

// pull in the storeIDs
$storeIDs = array();
$sql = "SELECT sto_storeID FROM stores WHERE sto_storeID!=1";
$result = mysql_query($sql,$db);
while ($row = mysql_fetch_assoc($result)) { $storeIDs[] = $row['sto_storeID']; }

// uncomment the following line and put a storeIDs in an array to set up those stores
$storeIDs = array(10);

echo "Copying <b>Fun Unlimited of Logan</b> data to ".count($storeIDs)." stores (IDs: ".implode(', ',$storeIDs).")<p />";
?><p /><hr width="100%" size="-1" color="#000000" noshade="noshade" /><p /><?php

// pull in all tables
$exclude = array('customers','employees','invoices','orders','preorders','price_changes','quantity','quick_lookups','rankings','received_orders','return','stores','update_completed_stores','users','wishlists');
$tables = array();
$sql = "SHOW TABLES";
$result = mysql_query($sql,$db);
while ($row = mysql_fetch_row($result))
{
	if (!in_array($row[0],$exclude)) { $tables[] = $row[0]; }
}

while (list($a,$table) = each($tables))
{
	set_time_limit(60);

	// find storeID field
	echo "Searching table <b>$table</b> for storeID field...";

	$storeID_field = '';
	$sql = "DESC $table";
	$result = mysql_query($sql,$db);
	while ($row = mysql_fetch_row($result))
	{
		if (substr($row[0],-8) == '_storeID') { $storeID_field = $row[0]; break; }
	}

	if (strlen($storeID_field))
	{
		echo "found! ($storeID_field)<br />";

		// pull in all values from the table for storeID=1
		$values = array();
		$sql = "SELECT * FROM $table WHERE $storeID_field=1";
		$result = mysql_query($sql,$db);
		while ($row = mysql_fetch_assoc($result)) { $values[] = $row; }
		echo count($values)." row".(count($values)!=1?'s':'')." found.<br />";

		// build INSERT queries for each store
		echo "Populating INSERT queries for each store...";

		$sqls = array();
		while (list($b,$storeID) = each($storeIDs))
		{
			$sqls[] = "DELETE FROM $table WHERE $storeID_field=$storeID";
			while (list($c,$row) = each($values))
			{
				// build the INSERT line, replacing the storeID field's value with the current storeID
				$vals = array();
				$row[$storeID_field] = $storeID;
				while (list($k,$v) = each($row)) { $vals[] = "$k='".mysql_escape_string($v)."'"; }
				$sqls[] = "INSERT INTO $table SET ".implode(',',$vals);
			}
			reset($values);
		}
		reset($storeIDs);
		echo "done!<br />";

		echo "Performing ".count($sqls)." queries...";
		while (list($d,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			if (mysql_errno()) { die('MySQL Error: '.mysql_error()."<p />$sql"); }
		}
		echo "done!<br />";
	}
	else { echo "not found!<br />"; }
	?><p /><hr width="100%" size="-1" color="#000000" noshade="noshade" /><p /><?php
}

?>
done...