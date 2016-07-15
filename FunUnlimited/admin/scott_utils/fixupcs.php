<?php
include('../../include/include.inc');
check_scottc_loggedin();

die("DON'T RUN THIS!!!");

/*
$sqls = array();

$sql = "SELECT itm_itemID,itm_upc FROM items WHERE LENGTH(itm_upc)<12 AND LENGTH(itm_upc)>0";
$result = mysql_query($sql,$db);

while ($row = mysql_fetch_assoc($result))
{
	$itemID = $row['itm_itemID'];
	$upc = $row['itm_upc'];
	$fixupc = str_pad($upc,12,'0',STR_PAD_LEFT);

	$sqls[] = "UPDATE items SET itm_upc='$fixupc' WHERE itm_itemID=$itemID";
}

while (list($a,$sql) = each($sqls))
{
	set_time_limit(30);
	mysql_query($sql,$db);
	if (mysql_errno()) { die(mysql_error()); }
}
*/

?>
done...