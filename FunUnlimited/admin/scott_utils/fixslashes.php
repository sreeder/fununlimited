<?php
include('../../include/include.inc');
check_scottc_loggedin();

echo "Fixing titles with slashes...";

$fix = array();
$sql = 'SELECT * FROM items WHERE itm_title REGEXP "[\\\]" ORDER BY itm_title';
$result = mysql_query($sql,$db);
while ($row = mysql_fetch_assoc($result))
{
	$title = $row['itm_title'];
	$last_title = $title;
	while (stripslashes($title) != $last_title) { $title = stripslashes($title); $last_title = $title; }
	$row['itm_title'] = $title;

	$fix[] = $row;
}

while (list($a,$arr) = each($fix))
{
	$sql = "UPDATE items SET itm_title='".mysql_real_escape_string($arr['itm_title'])."' WHERE itm_itemID={$arr['itm_itemID']}";
	mysql_query($sql,$db);
	if (mysql_errno()) { die(mysql_error()); }
}

echo "done. ".count($fix)." fixed";
?>
