<?php
include('../../include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$type = (isset($_GET['type'])?$_GET['type']:@$_POST['type']);
$resultIDX = (isset($_GET['resultIDX'])?$_GET['resultIDX']:@$_POST['resultIDX']);

$pg = new admin_page();
$error = new error('Returned Items Lookup Results');

$pg->setFull(NO);
$pg->setTitle('Returned Items Lookup Results');
$pg->head('Returned Items Lookup Results');

if ($act == "")
{
	// see if the item is in the customer's purchase history/ask for purchase date
	$results = $_SESSION['search_results'];
	$itemID = @$results[$resultIDX]['itm_itemID'];
	$invoice_items = array();

	if ($itemID)
	{
		$itm = new items($pg);
		$itm->set_itemID($itemID);
		?><b>Selected Item:</b> <?=$itm->info['name'];?> - <?=$itm->info['title'];?><p /><?php

		$sql = "SELECT * FROM invoices,invoice_items WHERE inv_customerID={$_SESSION['customerID']} AND inv_completed=" . YES . " AND inv_invoiceID=ini_invoiceID AND ini_type=".SALE." AND ini_itemID=$itemID ORDER BY ini_timeadded";
		$result = mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result)) { $invoice_items[] = $row; }

		?>
		<script type="text/javascript">
			function verify(frm)
			{
				if (frm.sel_purchdateprice.length == undefined || frm.sel_purchdateprice[0].checked == true)
				{
					if (!validDate(frm.purchdate.value)) { alert('Please enter a valid purchase date'); frm.purchdate.select(); return false; }
					else if (!frm.purchprice.value.length) { alert('Please enter a purchase price'); frm.purchprice.select(); return false; }
					else { return true; }
				}
				else { return true; }
			}

			function fillData(frm,ddis,pdis,dval,pval,nu)
			{
				frm.purchdate.disabled = ddis;
				frm.purchprice.disabled = pdis;
				frm.purchdate.value = dval;
				frm.purchprice.value = pval;
				if (nu == <?=ITEM_NEW;?> || nu == <?=ITEM_USED;?>) { document.getElementById('nu'+nu).checked = true; }
				if (!ddis) { frm.purchdate.focus() }
			}
		</script>

		The selected item was purchased on the following dates.<br />
		Select the returned item's condition and either select a purchased item or enter a purchase date/price:
		<p />
		<form method="post" action="/admin/pos/invoice_return_lookup.php" id="retfrm" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="select" />
		<input type="hidden" name="type" value="<?=$type;?>" />
		<input type="hidden" name="resultIDX" value="<?=$resultIDX;?>" />
		<?=$pg->outlineTableHead();?>
		<tr bgcolor="<?=$pg->color('table-head');?>">
			<td>&nbsp;</td>
			<td><b>Date</b></td>
			<td><b>Price</b></td>
			<td><b>Platform</b></td>
			<td><b>Title</b></td>
			<td><b>New/Used</b></td>
		</tr>
		<?php
		$invoice_items = array_merge(array(array('ini_itemID'=>'none')),$invoice_items);
		while (list($a,$arr) = each($invoice_items))
		{
			$bg = (($a%2)?$pg->color('table-cell'):$pg->color('table-cell2'));

			if ($arr['ini_itemID'] == 'none')
			{
				?>
				<label for="pd<?=$a;?>">
				<tr bgcolor="<?=$bg;?>">
					<td><input type="radio" name="sel_purchdateprice" value="0" onclick="fillData(this.form,false,false,'','','')" class="nb" id="pd<?=$a;?>" checked="checked" /></td>
					<td colspan="5">None (enter purchase date/price below)</td>
				</tr>
				</label>
				<?php
			}
			else
			{
				?>
				<label for="pd<?=$a;?>">
				<tr bgcolor="<?=$bg;?>">
					<td><input type="radio" name="sel_purchdateprice" value="<?="{$arr['inv_completedtime']}|{$arr['ini_price']}";?>" onclick="fillData(this.form,true,true,'<?=date('m/d/Y',$arr['inv_completedtime']);?>',<?=$arr['ini_price'];?>,<?=$arr['ini_newused'];?>)" class="nb" id="pd<?=$a;?>" /></td>
					<td><?=date('m/d/Y',$arr['inv_completedtime']);?></td>
					<td align="right">$<?=number_format($arr['ini_price'],2);?></td>
					<td><?=$arr['ini_platform_name'];?></td>
					<td><?=$arr['ini_title'];?></td>
					<td><?=($arr['ini_newused']==ITEM_NEW?'New':'Used');?></td>
				</tr>
				</label>
				<?php
			}
		}
		$pg->outlineTableFoot();
		?>
		<p />
		<?=$pg->outlineTableHead();?>
		<tr>
			<td bgcolor="<?=$pg->color('table-label');?>"><b>Item Condition:</b></td>
			<td bgcolor="<?=$pg->color('table-cell');?>">
				<input type="radio" name="newused" value="<?=ITEM_NEW;?>" id="nu<?=ITEM_NEW;?>" class="nb" /> <label for="nu<?=ITEM_NEW;?>">New</label>
				<input type="radio" name="newused" value="<?=ITEM_USED;?>" id="nu<?=ITEM_USED;?>" class="nb" checked="checked" /> <label for="nu<?=ITEM_USED;?>">Used</label>
			</td>
		</tr>
		<?=$pg->outlineTableFoot();?>
		<p />
		<div style="width:250px;text-align:left">
			<b>Purchase Date:</b> <input type="text" name="purchdate" size="12" /> <font size="1">(mm/dd/yyyy)</font><br />
			<b>Purchase Price:</b> $<input type="text" name="purchprice" size="7" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value,false)" style="text-align:right" />
		</div>
		<p />
		<input type="submit" value="Select Purchase Date/Price &gt;" class="btn" />
		</form>
		<?php

		$pg->addOnload("document.getElementById('retfrm').purchdate.select()");
	}
	else { echo "unable to obtain itemID"; }
}
elseif ($act == "select")
{
	// add the returned item to the invoice
	$newused = @$_POST['newused'];
	$sel_purchdateprice = @$_POST['sel_purchdateprice'];
	$enter_purchdate = @$_POST['purchdate'];
	$enter_purchprice = @$_POST['purchprice'];
	if ($sel_purchdateprice != 0) { list($purchdate,$purchprice) = explode('|',$sel_purchdateprice); }
	else { $purchdate = strtotime($enter_purchdate); $purchprice = $enter_purchprice; }

	?>
	<script type="text/javascript">
		window.opener.parent.lookupifrm.location = '/admin/pos/invoice_lookup.php?act=doadd&addIDXs=<?=$resultIDX;?>&add_newused=<?=$newused;?>&type=<?=$type;?>&purchdate=<?=$purchdate;?>&purchprice=<?=$purchprice;?>';
		window.close();
	</script>
	<?php
}

?>
<div style="width:98%;text-align:right;padding-bottom:1px"><input type="button" value="Close &gt;" class="btn" onclick="window.opener.parent.clearSearch(<?=$type;?>);window.close()"></div>

<script type="text/javascript">
	onunload = function() {
		if (window.opener.parent && !window.opener.parent.closed) { window.opener.parent.clearSearch(<?=$type;?>); }
	}
</script>
<?php

$pg->foot();
?>