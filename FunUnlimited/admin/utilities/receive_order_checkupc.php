<?php
include('../../include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$upc = @$_GET['upc'];
$newused = @$_GET['newused'];
$platformID = getG('platformID');

$pg = new admin_page();
$pg->setFull(NO);
$pg->head();

$error = new error('Receive Order UPC Check');

if (strlen($upc))
{
	// check that the UPC doesn't exist in any other platforms
	$sql = "SELECT itm_itemID,itm_platformID FROM items WHERE itm_upc='".mysql_real_escape_string($upc)."'";
	$result = mysql_query($sql,$db);
	if (mysql_errno()) { ?><script type="text/javascript">alert('MySQL error in receive_order UPC check - tell Scott!')</script><?php }
	$error->mysql(__FILE__,__LINE__);

	if (mysql_num_rows($result))
	{
		// the UPC was found - add call the parent add function
		$row = mysql_fetch_assoc($result);
		$itemID = $row['itm_itemID'];
		$platformID = $row['itm_platformID'];
		?>
		<script type="text/javascript">
			parent.add_item(<?php echo $itemID;?>,<?php echo $newused;?>,<?php echo $platformID;?>);
		</script>
		<?php
	}
	else
	{
		// the UPC doesn't exist - unlock the UPC text box
		$_SESSION['receive_newused'] = $newused;
		$_SESSION['receive_last_platformID'] = $platformID;
		?>
		<script type="text/javascript">
			parent.ask_addnew('<?php echo $upc;?>');
		</script>
		<?php
	}
}

$pg->foot();
?>