<?php
include('../../include/include.inc');

$act = getGP('act');
$type = getGP('type');
$itemID = getGP('itemID');
$returndata = getGP('returndata');

$pg = new admin_page();
$error = new error('Add Item Lookup Results');
$its = new item_search($pg);
$its->action = $_SESSION['root_admin'] . 'pos/invoice_lookup_results.php';
$its->max_results = 250;

$title = 'Add ' . invType($type,YES) . ' Item Search Results';
$pg->setFull(NO);
$pg->setTitle($title);
$pg->head($title);

if ($act == '')
{
	$results = $_SESSION['search_results'];
	$its->results = $_SESSION['search_results'];

	$only1 = ''; $only2 = '';
	$count = count($results);
	if ($count > $its->max_results)
	{
		$only1 = " Only the first $its->max_results are shown.";
		$only2 = 'Please narrow your search criteria.<p />';

		$results = array_slice($results,0,$its->max_results);
		$its->results = $results;
		$_SESSION['search_results'] = $results;
	}

	?>
	<?=$count;?> item<?=($count==1?'':'s');?> matched your criteria.<?=$only1;?>
	<p />
	<?=$only2;?>
	<?php
	lookup_form($type,@$_SESSION['invoice_last_search'][$type]);

	if ($type == RETURNS)
	{
		// see if any of the above items are in the past invoices for the current customer
		$itemIDs = array();
		while (list($a,$arr) = each($_SESSION['search_results'])) { $itemIDs[] = $arr['itm_itemID']; }
		reset($_SESSION['search_results']);

		$in_invoice = array();
		$sql = "SELECT * FROM invoices,invoice_items,items,platforms WHERE inv_completed=" . YES . " AND inv_customerID={$_SESSION['customerID']} AND inv_invoiceID=ini_invoiceID AND ini_type=".SALE." AND ini_itemID=itm_itemID AND itm_itemID IN (".implode(',',$itemIDs).") AND itm_platformID=pla_platformID ORDER BY pla_name,itm_title";
		$result = mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result)) { $in_invoice[] = $row; }

		if (count($in_invoice))
		{
			?>
			<p />
			<?=count($in_invoice);?> of the below items <?=(count($in_invoice)!=1?'have':'has');?> been purchased by the current customer.
			<p />
			<?=$pg->outlineTableHead();?>
			<tr bgcolor="<?=$pg->color('table-head');?>">
				<td><b>Date</b></td>
				<td><b>Qty</b></td>
				<td><b>Price</b></td>
				<td><b>Platform</b></td>
				<td><b>Title</b></td>
				<td><b>New/Used</b></td>
				<td>&nbsp;</td>
			</tr>
			<?php
			while (list($a,$arr) = each($in_invoice))
			{
				$bg = (($a%2)?$pg->color('table-cell'):$pg->color('table-cell2'));

				?>
				<tr bgcolor="<?=$bg;?>">
					<td><?=date('m/d/Y',$arr['inv_completedtime']);?></td>
					<td align="right"><?=number_format($arr['ini_qty'],0);?></td>
					<td align="right">$<?=number_format($arr['ini_price'],2);?></td>
					<td><?=$arr['pla_name'];?></td>
					<td><?=$arr['itm_title'];?></td>
					<td><?=($arr['ini_newused']==ITEM_NEW ? 'New' : 'Used');?></td>
					<td><a href="javascript:void(0)" onclick="sel(<?=$arr['itm_itemID'];?>);$('itmresults').returndata.value='<?="{$arr['itm_itemID']}|{$arr['inv_completedtime']}|" . ($arr['ini_price'] / $arr['ini_qty']) . "|{$arr['ini_newused']}";?>';$('itmresults').submit();return false">Select &gt;</a></td>
				</tr>
				<?php
			}
			$pg->outlineTableFoot();
			?>
			<p />
			<font size="1"><b>Note:</b> If the item being returned is not in the list of previously purchased items, you can still return it; you will be asked for the purchase date.</font>
			<?php
		}
		else
		{
			?>
			<p />
			None of the below items have been purchased by the current customer.
			<p />
			<font size="1"><b>Note:</b> You may still select an item, but you will be asked for the purchase date.</font>
			<?php
		}

		?>
		<p />
		<hr width="75%" size="-1" color="#000000" noshade="noshade" />
		<p />
		<?php
	} // if return

	// show the results
	$its->showSmallResults(
		($type==RETURNS ? NO : YES),
		array(
			'type'       => $type,
			'returndata' => ''
		),
		YES
	);
} // if no act
elseif ($act == 'select')
{
	// find idx of itemID from multiple results form in session results
	if (is_array($itemID)) { $itemID = implode('|',$itemID); }

	?>
	<script type="text/javascript">
		window.opener.lookupifrm.location = '/admin/pos/invoice_lookup.php?act=select&type=<?=$type;?>&itemID=<?=$itemID;?>&returndata=<?=$returndata;?>';
		window.close();
	</script>
	<?php
}

?>
<div style="width:98%;text-align:right;padding-bottom:1px">
	<input type="button" value="Close &gt;" class="btn" onclick="window.opener.clearSearch(<?=$type;?>);window.close()">
</div>

<script type="text/javascript">
	onunload = function() {
		if (window.opener && !window.opener.closed) { window.opener.clearSearch(<?=$type;?>); }
	}
</script>
<?php

$pg->foot();

function lookup_form($type,$criteria)
{
	global $pg;

	?>
	<script type="text/javascript">
		function submitSearch(frm)
		{
			var upctitle = frm.upctitle.value;
			var platformID = frm.platformID.options[frm.platformID.selectedIndex].value;
			window.opener.lookupifrm.location = '/admin/pos/invoice_lookup.php?act=search&type=<?=$type;?>&getcriteria=<?=YES;?>&upctitle='+upctitle+'&platformID='+platformID;
			window.close();
			return false;
		}
	</script>

	<?php
	$pg->outlineTableHead();
	?>
	<tr><td bgcolor="<?=$pg->color('table-head');?>" align="center"><b>Search Again</b></td></tr>
	<tr>
		<td bgcolor="<?=$pg->color('table-cell');?>">
			<form method="get" onsubmit="return submitSearch(this)">
			<input type="hidden" name="act" value="search" />
			<input type="hidden" name="type" value="<?=$type;?>" />
				<b>UPC/Title:</b> <input type="text" name="upctitle" id="upctitle" size="25" value="<?=@$criteria['upctitle'];?>" />
				&nbsp;&nbsp;
				<b>Platform:</b> <select name="platformID" size="1"><option value=""></option><?php
					$pla = new platforms($pg,0);
					$pla->set_item('platforms');
					while (list($a,$arr) = each($pla->values))
					{
						if ($arr[0] == @$criteria['platformID']) { $s = ' selected="selected"'; } else { $s = ''; }
						?><option value="<?=$arr[0];?>"<?=$s;?>><?=$arr[1];?></option><?php
					}
				?></select>
				&nbsp;&nbsp;
				<input type="submit" value="Find Item &gt;" class="btn" />
			</form>
		</td>
	</tr>
	<?php
	$pg->outlineTableFoot();
	?><p /><?php

	$pg->addOnload("$('upctitle').select()");
} // end function lookup_form
?>