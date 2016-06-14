<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Area Reports');

$act = getGP('act');

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Area Reports');
$pg->head('Area Reports');

if ($act == 'print')
{
	// output a printable list of the sales/trades by area
	$platformID = $_GET['platformID'];
	$from = strtotime($_GET['fromdate']);
	$to = strtotime($_GET['todate'] . ' 11:59:59pm');
	$type = $_GET['type'];
	$type_name = ($type==SALE ? 'Sales' : ($type==RETURNS ? 'Returns' : 'Trades'));
	$sort = getG('sort');

	$pla = new platforms($pg,$platformID);

	$inv = new invoice($pg);
	$inv->setAreaData($platformID, $from, $to, $type, $sort);
	$area_data = $inv->getAreaData();
	extract($area_data);

	?>
	<p>
	<?php echo $type_name;?> area/zip code totals from <?php echo date('m/d/Y',$from);?> - <?php echo date('m/d/Y',$to);?><?php
	if ($platformID)
	{
		?>, for platform <b><?php echo $pla->platform_name();?></b><?php
	}
	?>
	</p>

	<p class="note">
		<b>Note:</b>
		The totals shown reflect only those customers with 10- or 11-digit phone numbers and 5- or 9-digit zip codes.<br />
		They may differ due to entry differences in customer files.
	</p>

	<?php
	if (count($area_code) || count($zip_code))
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

		<p><b>Area Codes</b></p>
		<?php
		if (!count($area_code))
		{
			echo 'None found';
		}
		else
		{
			?>
			<table border="0" cellpadding="10" class="lr">
				<tr>
					<?php
					$tot_count = 0;
					$chunks = array_chunk($area_code, ceil(count($area_code) / 4), true);
					while (list($a, $area_codes) = each($chunks))
					{
						?>
						<td valign="top">
							<table border="0">
								<?php
								while (list($code, $count) = each($area_codes))
								{
									?>
									<tr>
										<td><?php echo $code;?></td>
										<td>&nbsp;</td>
										<td align="right"><?php echo number_format($count, 0);?></td>
										<td>&nbsp;</td>
										<td align="right"><?php echo number_format(($count / $area_code_total * 100), 2);?>%</td>
									</tr>
									<?php
									$tot_count += $count;
								}
								?>
							</table>
						</td>
						<?php
					}
					?>
				</tr>
			</table>

			<p>
				<b>
					Area Code Totals:
					<?php echo number_format($tot_count, 0) . " $type_name";?>,
					<?php echo count($area_code);?> distinct area codes
				</b>
			</p>
			<?php
		} // if area code totals found
		?>

		<hr />

		<p><b>Zip Codes</b></p>
		<?php
		if (!count($zip_code))
		{
			echo 'None found';
		}
		else
		{
			?>
			<table border="0" cellpadding="10" class="lr">
				<tr>
					<?php
					$tot_count = 0;
					$chunks = array_chunk($zip_code, ceil(count($zip_code) / 4), true);
					while (list($a, $zip_codes) = each($chunks))
					{
						?>
						<td valign="top">
							<table border="0">
								<?php
								while (list($code, $count) = each($zip_codes))
								{
									?>
									<tr>
										<td><?php echo $code;?></td>
										<td>&nbsp;</td>
										<td align="right"><?php echo number_format($count, 0);?></td>
										<td>&nbsp;</td>
										<td align="right"><?php echo number_format(($count / $zip_code_total * 100), 2);?>%</td>
									</tr>
									<?php
									$tot_count += $count;
								}
								?>
							</table>
						</td>
						<?php
					}
					?>
				</tr>
			</table>

			<p>
				<b>
					Zip Code Totals:
					<?php echo number_format($tot_count, 0) . " $type_name";?>,
					<?php echo count($zip_code);?> distinct zip codes
				</b>
			</p>
			<?php
		} // if zip code totals found
	} // if zip code or area code data found
	else
	{
		?>
		No <?php echo $type_name;?> found in selected date range
		<p />
		<input type="button" value="&lt; Select Different Date" onclick="document.location='by_area.php'" class="btn" />
		<?php
	}
}

$pg->foot();
?>
