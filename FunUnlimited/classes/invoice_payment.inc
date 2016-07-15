<?php
/**
* Point-of-sale invoice class - payment screen
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class invoice_payment
{
	/**
	* Output the payment options table
	* @param	float	$total_sale		sale total
	* @param	float	$total_cash		trade cash
	* @param	float	$total_credit	trade credit
	* @param	float	$cust_credit	customer credit
	*/
	function show_payment_options($total_sale,$total_cash,$total_credit,$cust_credit)
	{
		$store = new stores($this->pg);
		$store->set_storeID($_SESSION['storeID']);
		$tax_perc = "0.000";#$store->info['taxperc'];

		$with_tax = floor(($total_sale * (1 + ($tax_perc / 100))) * 100) / 100;
		$tax = ($with_tax - $total_sale);

		$avail_cash = $total_cash;
		$balance = ($total_sale + $tax);

		?>
		<script type="text/javascript">
			// these 6 never change - the others do
			var total_price          = <?php echo ($total_sale+$tax);?>;
			var total_sale           = <?php echo $total_sale;?>;
			var total_cash           = <?php echo $total_cash;?>;
			var total_cust_credit    = <?php echo $cust_credit;?>;
			var total_invoice_credit = <?php echo $total_credit;?>;
			var total_all_credit     = <?php echo ($total_credit+$cust_credit);?>;
			var tax                  = <?php echo $tax;?>;
			var tax_perc             = <?php echo $tax_perc;?>;

			var avail_cash   = <?php echo $avail_cash;?>;
			var avail_credit = <?php echo ($total_credit + $cust_credit);?>;
			var balance      = <?php echo $balance;?>;

			var cash_out   = 0;
			var cash_sale  = 0;
			var check      = 0;
			var credit     = 0;
			var charge     = 0;

			// apply the entire balance to one type
			function all(type)
			{
				var frm = document.pmnt;

				var rst = true;
				if (type == <?php echo CREDIT;?> && total_all_credit == 0) { rst = false; }

				if (rst == true)
				{
					frm.cash.value       = '';
					frm.check.value      = '';
					frm.credit.value     = '';
					frm.charge.value     = '';
					update_values();
				}

				if (type == <?php echo CASH;?>)
				{
					frm.cash.value = format_price(total_price);
				}
				else if (type == <?php echo CHECK;?>)
				{
					frm.check.value = format_price(total_price);
				}
				else if (type == <?php echo CREDIT;?> && total_all_credit > 0)
				{
					// add the additional and don't inclue the tax
					additional = (frm.additional.value?frm.additional.value:0);
					addition = parseFloat(additional);
					var total = (total_sale*1)+(additional*1);
					if (total_all_credit < total)
					{
						total = total_all_credit;
					}

					frm.credit.value = format_price(total);
				}
				else if (type == <?php echo CHARGE;?>)
				{
					frm.charge.value = format_price(total_price);
				}

				update_values();
			}

			// set a type to 0
			function none(type)
			{
				var frm = document.pmnt;

				if (type == <?php echo CASH;?>)
				{
					frm.cash.value = 0;
				}
				else if (type == <?php echo CHECK;?>)
				{
					frm.check.value = 0;
				}
				else if (type == <?php echo CREDIT;?>)
				{
					frm.credit.value = 0;
				}
				else if (type == <?php echo CHARGE;?>)
				{
					frm.charge.value = 0;
				}

				update_values();
			}

			// apply the remaining balance to one type
			function blnce(type)
			{
				var frm = document.pmnt;

				if (type == <?php echo CASH;?>)
				{
					frm.cash.value = (balance + (frm.cash.value * 1));
				}
				else if (type == <?php echo CHECK;?>)
				{
					frm.check.value = (balance + (frm.check.value * 1));
				}
				else if (type == <?php echo CREDIT;?>)
				{
					var val = (balance + (frm.credit.value * 1));
					if (val > total_all_credit)
					{
						val = total_all_credit;
					}
					frm.credit.value = val;
				}
				else if (type == <?php echo CHARGE;?>)
				{
					frm.charge.value = (balance + (frm.charge.value * 1));
				}

				update_values();
			}

			// update the variables and any hidden fields according to the text boxes
			function update_values()
			{
				var frm = document.pmnt;

				frm.additional.value = (frm.additional.value>0 ? format_price(frm.additional.value) : '');
				frm.cash_out.value   = (frm.cash_out.value>0 ? format_price(frm.cash_out.value) : '');
				frm.cash.value       = (frm.cash.value>0 ? format_price(frm.cash.value) : '');
				frm.check.value      = (frm.check.value>0 ? format_price(frm.check.value) : '');
				frm.credit.value     = (frm.credit.value>0 ? format_price(frm.credit.value) : '');
				frm.charge.value     = (frm.charge.value>0 ? format_price(frm.charge.value) : '');

				// pull the values from the form
				cash_out  = (frm.cash_out.value ? frm.cash_out.value : 0);
				cash_sale = (frm.cash.value ? frm.cash.value : 0);
				check     = (frm.check.value ? frm.check.value : 0);
				credit    = (frm.credit.value ? frm.credit.value : 0);
				charge    = (frm.charge.value ? frm.charge.value : 0);

				// stop javascript from treating numbers like strings
				cash_out  = (cash_out * 1);
				cash_sale = (cash_sale * 1);
				check     = (check * 1);
				credit    = (credit * 1);
				charge    = (charge * 1);

				additional   = parseFloat(frm.additional.value ? frm.additional.value : 0);
				avail_cash   = total_cash;
				avail_credit = total_all_credit;

				// reset the initial values
				balance = parseFloat((total_sale * 1) + (additional * 1));
				var subtotal = balance;

				// have they entered too much credit?
				if (credit > avail_credit)
				{
					credit = avail_credit;
				}

				// subtract any applied credit from the subtotal (credit isn't taxable)
				balance = truncate(balance - credit + 0.00001);
				var taxable = balance;

				// truncate thousands over, don't round
				with_tax = truncate(balance * (1 + (tax_perc / 100)));
				tax      = (with_tax - balance);

				total_price = (balance + tax);
				balance     = total_price;

				// if either cash (sale or trade), check, credit, or charge are greater than or equal to the balance, negate all others
				if (balance && cash_sale && truncate(cash_sale) >= truncate(balance))
				{
					cash_sale = balance;
					check = 0;
					credit = (balance==total_sale ? 0 : credit);
					charge = 0;
				}
				if (balance && check && truncate(check) >= truncate(balance))
				{
					check = balance;
					cash_sale = 0;
					credit = (balance==total_sale ? 0 : credit);
					charge = 0;
				}
				if (balance && charge && truncate(charge) >= truncate(balance))
				{
					charge = balance;
					cash_sale = 0;
					check = 0;
					credit = (balance==total_sale ? 0 : credit);
				}

				// adjust the available cash
				avail_cash = (avail_cash - cash_out);
				if (avail_cash < 0)
				{
					avail_cash = 0;
				}

				// if there is credit and the total is too high, subtract credit until
				//  the total is even
				var total = (cash_sale + check + credit + charge);
				if (credit > 0 && total > (balance + credit))
				{
					var diff = (total - balance);
					if (diff < credit)
					{
						credit = (credit - diff);
					}
				}
				avail_credit = (total_all_credit - credit);

				frm.tax.value        = tax;
				frm.cash.value       = (cash_sale>0 ? format_price(cash_sale) : '');
				frm.check.value      = (check>0 ? format_price(check) : '');
				frm.credit.value     = (credit>0 ? format_price(credit) : '');
				frm.charge.value     = (charge>0 ? format_price(charge) : '');

				total   = (cash_sale + check + credit + charge);
				balance = (total_price + credit - total + 0.000001); // DOES THE +0.00001 FIX THE ROUNDING PROBLEMS?
				if (balance < 0) { balance = 0; }

				// Logan's cash register just truncates (IE: 19.99*1.064=21.269, but register shows 21.26, not 21.27)
				balance = truncate(balance);

				// customer cash = (paid cash -
				cust_cash = cash_sale;
				if (cust_cash < 0) { cust_cash = 0; }

				lyrsubtotal.innerHTML  = '$' + format_price(subtotal);
				lyrtaxable.innerHTML   = '$' + format_price(taxable);
				lyrtax.innerHTML       = '$' + format_price(tax);
				lyrtotsale.innerHTML   = '$' + format_price(total_price);
				lyravailcash.innerHTML = '$' + format_price(avail_cash);
				lyravailcred.innerHTML = '$' + format_price(avail_credit);
				lyrbalance.innerHTML   = '$' + format_price(balance);
				lyrcustcash.innerHTML  = '$' + format_price(cust_cash);

				frm.set_credit.value = avail_credit;

				frm.btncmplt.disabled = (balance>0 ? true : false);
			}

			// if they have entered a charge amount and haven't selected a card, return false; otherwise, return true
			function verify(frm)
			{
				if (frm.charge.value > 0 && frm.card_type.selectedIndex == 0)
				{
					alert('Please select a card type');
					frm.card_type.focus();
					return false;
				}
				else
				{
					if (avail_cash > 0)
					{
						alert('The customer still has trade cash remaining.\n\nPlease give the customer $' + format_price(avail_cash));
					}
					if ((frm.charge.value == '' || frm.charge.value <= 0) && frm.card_type.selectedIndex > 0)
					{
						frm.card_type.selectedIndex = 0;
					}
					frm.btncmplt.disabled = true;
					return true;
				}
			}

			// set the customer's credit
			var orig_credit = -1;
			function set_credit(orig)
			{
				if (orig_credit == -1)
				{
					orig_credit = orig;
				}
				var newcredit = prompt("Please enter the customer's credit amount:",orig_credit);
				var credit = parseFloat(newcredit);
				if (newcredit != null && credit != orig_credit)
				{
					setcreditfrm.location = '/admin/pos/customers_setcredit.php?credit='+credit;
				}
			}
			function change_credit(credit)
			{
				document.location = '/admin/pos/invoice_payment.php';
			}
			function truncate(price)
			{
				return (Math.floor(price * 100) / 100);
			}

			/**
			* If they are paying out more than the trade cash, confirm
			*/
			function checkCashPaidOut(obj)
			{
				var paidout = (obj.value * 1);
				if (paidout > total_cash && !confirm('The Cash Paid Out amount you entered is more than the trade cash amount.\nAre you sure this is the amount you would like to pay out?\nOK = YES, Cancel = Use Cash Paid Out Amount'))
				{
					obj.value = total_cash;
				}
			}
		</script>

		<table border="0" cellspacing="3" cellpadding="0">
			<tr id="itmtableshow" style="display:block"><td align="center"><input type="button" value="Show Invoice Items &gt;" onclick="document.getElementById('itmtable').style.display='block';document.getElementById('itmtableshow').style.display='none';document.getElementById('itmtablehide').style.display='block'" class="btn" /></td></tr>
			<tr id="itmtablehide" style="display:none"><td align="center"><input type="button" value="&lt; Hide Invoice Items" onclick="document.getElementById('itmtable').style.display='none';document.getElementById('itmtableshow').style.display='block';document.getElementById('itmtablehide').style.display='none'" class="btn" /><br />&nbsp;</td></tr>
			<tr id="itmtable" style="display:none">
				<td align="center" width="600">
					<?php
					$show = array(SALE,TRADE,RETURNS);
					while (list($a,$type) = each($show))
					{
						$found = NO;
						while (list($a,$arr) = each($_SESSION['cust_items']))
						{
							if ($arr['ini_type'] == $type) { $found = YES; break; }
						}
						reset($_SESSION['cust_items']);

						if ($found)
						{
							$this->pg->outlineTableHead(600);
							?>
							<tr><td colspan="6" align="center" bgcolor="<?php echo $this->pg->color('table-head-darker');?>"><b><?php echo invType($type,YES);?> Items</b></td></tr>
							<tr>
								<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Title</b></td>
								<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>N/U</b></td>
								<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Cash/Cred</b></td>
								<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Platform</b></td>
								<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Qty</b></td>
								<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Price</b></td>
							</tr>
							<?php
							$shown = -1;
							$total = 0;
							while (list($a,$arr) = each($_SESSION['cust_items']))
							{
								if ($arr['ini_type'] == $type)
								{
									$shown++;
									$bg = (($shown%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));
									$total += $arr['ini_price'];

									?>
									<tr>
										<td bgcolor="<?php echo $bg;?>" width="100%"><?php echo $arr['ini_title'];?></td>
										<td bgcolor="<?php echo $bg;?>"><?php echo ($arr['ini_newused']==ITEM_NEW?'New':'Used');?></td>
										<td bgcolor="<?php echo $bg;?>"><?php echo ($arr['ini_type']==TRADE||$arr['ini_type']==RETURNS?($arr['ini_trade_type']==CASH?'Cash':'Credit'):'&nbsp;');?></td>
										<td bgcolor="<?php echo $bg;?>"><?php echo $arr['ini_platform_name'];?></td>
										<td bgcolor="<?php echo $bg;?>"><?php echo $arr['ini_qty'];?></td>
										<td align="right" bgcolor="<?php echo $bg;?>">$<?php echo number_format($arr['ini_price'],2);?></td>
									</tr>
									<?php
								}
							}
							reset($_SESSION['cust_items']);

							?>
							<tr>
								<td align="right" colspan="5" bgcolor="<?php echo $this->pg->color('table-head');?>"><b><?php echo invType($type,YES);?> Total:</b></td>
								<td align="right" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>$<?php echo number_format($total,2);?></b></td>
							</tr>
							<?php

							$this->pg->outlineTableFoot();
							?><p /><?php
						}
					}
					?>
				</td>
			</tr>
			<tr><td colspan="2">&nbsp;</td></tr>
		</table>

		<?php
		$w = 200; // table widths
		$h = 95; // table heights
		?>
		<table border="0" cellspacing="3" cellpadding="0" width="600">
			<form method="post" action="/admin/pos/invoice_payment.php" name="pmnt" onsubmit="return verify(this)">
			<input type="hidden" name="act" value="complete">
			<input type="hidden" name="tax" value="<?php echo $tax;?>">
			<tr>
				<td align="center" valign="top" width="200">
					<?php
					$this->pg->outlineTableHead($w,$h);
					?>
					<tr bgcolor="<?php echo $this->pg->color('table-label');?>">
						<td valign="top">
							<table border="0" cellspacing="0" cellpadding="2" width="100%">
								<tr>
									<td><b>Sale Total:</b></td>
									<td align="right">
										<b>$<?php echo number_format($total_sale,2);?></div></b>
									</td>
								</tr>
								<tr>
									<td><b>+ Additional Charges:</b></td>
									<td align="right">
										$<input tabindex="1" type="text" size="7" name="additional" onfocus="this.select()" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right" onblur="this.value=format_price(this.value,false);update_values()">
									</td>
								</tr>
								<tr><td colspan="2" bgcolor="#000000"></td></tr>
								<tr>
									<td><b>Subtotal:</b></td>
									<td align="right">
										<b><div id="lyrsubtotal">$<?php echo number_format($total_sale,2);?></div></b>
									</td>
								</tr>
							</table>
						</td>
					</tr>
					<?php
					$this->pg->outlineTableFoot();
					?>
				</td>
				<td align="center" valign="top" width="200">
					<?php
					$this->pg->outlineTableHead($w,$h);
					?>
					<tr bgcolor="<?php echo $this->pg->color('table-label');?>">
						<td valign="top">
							<table border="0" cellspacing="0" cellpadding="2" width="100%">
								<tr>
									<td><b>Taxable:</b></td>
									<td align="right"><b><div id="lyrtaxable">$<?php echo number_format($total_sale,2);?></div></b></td>
								</tr>
								<tr>
									<td><b>+ Tax:</b> (<?php echo trim($tax_perc,'0');?>%)</td>
									<td align="right"><b><span id="lyrtax">$<?php echo number_format($tax,2);?></span></b></td>
								</tr>
								<tr><td colspan="2" bgcolor="#000000"></td></tr>
								<tr>
									<td><b>Total:</b></td>
									<td align="right"><b><div id="lyrtotsale">$<?php echo number_format(($total_sale+$tax),2);?></div></b></td>
								</tr>
							</table>
						</td>
					</tr>
					<?php
					$this->pg->outlineTableFoot();
					?>
				</td>
				<td align="center" valign="top" width="200">
					<?php
					$this->pg->outlineTableHead($w,$h);
					?>
					<tr bgcolor="<?php echo $this->pg->color('table-label');?>">
						<td valign="top">
							<table border="0" cellspacing="0" cellpadding="2" width="100%">
								<tr>
									<td><b>Total Trade Cash:</b></td>
									<td align="right"><b>$<?php echo number_format($avail_cash,2);?></b></td>
								</tr>
								<tr>
									<td>
										<b>- Cash Paid Out:</b>
									</td>
									<td align="right">
										<input type="hidden" name="cash_out" value="<?php echo $avail_cash;?>" />
										<b>$<?php echo number_format($avail_cash,2);?></b>
									</td>
								</tr>
								<tr><td colspan="2" bgcolor="#000000"></td></tr>
								<tr>
									<td><b>Trade Cash Available:</b></td>
									<td align="right"><b><div id="lyravailcash">$0.00</div></b></td>
								</tr>
								<tr>
									<input type="hidden" name="set_credit" value="<?php echo ($total_credit+$cust_credit);?>">
									<td><b>Trade Credit Available:</b><?php echo ($cust_credit>0?' <font color="red">*</font>':'');?></td>
									<td align="right"><b><div id="lyravailcred">$<?php echo number_format(($total_credit+$cust_credit),2);?></div></b></td>
								</tr>
								<tr>
									<td colspan="2" align="center">
										<a href="javascript:set_credit(total_cust_credit)"><img src="/images/setcredit.gif" width="73" height="11" border="0" style="vertical-align:bottom" /></a>
										<iframe name="setcreditfrm" src="/admin/pos/customers_setcredit.php" width="1" height="1" frameborder="0" marginwidth="0" marginheight="0">
											Your browser does not support iframes. Please upgrade.
										</iframe>
									</td>
								</tr>
							</table>
						</td>
					</tr>
					<?php
					$this->pg->outlineTableFoot();
					?>
				</td>
			</tr>
		</table>
		<?php

		function anb($type)
		{
			?>
			<table border="0" cellspacing="0" cellpadding="0">
				<tr>
					<td>
						<a href="javascript:all(<?php echo $type;?>)"><img src="/images/invoice/anb_all.gif" border="0" style="vertical-align:top"></a>
					</td>
				</tr>
				<tr>
					<td>
						<a href="javascript:none(<?php echo $type;?>)"><img src="/images/invoice/anb_none.gif" border="0" style="vertical-align:top"></a>
					</td>
				</tr>
				<tr>
					<td>
						<a href="javascript:blnce(<?php echo $type;?>)"><img src="/images/invoice/anb_balance.gif" border="0" style="vertical-align:top"></a>
					</td>
				</tr>
			</table>
			<?php
		}

		?><p /><?php
		$this->pg->outlineTableHead(300);
		?>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>">
				<table border="0" cellspacing="4" cellpadding="0" width="100%">
					<tr>
						<td><b>Credit Used:</b></td>
						<td align="right">$<input tabindex="3" type="text" size="7" name="credit" onfocus="this.select()" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right" onblur="this.value=format_price(this.value,false);update_values()"></td>
						<td valign="middle"><?php echo anb(CREDIT);?></td>
					</tr>
					<tr><td colspan="3" bgcolor="#CCCCCC"></td></tr>
					<tr>
						<td><b>Cash Paid:</b></td>
						<td align="right">
							$<input tabindex="4" type="text" size="7" name="cash" onfocus="this.select()" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right" onblur="this.value=format_price(this.value,false);update_values()">
						</td>
						<td valign="middle"><?php echo anb(CASH);?></td>
					</tr>
					<tr><td colspan="3" bgcolor="#CCCCCC"></td></tr>
					<tr>
						<td><b>Check Paid:</b></td>
						<td align="right">$<input tabindex="5" type="text" size="7" name="check" onfocus="this.select()" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right" onblur="this.value=format_price(this.value,false);update_values()"></td>
						<td valign="middle"><?php echo anb(CHECK);?></td>
					</tr>
					<tr><td colspan="3" bgcolor="#CCCCCC"></td></tr>
					<tr>
						<td><b>Charged:</b></td>
						<td align="right">
							<select tabindex="6" name="card_type" size="1" style="vertical-align:middle">
								<option value="">- Card Type -</option>
								<?php
								$cards = array(
									'Visa',
									'MasterCard',
									'American Express',
									'Discover',
									'Debit Card',
									'Other'
								);
								while (list($a,$card) = each($cards))
								{
									$sel = ($card=='Visa' ? ' selected="selected"' : '');
									?>
									<option value="<?php echo $card;?>"<?php echo $sel;?>><?php echo $card;?></option>
									<?php
								}
								?>
							</select>
							$<input tabindex="7" type="text" size="7" name="charge" onfocus="this.select()" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right" onblur="this.value=format_price(this.value,false);update_values()">
						</td>
						<td valign="middle"><?php echo anb(CHARGE);?></td>
					</tr>
				</table>
			</td>
		</tr>
		<tr bgcolor="<?php echo $this->pg->color('table-label');?>">
			<td>
				<table border="0" cellspacing="0" cellpadding="2" width="100%">
					<tr>
						<td><b>Remaining Balance:</b></td>
						<td align="right"><b><div id="lyrbalance">$<?php echo number_format($balance,2);?></div></b></td>
					</tr>
					<tr>
						<td><b>Required Cash From Customer:</b></td>
						<td align="right"><b><div id="lyrcustcash">$0.00</div></b></td>
					</tr>
				</table>
			</td>
		</tr>
		<?php
		$this->pg->outlineTableFoot();
		?>
		<p />
		<table border="0" cellspacing="2" cellpadding="0">
			<tr>
				<td valign="top"><input tabindex="8" type="button" value="&lt; Return to Invoice" onclick="document.location='/admin/pos/invoice.php'" class="btn"></td>
				<td align="center" valign="top"><input tabindex="9" type="submit" value="Complete Invoice &gt;"<?php echo ($total_sale?' disabled="true"':'');?> id="btncmplt" class="btn"><br /><img src="/images/blank.gif" width="1" height="1" /></td>
			</tr>
		</table>
		</form>

		<p />

		<span class="note">
			<b>Note:</b> After you press <b>Complete Invoice</b>, the rankings will be updated.<br />
			It may take up to a minute to update them, so please be patient!
		</span>

		<?php
		if ($cust_credit > 0)
		{
			?>
			<p />
			<font size="1">
				<font color="red">*</font> Available credit is $<?php echo number_format($total_credit,2);?> invoice credit plus $<?php echo number_format($cust_credit,2);?> customer credit.
			</font>
			<?php
		}

		$this->pg->addOnload('window.scrollBy(0,1000)');
		$this->pg->addOnload('document.pmnt.additional.focus()');
	} // end function show_payment_options
} // end class invoice_payment

?>