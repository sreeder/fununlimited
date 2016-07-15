<?php
include('../../include/include.inc');
check_scottc_loggedin();

$sqls = array();

echo "Inserting 0 new, 0 used quantity where it's not set...<br />";flush();

// get storeIDs
$storeIDs = array();
$sql = "SELECT sto_storeID FROM stores";
$result = mysql_query($sql,$db);
while ($row = mysql_fetch_assoc($result)) { $storeIDs[] = $row['sto_storeID']; }
echo "StoreIDs: ".implode(', ',$storeIDs).'<br />';flush();

// get itemIDs
$itemIDs = array();
$sql = "SELECT itm_itemID FROM items WHERE itm_active=" . YES . " ORDER BY itm_itemID";
$result = mysql_query($sql,$db);
while ($row = mysql_fetch_assoc($result)) { $itemIDs[] = $row['itm_itemID']; }
echo "Found ".count($itemIDs)." itemIDs<br />";flush();

// build "VALUES ..." lines
$vals = array();
while (list($a,$itemID) = each($itemIDs)) { $vals[] = "(%storeID%,$itemID,0,0)"; }
$values = implode(',',$vals);

echo "Inserting rows into database...<br />";flush();
while (list($a,$storeID) = each($storeIDs))
{
	set_time_limit(60);
	$sql = "INSERT IGNORE INTO quantity VALUES ".str_replace('%storeID%',$storeID,$values);
	mysql_query($sql,$db);
	if (mysql_errno()) { die('MySQL Error: '.mysql_error()); }
	echo "Inserted ".mysql_affected_rows()." quantities into database for storeID $storeID<br />";flush();
}

?>
done...