<?php
include('../../include/include.inc');
check_scottc_loggedin();

die("DON'T RUN THIS!!!");

/*
$sqls = array();

for ($i=12; $i>0; $i--)
{
	$zeros = str_repeat('0',$i);
	$sql = "SELECT itm_itemID,itm_upc FROM items WHERE itm_upc LIKE '%$zeros'";
	$result = mysql_query($sql,$db);

	while ($row = mysql_fetch_assoc($result))
	{
		$itemID = $row['itm_itemID'];
		$upc = $row['itm_upc'];
		$fixupc = $zeros.str_replace($zeros,'',$upc);

		$sqls[] = "UPDATE items SET itm_upc='$fixupc' WHERE itm_itemID=$itemID";
	}
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