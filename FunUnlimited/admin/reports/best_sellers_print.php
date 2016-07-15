<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Best Selling Items');

$act = (isset($_GET['act'])?$_GET['act']:@$_GET['act']);

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Best Selling Items');
$pg->head('Best Selling Items');

if ($act == 'print')
{
	// output a printable list of the best selling items
	$platformID = $_GET['platformID'];
	$from = strtotime($_GET['fromdate']);
	$to = strtotime($_GET['todate'] . ' 11:59:59pm');
	$num = $_GET['num'];
	$order = $_GET['order'];
	$stock = $_GET['stock'];

	$pla = new platforms($pg,$platformID);

	$inv = new invoice($pg);
	$inv->setBestSellers($platformID,$from,$to,$num,$order,$stock);
	$best_sellers = $inv->getBestSellers();

	?>
	<style type="text/css">
		.cr { font-family:Courier New;font-size:8pt; }
	</style>

	Top <?=$num;?> best sellers from <?=date('m/d/Y',$from);?> - <?=date('m/d/Y',$to);?><?php
	if ($platformID)
	{
		?>, for platform <b><?=$pla->platform_name();?></b><?php
	}
	?>, calculated on <b><?=($order=='qty' ? 'units sold' : 'total sales');?></b>,
	showing <?=($stock=='either' ? 'both in and out of stock items' : ($stock=='in' ? 'in stock items only' : 'out of stock items only'));?>
	<p />
	<?php
	if (count($best_sellers))
	{
		$tot_qty = 0;
		$tot_qty_new = 0;
		$tot_qty_used = 0;
		$tot_price = 0;
		?>
		<table border="0">
			<tr>
				<th>#</th>
				<th>Platform</th>
				<th>Item</th>
				<th>Units</th>
				<th>Sales</th>
				<th>Avg/Unit</th>
				<th>&nbsp; OH N/U</th>
			</tr>
			<?php
			while (list($itemID,$arr) = each($best_sellers))
			{
				?>
				<tr>
					<td align="right">#<?=$arr['PLACE'];?></td>
					<td>&nbsp;<?=$arr['pla_name'];?></td>
					<td>&nbsp;<?=$arr['itm_title'];?></td>
					<td align="right"><?=number_format($arr['tot_qty'],0);?></td>
					<td align="right">$<?=number_format($arr['tot_price'],2);?></td>
					<td align="right">$<?=number_format(($arr['tot_price'] / $arr['tot_qty']),2);?></td>
					<td align="right"><?=number_format($arr['qty_new'], 0) . ' / ' . number_format($arr['qty_used'], 0);?></td>
				</tr>
				<?php
				$tot_qty += $arr['tot_qty'];
				$tot_qty_new += $arr['qty_new'];
				$tot_qty_used += $arr['qty_used'];
				$tot_price += $arr['tot_price'];
			}
			?>
			<tr>
				<td colspan="3"><b>TOTALS:</b></td>
				<td align="right"><b><?=number_format($tot_qty,0);?></b></td>
				<td align="right"><b>$<?=number_format($tot_price,2);?></b></td>
				<td align="right"><b>$<?=number_format(($tot_price / $tot_qty),2);?></b></td>
				<td align="right"><b><?=number_format($tot_qty_new, 0) . ' / ' . number_format($tot_qty_used, 0);?></b></td>
			</tr>
		</table>
		<?php
	} // if best sellers
	else
	{
		?>
		No sales found in selected date range
		<p />
		<input type="button" value="&lt; Select Different Date" onclick="document.location='best_sellers.php'" class="btn" />
		<?php
	}
}

$pg->foot();
?>
