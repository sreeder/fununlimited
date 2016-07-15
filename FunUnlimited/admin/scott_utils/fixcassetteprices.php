<?php
// FUNCTION: update music cassette pricing - set all prices to 5.99 where they are originally 7.99
include('../../include/include.inc');
check_scottc_loggedin();

die("DON'T RUN THIS!!!");

/*
$itemIDs = array();
$sql = "SELECT itm_itemID FROM items,prices WHERE itm_platformID=29 AND itm_itemID=prc_itemID AND ROUND(prc_used,2)=7.99";
$result = mysql_query($sql,$db);
while ($row = mysql_fetch_assoc($result)) { $itemIDs[] = $row['itm_itemID']; }

echo "Found ".count($itemIDs)." items with a price of 7.99<br />";

$sql = "UPDATE prices SET prc_used=ROUND(5.99,2) WHERE prc_itemID IN (".implode(',',$itemIDs).") AND ROUND(prc_used,2)=7.99";
mysql_query($sql,$db);

echo "Changed ".mysql_affected_rows()." prices to 5.99<br />";
*/

?>
done...