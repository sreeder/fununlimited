<?php
include('../../include/include.inc');

$itemID = @$_GET['itemID'];
$notes = @$_GET['notes'];

$pg = new admin_page();
$pg->setFull(NO);
$pg->setCenter(NO);
$pg->colors['body'] = $pg->color('table-cell');
$pg->head();

$error = new error('Item Information - Note Set');

if ($itemID)
{
	$sql = "UPDATE items SET itm_notes='".mysql_escape_string($notes)."' WHERE itm_itemID=$itemID";
	mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);
	?>
	<table border="0" cellspacing="0" cellpadding="0"><tr><td height="20" valign="middle">&nbsp;&nbsp;&nbsp;<b>Status:</b> Notes updated</td></tr></table>
	<?php
}

$pg->foot();
?>