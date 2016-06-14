<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Age/Gender Reports');

$act = getGP('act');

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Age/Gender Reports');
$pg->head('Age/Gender Reports');

if ($act == 'print')
{
	// output a printable list of the sales/trades by age/gender
	$platformID = $_GET['platformID'];
	$from = strtotime($_GET['fromdate']);
	$to = strtotime($_GET['todate'] . ' 11:59:59pm');
	$type = $_GET['type'];
	$type_name = ($type==SALE ? 'Sales' : ($type==RETURNS ? 'Returns' : 'Trades'));

	$pla = new platforms($pg,$platformID);

	$inv = new invoice($pg);
	$inv->setDemoData($platformID, $from, $to, $type);
	$demo_data = $inv->getDemoData();
	extract($demo_data);

	?>
	<p>
		<?php echo $type_name;?> age/gender totals from <?php echo date('m/d/Y',$from);?> - <?php echo date('m/d/Y',$to);?><?php
		if ($platformID)
		{
			?>, for platform <b><?php echo $pla->platform_name();?></b><?php
		}
		?>
	</p>

	<p class="note">
		<b>Note:</b> The totals shown include only those customers that have a gender selected or a DOB entered.<br />
		These are not indicative of your overall totals for the above date range<?php echo ($platformID ? '/platform' : '');?>.
	</p>

	<?php
	if (count($dob) || count($gender))
	{
		?>
		<style type="text/css">
			table.lr
			{
				border-collapse:collapse;
			}

			table.lr tr td
			{
				border:solid 1px #000;
			}

			table.lr tr td table tr td
			{
				border:0px;
			}
		</style>

		<hr />

		<p><b>By Gender</b></p>
		<?php
		if (!count($gender))
		{
			echo 'None found';
		}
		else
		{
			?>
			<table border="0" cellpadding="10" class="lr">
				<tr>
					<td valign="top">
						<table border="0">
							<tr>
								<th>Gender</th>
								<th>&nbsp;</th>
								<th align="right">Total Invoices</th>
								<th>&nbsp;</th>
								<th align="right">Total Qty</th>
								<th>&nbsp;</th>
								<th align="right">Total $</th>
								<th>&nbsp;</th>
								<th align="right">Total %</th>
							</tr>
							<?php
							while (list($show_gender, $arr) = each($gender))
							{
								?>
								<tr>
									<td><?php echo ($show_gender==MALE ? 'Male' : 'Female');?></td>
									<td>&nbsp;</td>
									<td align="right"><?php echo number_format($arr['tot_invoice'], 0);?></td>
									<td>&nbsp;</td>
									<td align="right"><?php echo number_format($arr['tot_qty'], 0);?></td>
									<td>&nbsp;</td>
									<td align="right">$<?php echo number_format($arr['tot_price'], 2);?></td>
									<td>&nbsp;</td>
									<td align="right"><?php echo number_format(($arr['tot_price'] / $total_gender['tot_price'] * 100), 2);?>%</td>
								</tr>
								<?php
							}
							?>
							<tr class="bold">
								<td>Totals:</td>
								<td>&nbsp;</td>
								<td align="right"><?php echo number_format($total_gender['tot_invoice'], 0);?></td>
								<td>&nbsp;</td>
								<td align="right"><?php echo number_format($total_gender['tot_qty'], 0);?></td>
								<td>&nbsp;</td>
								<td align="right">$<?php echo number_format($total_gender['tot_price'], 2);?></td>
								<td>&nbsp;</td>
								<td align="right"><?php echo number_format(100, 2);?>%</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
			<?php
		} // if area code totals found
		?>

		<hr />

		<p><b>By Age</b></p>
		<?php
		if (!count($dob))
		{
			echo 'None found';
		}
		else
		{
			?>
			<table border="0" cellpadding="10" class="lr">
				<tr>
					<td valign="top">
						<table border="0">
							<tr>
								<th>Age</th>
								<th>&nbsp;</th>
								<th align="right">Total Invoices</th>
								<th>&nbsp;</th>
								<th align="right">Total Qty</th>
								<th>&nbsp;</th>
								<th align="right">Total $</th>
								<th>&nbsp;</th>
								<th align="right">Total %</th>
							</tr>
							<?php
							while (list($show_age, $arr) = each($dob))
							{
								?>
								<tr>
									<td align="center"><?php echo $show_age;?></td>
									<td>&nbsp;</td>
									<td align="right"><?php echo number_format($arr['tot_invoice'], 0);?></td>
									<td>&nbsp;</td>
									<td align="right"><?php echo number_format($arr['tot_qty'], 0);?></td>
									<td>&nbsp;</td>
									<td align="right">$<?php echo number_format($arr['tot_price'], 2);?></td>
									<td>&nbsp;</td>
									<td align="right"><?php echo number_format(($arr['tot_price'] / $total_dob['tot_price'] * 100), 2);?>%</td>
								</tr>
								<?php
							}
							?>
							<tr class="bold">
								<td>Totals:</td>
								<td>&nbsp;</td>
								<td align="right"><?php echo number_format($total_dob['tot_invoice'], 0);?></td>
								<td>&nbsp;</td>
								<td align="right"><?php echo number_format($total_dob['tot_qty'], 0);?></td>
								<td>&nbsp;</td>
								<td align="right">$<?php echo number_format($total_dob['tot_price'], 2);?></td>
								<td>&nbsp;</td>
								<td align="right"><?php echo number_format(100, 2);?>%</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
			<?php
		} // if age totals found
	} // if gender or age
	else
	{
		?>
		No <?php echo $type_name;?> found in selected date range
		<p />
		<input type="button" value="&lt; Select Different Date" onclick="document.location='by_demo.php'" class="btn" />
		<?php
	}
}

$pg->foot();
?>
