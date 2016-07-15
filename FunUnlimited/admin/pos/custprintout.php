<?php
include('../../include/include.inc');

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Customer Trade Printout');
$pg->head();

$sto = new stores($pg);
$sto->set_storeID($_SESSION['storeID']);
$store_name = (strlen($sto->info['name'])?$sto->info['name']:'Unknown Store');

$inv = new invoice($pg);

$error = new error('Customer printout');

$colspan = 12;

?>
<font size="4"><b>Customer Trade Information</b></font>
<p />
<b>Store:</b> <?=$store_name;?><br />
<b>Customer:</b> <?=$_SESSION['cust_info']['fname'].' '.$_SESSION['cust_info']['lname'];?><br />
<p />
<font size="1">Please circle whether you would like cash or credit for each item.</font>
<p />

<table border="0" cellspacing="8" cellpadding="0">
	<tr>
		<td align="center" valign="bottom">&nbsp;&nbsp;<b>Title</b></td>
		<td width="1" rowspan="<?=((count($_SESSION['cust_items'])*2)+4);?>" bgcolor="#000000"></td>
		<td align="center" valign="bottom"><b>Plfrm</b></td>
		<td width="1" rowspan="<?=((count($_SESSION['cust_items'])*2)+4);?>" bgcolor="#000000"></td>
		<td align="center"><b>Cash</b></td>
		<td width="1" rowspan="<?=((count($_SESSION['cust_items'])*2)+4);?>" bgcolor="#000000"></td>
		<td align="center"><b>Credit</b></td>
		<td width="1" rowspan="<?=((count($_SESSION['cust_items'])*2)+4);?>" bgcolor="#000000"></td>
	</tr>
	<?php

	$all_itemIDs = array();
	while (list($a,$arr) = each($_SESSION['cust_items']))
	{
		if (!in_array($arr['ini_itemID'],$all_itemIDs)) { $all_itemIDs[] = $arr['ini_itemID']; }
	}
	reset($_SESSION['cust_items']);

	$percopyqty = array();
	if (count($all_itemIDs))
	{
		$sql = "SELECT itm_itemID,qty_new,qty_used FROM items,quantity WHERE itm_itemID IN (".implode(',',$all_itemIDs).") AND itm_itemID=qty_itemID AND qty_storeID=".$_SESSION['storeID'];
		$result = mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			$percopyqty[$row['itm_itemID']] = ($row['qty_new']+$row['qty_used']);
		}
	}

	$items = 0;
	$numtimes = array();
	$totcash = 0;
	$totcredit = 0;
	while (list($a,$arr) = each(@$_SESSION['cust_items']))
	{
		if ($arr['ini_type'] == TRADE)
		{
			if (!isset($numtimes[$arr['ini_itemID']])) { $numtimes[$arr['ini_itemID']] = -1; }
			$numtimes[$arr['ini_itemID']]++;
			$items++;

			$base = $arr['ini_price_used'];

			$mods = array(
				'customerID'=>$_SESSION['customerID'],
				'itemID'=>$arr['ini_itemID'],
				'platformID'=>$arr['ini_platformID'],
				'type'=>$arr['ini_type'],
				'trade_type'=>$arr['ini_trade_type'],
				'sale_newused'=>$arr['ini_newused'],
				'box_type'=>$arr['ini_box'],
				'trade_condition'=>$arr['ini_condition'],
				'percentoff'=>$arr['ini_percentoff'],
				'salemilestoneoff'=>$arr['ini_salemilestoneoff'],
				'trademilestoneup'=>$arr['ini_trademilestoneup'],
				'totcopies'=>$percopyqty[$arr['ini_itemID']]+$numtimes[$arr['ini_itemID']],
				'invoiceqty'=>$arr['ini_qty']
			);

			$qty = ($percopyqty[$arr['ini_itemID']]+$numtimes[$arr['ini_itemID']]);

			if ($arr['ini_box'] == BOX)
			{
				$ca_show = $inv->apply_price_mods($base,array_replace_keys($mods,array('trade_type'=>CASH,'box_type'=>BOX)));
				$cr_show = $inv->apply_price_mods($base,array_replace_keys($mods,array('trade_type'=>CREDIT,'box_type'=>BOX)));
			}
			else
			{
				$ca_show = $inv->apply_price_mods($base,array_replace_keys($mods,array('trade_type'=>CASH,'box_type'=>NOBOX)));
				$cr_show = $inv->apply_price_mods($base,array_replace_keys($mods,array('trade_type'=>CREDIT,'box_type'=>NOBOX)));
			}

			$totcash += $ca_show;
			$totcredit += $cr_show;

			?>
			<tr><td colspan="<?=$colspan;?>" bgcolor="#000000"></td></tr>
			<tr>
				<td>&nbsp;&nbsp;<?=$arr['ini_title'];?></td>

				<td><?=$arr['ini_platform_abbr'];?></td>

				<td align="right"><?=number_format($ca_show,2);?></td>

				<td align="right"><?=number_format($cr_show,2);?></td>
			</tr>
			<?php
		}
	}
	reset($_SESSION['cust_items']);

	if (!$items)
	{
		?><tr><td align="center" colspan="<?=$colspan;?>">&nbsp;<br />There are no trade items on the current invoice<br />&nbsp;</td></tr><?php
	}
	else
	{
		?>
		<tr><td colspan="<?=$colspan;?>" bgcolor="#000000"></td></tr>
		<tr>
			<td colspan="3" align="right"><b>Totals:</b></td>
			<td align="right"><b>$<?=number_format($totcash,2);?></b></td>
			<td align="right"><b>$<?=number_format($totcredit,2);?></b></td>
		</tr>
		<tr><td colspan="<?=$colspan;?>" bgcolor="#000000"></td></tr>
		<?php
	}

	?>
</table>

<script type="text/javascript">
	function closewin() { window.close(); }
</script>
<?php

$pg->addOnload('window.print()');
$pg->foot();
?>