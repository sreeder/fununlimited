<?php
include('../../include/include.inc');

$pg = new admin_page();
$pg->setTitle('Fix UPC');
$pg->head('Fix UPC');

$error = new error('Fix UPC');

if (isset($_POST['origupc']) && isset($_POST['newupc']))
{
	$sql = "UPDATE items SET itm_upc='{$_POST['newupc']}' WHERE itm_upc='{$_POST['origupc']}'";
	mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);

	echo "UPC changed. ".mysql_affected_rows()." row(s) affected.<p />";
}

?>
<form method="post" action="fixupc.php" name="fixupc">
Original UPC: <input type="text" name="origupc" size="15" />
<p />
Fixed UPC: <input type="text" name="newupc" size="15" />
<p />
<input type="submit" value="Fix UPC &gt;" class="btn" />
</form>
<?php
$pg->addOnload('document.fixupc.origupc.focus()');

$pg->foot();
?>