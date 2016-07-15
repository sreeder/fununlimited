<?php
include('../../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setTitle('Customer Invoice History');
$pg->head('Customer Invoice History',YES);

$error = new error('Customer Invoice History');
$cust = new customers($pg);

$act = getGP('act');
$limit_store = getGP('limit_store');
$customerID = @$_SESSION['customerID'];
$show = getG('show','sales',array('sales','trades','returns'));

if (!$customerID)
{
	echo "Insufficient/invalid information";
}
else
{
	// output the invoice history for the current customer
	$cust = new customers($pg);
	$cust->set_customerID($customerID);

	?>
	<b>Customer:</b> <?php echo "{$cust->info['fname']} {$cust->info['lname']}";?>
	<p />

	<font size="1">
		<b>Viewing:</b>
		<?php echo ($show=='sales' ? '<b><u>' : '<a href="/admin/pos/custhistory.php?show=sales&limit_store=' . $limit_store . '">');?>Sales<?php echo ($show=='sales' ? '</u></b>' : '</a>');?>
		|
		<?php echo ($show=='trades' ? '<b><u>' : '<a href="/admin/pos/custhistory.php?show=trades&limit_store=' . $limit_store . '">');?>Trades<?php echo ($show=='trades' ? '</u></b>' : '</a>');?>
		|
		<?php echo ($show=='returns' ? '<b><u>' : '<a href="/admin/pos/custhistory.php?show=returns&limit_store=' . $limit_store . '">');?>Returns<?php echo ($show=='returns' ? '</u></b>' : '</a>');?>
	</font>
	<p />
	<?php

	if ($limit_store) { $store_where = "inv_storeID=$limit_store AND "; }
	else { $store_where = ""; }

	$sql = "SELECT * FROM invoices,invoice_items,stores WHERE {$store_where}inv_storeID=sto_storeID AND inv_completed=" . YES . " AND inv_customerID=$customerID AND inv_invoiceID=ini_invoiceID AND ini_type=" . ($show=='sales' ? SALE : ($show=='returns' ? RETURNS : TRADE)) . ' ORDER BY inv_time,ini_platform_name,ini_title';
	$result = mysql_query($sql, $db);
	$error->mysql(__FILE__,__LINE__);

	$lastinvoiceID = 0;
	$count = 0;
	$total = 0;
	$total_credit = 0; // total applied credit
	$total_qty = 0;
	$invtotal = 0;
	$invtotal_qty = 0;

	if (mysql_num_rows($result))
	{
		$this_store = NO; // is there any orders from the current store?
		$storeIDs = array();
		$orders = array();
		while ($row = mysql_fetch_assoc($result))
		{
			$orders[] = $row;
			$storeIDs[$row['inv_storeID']] = $row['inv_storeID'];
			if ($_SESSION['storeID'] == $row['inv_storeID']) { $this_store = YES; }
		}

		// if there are multiple stores, show the 'Limit to...' links
		if (count($storeIDs) > 1 || (count($storeIDs) == 1 && !$this_store))
		{
			$stores = array();
			$sql = "SELECT sto_storeID,sto_name FROM stores WHERE sto_storeID IN (".implode(',', $storeIDs).") ORDER BY sto_name";
			$result = mysql_query($sql, $db);
			$error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result)) { $stores[$row['sto_storeID']] = $row['sto_name']; }

			?>
			<form method="get" action="/admin/pos/custhistory.php">
			<input type="hidden" name="show" value="<?php echo $show;?>">
			<font size="1"><b>Limit to Store:</b></font>
			<select name="limit_store" size="1" onchange="this.form.submit()">
				<option value="">Show all stores</option>
				<?php
				while (list($storeID, $name) = each($stores))
				{
					if ($storeID == $limit_store) { $s = ' selected="selected"'; } else { $s = ''; }
					?>
					<option value="<?php echo $storeID;?>"<?php echo $s;?>><?php echo $name;?></option>
					<?php
				}
				?>
			</select>
			<input type="submit" value="Limit &gt;" class="btn" />
			</form>
			<p />
			<?php
		}
		else
		{
			$limit_store = array_sum($storeIDs);
		}

		// get the employees
		$emp = new employees($pg);
		$emp->get_employees(BOTH);
		$employees = $emp->employees;

		// output the order list
		$lastcredit = 0;
		$last_arr = array();
		while (list($a, $arr) = each($orders))
		{
			if ($arr['inv_invoiceID'] != $lastinvoiceID)
			{
				if ($lastinvoiceID)
				{
					table_bottom($count,($show=='sales' ? $lastcredit : 0), $invtotal, $invtotal_qty, $last_arr);
				}
				table_head($arr, $employees);
				$lastinvoiceID = $arr['inv_invoiceID'];
				$count = 0;
				$invtotal = ($show=='sales' ? $arr['inv_additional'] : 0);//($arr['inv_additional'] - ($show=='sales' ? ($arr['inv_credit'] + $arr['inv_cash_out']) : 0));
				$invtotal_qty = 0;
				$total += $invtotal;

				if ($show == 'sales')
				{
					$total_credit += $arr['inv_credit'];
					$lastcredit = $arr['inv_credit'];
				}
			}

			$count++;
			$bg = (!($count%2)?$pg->color('table-cell'):$pg->color('table-cell2'));

			$total        += $arr['ini_price'];
			$invtotal     += $arr['ini_price'];
			$total_qty    += $arr['ini_qty'];
			$invtotal_qty += $arr['ini_qty'];

			?>
			<tr>
				<td bgcolor="<?php echo $bg;?>"><?php echo $arr['ini_platform_name'];?></td>
				<td bgcolor="<?php echo $bg;?>"><?php echo $arr['ini_title'];?></td>
				<td bgcolor="<?php echo $bg;?>"><?php echo ($arr['ini_newused']==ITEM_NEW?'New':($arr['ini_condition']==CNEW?'New':'Used'));?></td>
				<td bgcolor="<?php echo $bg;?>"><?php echo ($show!='sales'?($arr['ini_trade_type']==CASH?'Cash':'Credit'):'&nbsp;');?></td>
				<td align="right" bgcolor="<?php echo $bg;?>"><?php echo number_format($arr['ini_qty'],0);?></td>
				<td align="right" bgcolor="<?php echo $bg;?>">$<?php echo number_format($arr['ini_price'],2);?></td>
			</tr>
			<?php
			$last_arr = $arr;
		}

		table_bottom($count, $lastcredit, $invtotal, $invtotal_qty, $last_arr);

		?>
		<b>Total Quantity:</b> <?php echo number_format($total_qty,0);?><br />
		<b>Total <?php echo ucwords($show);?>:</b> $<?php echo number_format($total,2);?>
		<?php
		if ($show == 'sales')
		{
			?>
			<br />
			<b>Total Sales w/o Applied Credit:</b> $<?php echo number_format($total - $total_credit, 2);?> <font color="red"><b>*</b></font>

			<p class="note">
				<font color="red"><b>*</b></font> <b>Note:</b> Due to taxes, this total may be slightly off.
			</p>
			<?php
		}
	}
	else
	{
		?>
		No <?php echo substr($show,0,(strlen($show)-1));?> history was found for the above customer
		<?php
	}

	?>
	<p />
	<input type="button" value="&lt; Return to Customer Information" onclick="document.location='/admin/pos/pos.php'" class="btn">
	<?php
}

