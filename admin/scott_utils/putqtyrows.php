<?php
include('../../include/include.inc');
check_scottc_loggedin();

$sqls = array();

$storeIDs = array();
$sql = "SELECT sto_storeID FROM stores";
$result = mysql_query($sql,$db);
while ($row = mysql_fetch_assoc($result)) { $storeIDs[] = $row['sto_storeID']; }

$putinstoreIDs = array(); // storeIDs with 0 rows in 'quantity'
while (list($a,$storeID) = each($storeIDs))
{
	$sql = "SELECT COUNT(*) AS count FROM quantity WHERE qty_storeID=$storeID";
	$result = mysql_query($sql,$db);
	$row = mysql_fetch_assoc($result);
	if (!$row['count']) { $putinstoreIDs[] = $storeID; }
}

if (count($putinstoreIDs))
{
	echo "Inserting quantity lines into stores: ".implode(', ',$putinstoreIDs).'<p />';

	$itemIDs = array();
	$sql = "SELECT itm_itemID FROM items ORDER BY itm_itemID";
	$result = mysql_query($sql,$db);
	while ($row = mysql_fetch_assoc($result)) { $itemIDs[] = $row['itm_itemID']; }

	$sqls = array();
	while (list($a,$storeID) = each($putinstoreIDs))
	{
		$vals = array();

		while (list($b,$itemID) = each($itemIDs))
		{
			$vals[] = "($storeID,$itemID,0,0)";
		}
		reset($itemIDs);
		
		$broken = array_chunk($vals,10000);
		
		while (list($a,$vals) = each($broken))
		{
			$sqls[] = "INSERT IGNORE INTO quantity VALUES ".implode(',',$vals);
		}
	}

	while (list($a,$sql) = each($sqls))
	{
		set_time_limit(60);
		mysql_query($sql,$db);
		if (mysql_errno()) { die('MySQL Error: '.mysql_error()); }
	}
}
else
{
	echo "There are no stores with empty quantities<p />";
}

?>
done...