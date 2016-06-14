<?php
include('../../include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$idx = @$_GET['idx'];
$itemID = @$_GET['itemID'];
$upc = trim(@$_GET['upc']);

$pg = new admin_page();
$pg->setFull(NO);
$pg->head();

$error = new error('Whole Platform Quantities UPC Set');

if (strlen($idx) && strlen($itemID))
{
	if (strlen($upc))
	{
		// check that the UPC doesn't exist in any other platforms
		$sql = "SELECT pla_name,itm_title FROM items,platforms WHERE itm_upc='".mysql_escape_string($upc)."' AND itm_platformID=pla_platformID AND itm_active=".YES;
		$result = mysql_query($sql,$db);
		if (mysql_errno()) { ?><script type="text/javascript">alert('MySQL error in begqty UPC set query 1 - tell Scott!')</script><?php }
		$error->mysql(__FILE__,__LINE__);
	}

	if (strlen($upc) && mysql_num_rows($result))
	{
		// it does exist in another platform - alert
		$row = mysql_fetch_assoc($result);
		$name = $row['pla_name'];
		$title = $row['itm_title'];
		?>
		<script type="text/javascript">
			parent.upc_exists('<?=$upc;?>','<?=mysql_escape_string($title);?>','<?=mysql_escape_string($name);?>');
			parent.unlock_upc();
		</script>
		<?php
	}
	else
	{
		// it doesn't exist in another platform - set the UPC and unlock the UPC text box
		$sql = "UPDATE items SET itm_upc='$upc' WHERE itm_itemID=$itemID";
		mysql_query($sql,$db);
		if (mysql_errno()) { ?><script type="text/javascript">alert('MySQL error in begqty UPC set query 2 - tell Scott!')</script><?php }
		$error->mysql(__FILE__,__LINE__);

		if (@$_GET['change_results'])
		{
			// search $_SESSION['search_results'] and change the UPC in the array
			while (list($a,$arr) = each($_SESSION['search_results']))
			{
				if ($arr['itm_itemID'] == $itemID) { $_SESSION['search_results'][$a]['itm_upc'] = $upc; break; }
			}
		}

		?>
		<script type="text/javascript">
			//alert('UPC set to <?=$upc;?> for item '+parent.titles[<?=$idx;?>]+'\n\nNote: The quantity has not been changed - please scan the item again if needed');
			parent.change_upc(<?=$idx;?>,'<?=$upc;?>');
			parent.unlock_upc();
		</script>
		<?php
	}
}

$pg->foot();
?>