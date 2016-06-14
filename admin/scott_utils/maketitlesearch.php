<?php
// FUNCTION: create/populate the itm_title_search field in the items table
// the value of the field is the value of itm_title with everything but letters/numbers removed (including whitespace)

include('../../include/include.inc');
check_scottc_loggedin();

//die("DON'T RUN THIS!!!");

$sqls = array();

// get the item titles
$total = 0;
$sql = "SELECT itm_itemID,itm_title,itm_title_search FROM items ORDER BY itm_title";
$result = mysql_query($sql,$db);
if (mysql_errno()) { die(mysql_error()); }
while ($row = mysql_fetch_assoc($result))
{
	if ($row['itm_title_search'] != format_title($row['itm_title']))
	{
		$total++;
		$sqls[] = "UPDATE items SET itm_title_search='".format_title($row['itm_title'])."' WHERE itm_itemID={$row['itm_itemID']}";
	}
}

while (list($a,$sql) = each($sqls))
{
	set_time_limit(30);
	//echo "$sql<br />";
	mysql_query($sql,$db);
	if (mysql_errno() && $a) { die(mysql_error()); }
}

// remove everything but letters/numbers
function format_title($title)
{
	return strtolower(preg_replace('/[^a-zA-Z0-9]/','',$title));
}

?>
<p /><hr /><p />set itm_title_search for <?=$total;?> items