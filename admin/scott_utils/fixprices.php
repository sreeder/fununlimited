<?php
include('../../include/include.inc');
check_scottc_loggedin();

die("DON'T RUN THIS!!!");

/*
$sqls = array();

for ($i=1; $i<151; $i++)
{
	$sqls[] = "UPDATE prices SET prc_new=".($i-0.01)." WHERE prc_new=$i";
	$sqls[] = "UPDATE prices SET prc_used=".($i-0.01)." WHERE prc_used=$i";
}

while (list($a,$sql) = each($sqls))
{
	set_time_limit(30);
	//echo "$sql<br />";
	mysql_query($sql,$db);
	if (mysql_errno()) { die(mysql_error()); }
}
*/

?>
done...