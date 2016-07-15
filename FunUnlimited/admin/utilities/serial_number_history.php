<?php
/**
 * Serial number history
 * Created: 10/05/2012
 * Version: 10/06/2012
 */

include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Serial Number History');

$pg = new admin_page();
$pg->setTitle('Serial Number History');
$pg->head('Serial Number History');

$serial_number = getG('serial_number');

if (!$serial_number)
{
	$pg->addOnload("$('serial_number').focus()");
}

?>
<form method="get" action="serial_number_history.php">

<?php echo $pg->outlineTableHead();?>
<tr>
	<td bgcolor="<?php echo $pg->color('table-label');?>"><b>Serial Number:</b></td>
	<td bgcolor="<?php echo $pg->color('table-cell');?>">
		<input type="text" size="35" name="serial_number" id="serial_number" value="<?php echo htmlspecialchars($serial_number);?>" />
	</td>
</tr>
<?php echo $pg->outlineTableFoot();?>

<p>
	<input type="submit" value="View History &gt;" class="btn">
</p>

</form>
<?php

if (!$serial_number)
{
	$pg->foot();
	die();
}

// look up the serial number history
echo "<p><b>Serial Number:</b> $serial_number</p>";

$inv = new invoice($pg);
$items = $inv->getInvoiceItems(0, 0, 0, $serial_number);

if ($items)
{
	echo $pg->outlineTableHead();

	?>
	<tr bgcolor="<?php echo $pg->color('table-head');?>">
		<th>Date/Time</th>
		<th>Type</th>
		<th>Invoice #</th>
		<th>Customer</th>
		<th>Employee</th>
		<th>Platform</th>
		<th>Title</th>
	</tr>
	<?php

	$shown = -1;

	foreach ($items as $arr)
	{
		$shown++;
		$bg = $pg->color('table-cell' . (!($shown % 2) ? '2' : ''));

		?>
		<tr style="background:<?php echo $bg;?>">
			<td>
				<?php echo date('m/d/Y h:ia', $arr['inv_completedtime']);?>
			</td>
			<td>
				<?php

				echo ($arr['ini_type']==SALE
					? 'Sale'
					: ($arr['ini_type']==TRADE
						? 'Trade'
						: ($arr['ini_type']==RETURNS
							? 'Return'
							: 'Other'
						)
					)
				);

				?>
			</td>
			<td align="right">
				<?php echo $arr['inv_invoiceID'];?>
			</td>
			<td>
				<?php echo $arr['cus_fname'] . ' ' . $arr['cus_lname'];?>
			</td>
			<td>
				<?php echo ($arr['emp_fname'] ? $arr['emp_fname'] . ' ' . $arr['emp_lname'] : '');?>
			</td>
			<td>
				<?php echo $arr['ini_platform_name'];?>
			</td>
			<td>
				<?php echo $arr['ini_title'];?>
			</td>
		</tr>
		<?php

		$last_type = $arr['ini_type'];
	} // foreach item

	echo $pg->outlineTableFoot();

	?>
	<p>
		<b>Expected item status:</b>
		<?php

		if ($last_type == SALE)
		{
			echo 'sold; should no longer be in store inventory';
		}
		elseif ($last_type == TRADE)
		{
			echo 'traded; should be in store inventory';
		}
		elseif ($last_type == RETURNS)
		{
			echo 'returned; should be in store inventory';
		}

		?>
	</p>
	<?php
} // if items found
else
{
	echo '<p>No history found for this serial number</p>';
}

$pg->foot();

/* END OF FILE */
/* Location: ./admin/utilities/serial_number_history.php */