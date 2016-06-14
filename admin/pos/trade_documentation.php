<?php
/**
 * Print trade documentation
 */

include('../../include/include.inc');

$cl = new check_login();
$error = new error('Trade Documentation');

$invoiceID = getG('invoiceID');

if (!$invoiceID)
{
	$error->show('No invoiceID');
}

$pg = new admin_page();
$pg->setFull(false);
$pg->setCenter(false);
$pg->setTitle('Trade Documentation');
$pg->head('Trade Documentation');

// get the invoice info
$sql = "
	SELECT
		*
	FROM
		invoices,
		invoice_items,
		stores
	WHERE
		inv_storeID=sto_storeID
		AND inv_completed=" . YES . "
		AND inv_invoiceID=$invoiceID
		AND inv_invoiceID=ini_invoiceID
		AND ini_type=" . TRADE . '
	ORDER BY
		ini_platform_name,
		ini_title
';
$result = mysql_query($sql, $db);
$error->mysql(__FILE__, __LINE__);

if (!mysql_num_rows($result))
{
	$error->show("Invalid invoiceID: $invoiceID");
}

$info = array();
$items = array();

while ($row = mysql_fetch_assoc($result))
{
	$info = $row;
	$items[] = $row;
}

// pull in the customer info
$cust = new customers($pg);
$cust->set_customerID($info['inv_customerID'], false, false);


// show the info tables
?>
<table border="0">
	<tr>
		<td valign="top">
			<?php

			$cust->show_info_table('Contact Information', '');

			echo '<br />';

			$cust->show_info_table('Other Information', '');

			echo '<br />';

			// trade invoice info
			$pg->outlineTableHead('');

			?>
			<tr>
				<td colspan="2" bgcolor="<?php echo $pg->color('table-label');?>" align="center">
					<b>Trade Invoice Information</b>
				</td>
			</tr>
			<tr>
				<td bgcolor="<?php echo $pg->color('table-cell');?>">
					<b>Started Date/Time</b>
				</td>
				<td bgcolor="<?php echo $pg->color('table-cell2');?>" width="100%">
					<?php echo date('m/d/Y h:i:sa', $info['inv_time']);?>
				</td>
			</tr>
			<tr>
				<td bgcolor="<?php echo $pg->color('table-cell');?>">
					<b>Completed Date/Time</b>
				</td>
				<td bgcolor="<?php echo $pg->color('table-cell2');?>" width="100%">
					<?php echo date('m/d/Y h:i:sa', $info['inv_completedtime']);?>
				</td>
			</tr>
			<tr>
				<td bgcolor="<?php echo $pg->color('table-cell');?>">
					<b>Store</b>
				</td>
				<td bgcolor="<?php echo $pg->color('table-cell2');?>" width="100%">
					<?php echo $info['sto_name'];?><br />
					<?php echo $info['sto_address'];?><br />
					<?php echo $info['sto_city'] . ', ' . $info['sto_state'] . ' ' . $info['sto_zip'];?><br />
					Phone: <?php echo $pg->format('phone', $info['sto_phone']);?><br />
					Fax: <?php echo $pg->format('phone', $info['sto_fax']);?><br />
					Email: <?php echo $info['sto_email'];?><br />
				</td>
			</tr>
			<?php

			$pg->outlineTableFoot();

			?>
		</td>
		<td valign="top" align="center">
			<p><b>Customer Fingerprint</b></p>
			<div style="width:150px;height:200px;border:solid 3px #000"></div>
		</td>
	</tr>
</table>
<?php

echo '<br />';

// items
$pg->outlineTableHead(600);

?>
<tr>
	<td colspan="6" bgcolor="<?php echo $pg->color('table-label');?>" align="center">
		<b>Traded Items</b>
	</td>
</tr>
<?php

foreach ($items as $idx => $arr)
{
	$bg = (!(($idx + 1) % 2) ? $pg->color('table-cell') : $pg->color('table-cell2'));

	if (!($idx % 10))
	{
		?>
		<tr>
			<td align="center" bgcolor="<?php echo $pg->color('table-head-lighter');?>"><b>Platform</b></td>
			<td width="100%" align="center" bgcolor="<?php echo $pg->color('table-head-lighter');?>"><b>Title</b></td>
			<td align="center" bgcolor="<?php echo $pg->color('table-head-lighter');?>"><b>New/Used</b></td>
			<td align="center" bgcolor="<?php echo $pg->color('table-head-lighter');?>"><b>Cash/Credit</b></td>
			<td align="center" bgcolor="<?php echo $pg->color('table-head-lighter');?>"><b>Qty</b></td>
			<td align="center" bgcolor="<?php echo $pg->color('table-head-lighter');?>"><b>Price</b></td>
		</tr>
		<?php
	}

	?>
	<tr>
		<td bgcolor="<?php echo $bg;?>"><?php echo $arr['ini_platform_name'];?></td>
		<td bgcolor="<?php echo $bg;?>"><?php echo $arr['ini_title'];?></td>
		<td bgcolor="<?php echo $bg;?>"><?php echo ($arr['ini_newused']==ITEM_NEW ? 'New' : ($arr['ini_condition']==CNEW ? 'New' : 'Used'));?></td>
		<td bgcolor="<?php echo $bg;?>"><?php echo ($arr['ini_trade_type']==CASH ? 'Cash' : 'Credit');?></td>
		<td align="right" bgcolor="<?php echo $bg;?>"><?php echo number_format($arr['ini_qty'], 0);?></td>
		<td align="right" bgcolor="<?php echo $bg;?>">$<?php echo number_format($arr['ini_price'], 2);?></td>
	</tr>
	<?php
} // foreach item

$pg->outlineTableFoot();

$pg->foot();

/* END OF FILE */
/* Location: ./admin/pos/trade_documentation.php */