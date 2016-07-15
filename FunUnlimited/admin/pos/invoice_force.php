<?php
include('../../include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$type = (isset($_GET['type'])?$_GET['type']:@$_POST['type']);
$newused = @$_POST['newused'];

$resultIDX = (isset($_GET['resultIDX'])?$_GET['resultIDX']:@$_POST['resultIDX']);
if ($resultIDX > -1)
{
	$itemID = $_SESSION['search_results'][$resultIDX]['itm_itemID'];
	$invoiceIDX = -1;
}
else
{
	$itemID = (isset($_GET['itemID'])?$_GET['itemID']:@$_POST['itemID']);
	$invoiceIDX = (isset($_GET['invoiceIDX'])?$_GET['invoiceIDX']:@$_POST['invoiceIDX']);
}

$pg = new admin_page();
$error = new error('Update Item Quantity');

$title = 'Update Item Quantity';
$pg->setFull(NO);
$pg->setTitle($title);
$pg->head($title);

if ($act == "")
{
	// show new/used select form
	$itm = new items($pg);
	$itm->set_itemID($itemID);

	$pla = new platforms($pg,$itm->info['platformID']);

	?>
	<script type="text/javascript">function doforce(frm,nu) { frm.newused.value = nu; frm.submit(); }</script>
	<form method="post" action="/admin/pos/invoice_force.php" id="forcefrm">
		<input type="hidden" name="act" value="update">
		<input type="hidden" name="type" value="<?=$type;?>">
		<input type="hidden" name="resultIDX" value="<?=$resultIDX;?>">
		<input type="hidden" name="invoiceIDX" value="<?=$invoiceIDX;?>">
		<input type="hidden" name="itemID" value="<?=$itemID;?>">
		<input type="hidden" name="newused" value="">

		<b>Platform:</b> <?=$pla->platform_name();?><br />
		<b>Title:</b> <?=$itm->info['title'];?>
		<p />
		<?php
		if ($resultIDX > -1)
		{
			?>
			Please select whether you would like to add a new or used item to your inventory.
			<p />
			This will increase the new/used quantity by 1 and add the new/used item to your invoice.
			<p />
			<input type="button" value="Add New to Inventory &gt;" onclick="doforce(this.form,<?=ITEM_NEW;?>)" class="btn" />
			<input type="button" value="Add Used to Inventory &gt;" onclick="doforce(this.form,<?=ITEM_USED;?>)" class="btn" />
			<?php
		}
		else
		{
			// decide whether they should be asked to add a new or used item, according to which type is on the invoice right now
			$itminfo = array();
			$curidx = -1;
			while (list($a,$arr) = each($_SESSION['cust_items']))
			{
				if ($arr['ini_type'] == $type)
				{
					$curidx++;

					if ($curidx == $invoiceIDX) { $itminfo = $arr; break; }
				}
			}

			if (!count($itminfo)) { echo "Unable to pull information for the given item.<p />Please tell Scott!"; }
			else
			{
				if ($arr['ini_newused'] == ITEM_NEW) { $nu = ITEM_USED; $word = 'used'; } else { $nu = ITEM_NEW; $word = 'new'; }

				?>
				If you would like to add a <?=$word;?> item to your inventory for the above item, click the button below.
				<p />
				This will set the <?=$word;?> quantity to 1 and change the item on your invoice.
				<p />
				<input type="button" value="Add <?=ucwords($word);?> to Inventory &gt;" onclick="doforce(this.form,<?=$nu;?>)" class="btn" />
				<?php
			}
		}
		?>
	</form>
	<?php
}
elseif ($act == "update")
{
	// update the database and add the item to the invoice (or refresh the invoice)
	$field = ($newused==ITEM_NEW?'qty_new':'qty_used');
	$sql = "UPDATE quantity SET $field=($field+1) WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID=$itemID";
	mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);

	?>
	<script type="text/javascript">
		<?php
		if ($resultIDX > -1) { ?>window.opener.lookupifrm.location='/admin/pos/invoice_lookup.php?act=doadd&addIDXs=<?=$resultIDX;?>&add_newused=<?=$newused;?>&type=<?=$type;?>';<?php }
		else { ?>window.opener.lookupifrm.location='/admin/pos/invoice_lookup.php?act=dochange&invoiceIDX=<?=$invoiceIDX;?>&itemID=<?=$itemID;?>&type=<?=$type;?>&newused=<?=$newused;?>';<?php }
		?>
		window.close();
	</script>
	<?php
}

?><div style="width:98%;text-align:right;padding-bottom:1px"><input type="button" value="Close &gt;" class="btn" onclick="window.close()"></div><?php

$pg->foot();
?>