function table_head($arr, $employees)
{
	global $pg, $show, $limit_store;

	$pg->outlineTableHead(600);
	?>
	<tr>
		<td colspan="6" bgcolor="<?php echo $pg->color('table-head');?>">
			<b>Invoice #<?php echo $arr['inv_invoiceID'];?></b><br />
			<b>Store:</b> <?php echo $arr['sto_name'];?>
			<?php
			if ($arr['sto_storeID'] != $limit_store)
			{
				?>
				<a href="custhistory.php?show=<?php echo $show;?>&limit_store=<?php echo $arr['sto_storeID'];?>">� Limit to Store</a>
				<?php
			}
			?><br />
			<b>Created:</b> <?php echo date('m/d/y h:ia', $arr['inv_time']);?><br />
			<b>Completed:</b> <?php echo date('m/d/y h:ia', $arr['inv_completedtime']);?><br />
			<b>Cash Paid Out:</b> $<?php echo number_format($arr['inv_cash_out'],2);?><br />
			<?php
			if ($show == 'sales')
			{
				?>
				<b>Additional Charges:</b> $<?php echo number_format($arr['inv_additional'],2);?><br />
				<b>Applied Credit:</b> $<?php echo number_format($arr['inv_credit'],2);?>
				<?php
			}
			if ($arr['inv_employeeID'] && isset($employees[$arr['inv_employeeID']]))
			{
				$emparr = $employees[$arr['inv_employeeID']];
				?>
				<br />
				<b>Employee:</b> <?php echo $emparr['emp_fname'] . ' ' . $emparr['emp_lname'];?>
				<?php
			}
			?>
		</td>
	</tr>
	<tr>
		<td align="center" bgcolor="<?php echo $pg->color('table-head-lighter');?>"><b>Platform</b></td>
		<td width="100%" align="center" bgcolor="<?php echo $pg->color('table-head-lighter');?>"><b>Title</b></td>
		<td align="center" bgcolor="<?php echo $pg->color('table-head-lighter');?>"><b>N/U</b></td>
		<td align="center" bgcolor="<?php echo $pg->color('table-head-lighter');?>"><b>Cash/Cred</b></td>
		<td align="center" bgcolor="<?php echo $pg->color('table-head-lighter');?>"><b>Qty</b></td>
		<td align="center" bgcolor="<?php echo $pg->color('table-head-lighter');?>"><b>Price</b></td>
	</tr>
	<?php
}

function table_bottom($count, $credit, $invtotal, $invtotal_qty, $arr)
{
	global $pg, $show;

	$count++;
	$bg = (!($count%2) ? $pg->color('table-cell') : $pg->color('table-cell2'));

	if ($show == 'sales' && $arr['inv_additional'] > 0)
	{
		?>
		<tr>
			<td colspan="4" align="right" bgcolor="<?php echo $bg;?>"><b>Additional Charges:</b></td>
			<td bgcolor="<?php echo $bg;?>">&nbsp;</td>
			<td align="right" bgcolor="<?php echo $bg;?>">+$<?php echo number_format($arr['inv_additional'],2);?></td>
		</tr>
		<?php
	}

	?>
	<tr>
		<td colspan="4" align="right" bgcolor="<?php echo $pg->color('table-head');?>"><b>Invoice Total:</b></td>
		<td align="right" bgcolor="<?php echo $pg->color('table-head');?>"><?php echo number_format($invtotal_qty,0);?></td>
		<td align="right" bgcolor="<?php echo $pg->color('table-head');?>">$<?php echo number_format($invtotal,2);?></td>
	</tr>
	<?php

	$pg->outlineTableFoot();
	?>
	<p />
	<?php
}

$pg->foot();
?>