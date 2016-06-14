<?php
/**
 * Trade information
 * Created: 10/08/2012
 * Revised: 10/08/2012
 */

include('../../include/include.inc');

$cl = new check_login();

$act = getGP('act');

$pg = new admin_page();
$cust = new customers($pg);

$error = new error('Trade Information');

// make sure $_SESSION['cust_items'] is populated
$inv = new invoice($pg);
$inv->check_cust_items();

$numtrade = 0;
$numsale = 0;
$totcredit = 0;
$totcash = 0;
while (list($a,$arr) = each($_SESSION['cust_items']))
{
	if ($arr['ini_type'] == TRADE || $arr['ini_type'] == RETURNS)
	{
		$numtrade++;
		if ($arr['ini_trade_type'] == CREDIT)
		{
			$totcredit += $arr['ini_price'];
		}
		elseif ($arr['ini_trade_type'] == CASH)
		{
			$totcash += $arr['ini_price'];
		}
	}
	elseif ($arr['ini_type'] == SALE)
	{
		$numsale++;
	}
}
reset($_SESSION['cust_items']);

$req_fields = 0;
if (!$numtrade && !$totcash)
{
	$act = 'complete';
}
else
{
	$req_fields = $cust->check_trade_info();
}

if ($act == '')
{
	$pg->setTitle('Trade Information');
	$pg->head('Trade Information',YES);

	if (!$numsale)
	{
		$cust->set_customerID($_SESSION['customerID']);
		$cust_credit = $cust->info['creditamount'];
		$totcredit += $cust_credit;

		$showcredit = 'Customer: $'.number_format($cust_credit,2).'<br /><u>+From Invoice: $'.number_format(($totcredit-$cust_credit),2).'</u><br />Total: $'.number_format($totcredit,2);
	}
	else
	{
		$totcredit = $_SESSION['cust_close_options']['set_credit'];

		$showcredit = '$'.number_format($totcredit,2);
	}

	$pg->outlineTableHead();
	?>
	<tr><td colspan="2" align="center" bgcolor="<?php echo $pg->color('table-head');?>"><b>Trade Information</b></td></tr>
	<tr>
		<td bgcolor="<?php echo $pg->color('table-label');?>"><b>Cash:</b></td>
		<td align="right" bgcolor="<?php echo $pg->color('table-cell');?>">$<?php echo number_format($totcash,2);?></td>
	</tr>
	<tr>
		<td bgcolor="<?php echo $pg->color('table-label');?>"><b>Credit:</b></td>
		<td align="right" bgcolor="<?php echo $pg->color('table-cell');?>"><?php echo $showcredit;?></td>
	</tr>
	<?php
	$pg->outlineTableFoot();
	?><p /><?php

	if (!isset($_SESSION['cust_close_options']) || !is_array(@$_SESSION['cust_close_options']))
	{
		$_SESSION['cust_close_options'] = array(
			'set_credit' => $totcredit,
			'cash_out'   => $totcash
		);
	}

	if (count($req_fields))
	{
		// output the required information entry form
		?>
		Please enter the required information below.
		<p />
		<font size="1"><b>Note:</b> Be sure to tell the customer that this information is <b>required</b> by the state!</font>
		<p />

		<form method="post" action="/admin/pos/invtradeinfo.php" name="ti" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="setinfo">
		<?php
		$pg->outlineTableHead();
		?>
		<tr>
			<td colspan="2" bgcolor="<?php echo $pg->color('table-label');?>" align="center"><b>Required Information</b></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $pg->color('table-label');?>"><b>Customer ID:</b></td>
			<td bgcolor="<?php echo $pg->color('table-cell');?>"><?php echo $_SESSION['customerID'];?></td>
		</tr>
		<?php
		while (list($a,$arr) = each($req_fields))
		{
			$title = $arr[0];
			$field = $arr[1];
			$size = $arr[2];
			$vals = @$arr[3];
			$swap_key_value = (@$arr[4]);

			if (!count($vals))
			{
				?>
				<tr>
					<td bgcolor="<?php echo $pg->color('table-label');?>"><b><?php echo $title;?>:</b></td>
					<td bgcolor="<?php echo $pg->color('table-cell');?>"><input type="text" name="<?php echo $field;?>" size="<?php echo $size;?>"></td>
				</tr>
				<?php
			}
			else
			{
				?>
				<tr>
					<td bgcolor="<?php echo $pg->color('table-label');?>"><b><?php echo $title;?>:</b></td>
					<td bgcolor="<?php echo $pg->color('table-cell');?>">
						<select name="<?php echo $field;?>" size="1">
							<?php

							foreach ($vals as $s => $v)
							{
								if ($swap_key_value)
								{
									$foo = $s;
									$s = $v;
									$v = $foo;
								}

								?>
								<option value="<?php echo $v;?>"><?php echo $s;?></option>
								<?php
							}

							?>
						</select>
					</td>
				</tr>
				<?php
			}
		}

		$pg->outlineTableFoot();
		?>
		<p />
		<input type="submit" value="Update Information &gt;" class="btn"> <input type="reset" value="Reset Form &gt;" class="btn">
		<?php
		if (!$numsale)
		{
			?>
			<p />
			<input type="button" value="&lt; Return to Invoice" onclick="document.location='/admin/pos/invoice.php'" class="btn">
			<?php
		}
		?>
		</form>

		<script type="text/javascript">
			function verify(frm)
			{
				var foc = null;

				for (var i=0; i<frm.elements.length; i++)
				{
					var obj = frm.elements[i];
					if (obj.type == 'text' && obj.value == '')
					{
						foc = obj;
						break;
					}
					else if (obj.type == 'select-one' && obj.selectedIndex == 0)
					{
						foc = obj;
						break;
					}
				}

				if (foc == null && !validDate(frm.elements['dob'].value))
				{
					// invalid DOB
					alert('Please enter a valid DOB (MM/DD/YYYY)');
					frm.elements['dob'].focus();
					return false;
				}

				if (foc)
				{
					alert('Please enter a value in all fields.');
					foc.focus();
					return false;
				}
				else
				{
					return true;
				}
			}
		</script>
		<?php

		$pg->addOnload('document.ti.elements[1].focus()');
	}
	else
	{
		?>
		Please give the customer their cash/credit before pressing <b>Complete Invoice</b>
		<p />

		<form method="post" action="/admin/pos/invtradeinfo.php">
		<input type="hidden" name="act" value="complete">
		<input type="button" value="&lt; Return to Invoice" onclick="document.location='/admin/pos/invoice.php'" class="btn">
		<input type="submit" value="Complete Invoice &gt;" class="btn">
		</form>
		<?php

		if (($totcash + $totcredit) == 0)
		{
			// no cash or credit, just continue
			headerLocation('/admin/pos/invtradeinfo.php?act=complete');
		}
	}

	$pg->foot();
}
elseif ($act == "setinfo" || $act == "complete")
{
	if ($act == "setinfo")
	{
		// set the information
		$sql = "UPDATE customers SET ";

		$vals = array();
		while (list($k,$v) = each($_POST))
		{
			if ($k != 'act')
			{
				$vals[] = "cus_$k='".mysql_escape_string(stripslashes($v))."'";
			}
		}
		$sql .= implode(',',$vals);

		$sql .= ' WHERE cus_customerID='.$_SESSION['customerID'];

		mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);
	}

	$pg->showUpdating('Closing invoice...', '/admin/pos/invoice.php?act=close&complete=' . YES . '&print_tradeID=' . $_SESSION['cust_invoiceID']);
}

/* END OF FILE */
/* Location: ./admin/pos/invtradeinfo.php */