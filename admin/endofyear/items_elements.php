<?php
// End of Year Item Quantities global arrays/functions

$years = array();
$yr = new years(); // pull in the years
while (list($a,$arr) = each($yr->years)) { $years[$arr['yer_yearID']] = $arr['yer_year']; }

/**
* Run the query with the pieces filled in and return the result reference
* @param integer $platformID
* @param string $select SELECT $select FROM ...
* @param string $limit LIMIT line [optional, default '']
* @return reference
*/
function runQuery($platformID,$select,$limit='')
{
	global $db,$error,$t;

	$sql = "SELECT $select FROM items,quantity WHERE itm_platformID=$platformID AND itm_active=" . YES . " AND itm_itemID=qty_itemID AND qty_storeID={$_SESSION['storeID']} ORDER BY itm_title $limit";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);

	return $result;
}
?>