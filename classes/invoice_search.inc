<?php
/**
 * Point-of-sale invoice class - searching
 *
 * @author Scott Carpenter <s-carp@comcast.net>
 * @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
 * @version 10/05/2012
 * @package Fun Unlimited
 */

class invoice_search extends invoice_payment
{
	/**
	* Output the invoice search form
	* @param string $action Form action
	*/
	function search_form($action)
	{
		?>
		<form method="post" action="<?php echo $action;?>" name="ordsrch" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="dosearch">
		<?php
		$this->pg->outlineTableHead();
		?>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Begin Date:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="criteria[begdate]" size="10" value="<?php echo @$this->criteria['begdate'];?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>End Date:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="criteria[enddate]" size="10" value="<?php echo @$this->criteria['enddate'];?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Employee:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><select name="criteria[employeeID]" size="1"><option value=""></option><?php
				$empl = new employees($this->pg);
				$empl->get_employees(YES);

				while (list($employeeID,$arr) = each($empl->employees))
				{
					if ($employeeID == @$this->criteria['employeeID']) { $s = ' selected="selected"'; } else { $s = ''; }
					?><option value="<?php echo $employeeID;?>"<?php echo $s;?>><?php echo $arr['emp_lname'];?>, <?php echo $arr['emp_fname'];?></option><?php
				}
			?></select></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Locale:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>">
				<?php
				if (!isset($this->criteria['locale'])) { $this->criteria['locale'] = "BOTH"; }
				$c1 = (@$this->criteria['locale']=="INSTORE"?' checked="checked"':'');
				$c2 = (@$this->criteria['locale']=="ONLINE"?' checked="checked"':'');
				$c3 = (@$this->criteria['locale']=="BOTH"?' checked="checked"':'');
				?>
				<input type="radio" name="criteria[locale]" id="li" value="<?php echo INSTORE;?>" style="border-width:0;vertical-align:middle"<?php echo $c1;?>> <label style="cursor:hand" for="li">In-Store</label>
				<input type="radio" name="criteria[locale]" id="lo" value="<?php echo ONLINE;?>" style="border-width:0;vertical-align:middle"<?php echo $c2;?>> <label style="cursor:hand" for="lo">Online</label>
				<input type="radio" name="criteria[locale]" id="lb" value="<?php echo BOTH;?>" style="border-width:0;vertical-align:middle"<?php echo $c3;?>> <label style="cursor:hand" for="lb">Both</label>
			</td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Completed:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>">
				<?php
				if (!isset($this->criteria['completed'])) { $this->criteria['completed'] = "BOTH"; }
				$c1 = (@$this->criteria['completed']==YES?' checked="checked"':'');
				$c2 = (@$this->criteria['completed']==NO?' checked="checked"':'');
				$c3 = (@$this->criteria['completed']==BOTH?' checked="checked"':'');
				?>
				<input type="radio" name="criteria[completed]" id="cy" value="<?php echo YES;?>" style="border-width:0;vertical-align:middle"<?php echo $c1;?>> <label style="cursor:hand" for="cy">YES</label>
				<input type="radio" name="criteria[completed]" id="cn" value="<?php echo NO;?>" style="border-width:0;vertical-align:middle"<?php echo $c2;?>> <label style="cursor:hand" for="cn">No</label>
				<input type="radio" name="criteria[completed]" id="cb" value="<?php echo BOTH;?>" style="border-width:0;vertical-align:middle"<?php echo $c3;?>> <label style="cursor:hand" for="cb">Either</label>
			</td>
		</tr>
		<?php

		$this->pg->outlineTableFoot();
		?>
		<p />
		<input type="submit" value="Find Invoices &gt;" class="btn"> <input type="button" value="Clear Form &gt;" class="btn" onclick="clearfrm(this.form)">
		</form>

		<script type="text/javascript">
			function verify(frm) { return true; }

			function clearfrm(frm)
			{
				frm.elements['criteria[begdate]'].value = '';
				frm.elements['criteria[enddate]'].value = '';
				frm.elements['criteria[employeeID]'].selectedIndex = 0;
				frm.lb.checked = true;
				frm.cb.checked = true;
			}
		</script>
		<?php

		$this->pg->addOnload("document.ordsrch.elements['criteria[begdate]'].focus()");
	}

	/**
	* Perform the invoice search
	*/
	function search()
	{
		global $db;

		$_SESSION['cust_invoice_criteria'] = $this->criteria;
		if (!isset($this->criteria['locale'])) { $this->criteria['locale'] = "BOTH"; }
		if (!isset($this->criteria['completed'])) { $this->criteria['completed'] = "YES"; }

		$from = array('invoices','customers');
		$where = array();

		if (strlen(@$this->criteria['begdate'])) { $begdate = strtotime($this->criteria['begdate']); } else { $begdate = ''; }
		if (strlen(@$this->criteria['enddate'])) { $enddate = strtotime($this->criteria['enddate']); } else { $enddate = ''; }

		if (strlen($begdate) && $begdate != -1) { $where[] = "inv_time >= $begdate"; }
		if (strlen($enddate) && $enddate != -1) { $where[] = "inv_time <= $enddate"; }
		if ($this->criteria['locale'] != BOTH) { $where[] = "inv_locale=".$this->criteria['locale']; }
		if ($this->criteria['completed'] != BOTH) { $where[] = "inv_completed=".$this->criteria['completed']; }
		if (strlen(@$this->criteria['employeeID'])) { $from[] = 'employees'; $where[] = "inv_employeeID=".$this->criteria['employeeID']; $where[] = "inv_employeeID=emp_employeeID"; }

		$where[] = "inv_customerID=".$this->customerID;
		$where[] = "inv_customerID=cus_customerID";
		$where[] = "inv_storeID=".$_SESSION['storeID'];

		$sql = "SELECT * FROM ".implode(',',$from)." WHERE ";
		$sql .= implode(' AND ',$where);
		$sql .= ' ORDER BY inv_time';

		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		$this->results = array();
		while ($row = mysql_fetch_assoc($result)) { $this->results[] = $row; }
	}

	/**
	* Output the invoice search results
	* @param string $action Form action
	* @param integer $openifone Reopen the invoice if there is only one match? [optional, default NO]
	*/
	function search_results($action,$openifone=NO)
	{
		global $db;

		if (count($this->results) > 1 || (count($this->results) == 1 && !$openifone) || !count($this->results)) { $this->pg->head('Search Previous Invoices',YES); }

		if (!count($this->results))
		{
			?>
			Your search returned no results.
			<p />
			<?php
			$this->search_form($action);
		}
		elseif (count($this->results) == 1 && $openifone)
		{
			// select the invoice and show the form
			$this->set_invoiceID($this->results[0]['inv_invoiceID']);

			$this->pg->showUpdating('Invoice successfully reopened...','/admin/pos/invoice.php?act=view');
		}
		else
		{
			// output the results

			$empl = new employees($this->pg);
			$empl->get_employees("BOTH");

			$invoiceID = @$_SESSION['cust_invoiceID'];
			if (!strlen($invoiceID)) { $invoiceID = 0; }

			?>
			<input type="button" value="&lt; Search Again" onclick="document.location='/admin/pos/invoice.php?act=search'" class="btn" />
			<p />
			<?php
			if (@$_SESSION['cust_invoiceID'])
			{
				?>
				<b>Note:</b> Since you currently have an open invoice,<br />
				you can only view the details of the below invoices.
				<p />
				<?php
			}
			?>

			<script type="text/javascript">
				var selID = 0;
				function sel(id)
				{
					selID = id;

					var obj = eval('document.ordresults.ord'+id);
					obj.checked = true;

					//document.ordresults.v.disabled = false;
					document.ordresults.r.disabled = (isNC(id)&&!<?php echo $invoiceID;?>?false:true);
					document.ordresults.d.disabled = (isNC(id)&&!<?php echo $invoiceID;?>?false:true);
				}

				var not_complete = new Array();
				function addNC(id) { not_complete[not_complete.length] = id; }
				function isNC(id) { return in_array(id,not_complete); }

				function go(where)
				{
					if (!selID) { alert('You must select an invoice.'); }
					else
					{
						if (where != 'delete' || (where == 'delete' && confirm('Are you SURE you want to delete the selected invoice?') == true))
						{
							var frm = document.ordresults;
							frm.act.value = where;
							frm.submit();
						}
					}
				}

				var goto_payment = new Array();
				var goto_complete = new Array();
			</script>

			<form method="post" action="<?php echo $action;?>" name="ordresults">
			<input type="hidden" name="act" value="select">
			<?php echo $this->pg->outlineTableHead();?>
			<tr bgcolor="<?php echo $this->pg->color('table-head');?>">
				<td>&nbsp;</td>
				<td><b>Customer</b></td>
				<td><b>Employee</b></td>
				<td><b>Details</b></td>
			</tr>

			<?php

			$val = new validate();

			while (list($a,$arr) = each($this->results))
			{
				$bg = (($a%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

				if ($arr['inv_completed'] == NO) { $this->update_prices($arr['inv_invoiceID']); }
				$additional = $arr['inv_additional'];

				$emparr = $empl->get_employee($arr['inv_employeeID']);
				if (!count($emparr)) { $employee = '- None -'; } else { $employee = "{$emparr['emp_lname']}, {$emparr['emp_fname']}"; }

				$address = $arr['cus_address'];
				$citystatezip = trim("{$arr['cus_city']}, {$arr['cus_state']} ".$this->pg->format('zip',$val->strip(@$arr['cus_zip'])),' ,');
				if (strlen($address)) { $address .= '<br />'; }
				if (strlen($citystatezip)) { $citystatezip .= '<br />'; }

				if ($arr['inv_completed'] == NO) { ?><script type="text/javascript">addNC(<?php echo $arr['inv_invoiceID'];?>);</script><?php }
				?>
				<label for="ord<?php echo $arr['inv_invoiceID'];?>">
				<tr bgcolor="<?php echo $bg;?>" style="cursor:hand">
					<td><input type="radio" name="invoiceID" value="<?php echo $arr['inv_invoiceID'];?>" id="ord<?php echo $arr['inv_invoiceID'];?>" class="nb" onclick="sel(<?php echo $arr['inv_invoiceID'];?>)"></td>
					<td>
						<b><?php echo $arr['cus_fname'].' '.$arr['cus_lname'];?></b><br />
						<?php echo $address.$citystatezip;?>
						<?php echo $this->pg->format('phone',$val->strip(@$arr['cus_phone']));?>
					</td>
					<td><?php echo $employee;?></td>
					<td>
						<?php
						$total_qty = 0;
						$ca_qty = 0;
						$cr_qty = 0;
						$sa_qty = 0;
						$ret_qty = 0;

						$cash_total = 0;
						$credit_total = 0;
						$sale_total = 0;
						$return_total = 0;
						$sums = array(
							array('cash_total','ini_type='.TRADE.' AND ini_trade_type='.CASH),
							array('credit_total','ini_type='.TRADE.' AND ini_trade_type='.CREDIT),
							array('sale_total','ini_type='.SALE),
							array('return_total','ini_type='.RETURNS)
						);

						while (list($a,list($var,$where)) = each($sums))
						{
							$sql = "SELECT SUM(ini_price) AS total FROM invoice_items WHERE ini_invoiceID=".$arr['inv_invoiceID']." AND $where";
							$result = mysql_query($sql,$db);
							$this->error->mysql(__FILE__,__LINE__);

							$row = mysql_fetch_assoc($result);
							${$var} = $row['total'];
						}

						$sql = "SELECT ini_type,ini_trade_type FROM invoice_items WHERE ini_invoiceID=".$arr['inv_invoiceID'];
						$result = mysql_query($sql,$db);
						$this->error->mysql(__FILE__,__LINE__);
						while ($row = mysql_fetch_assoc($result))
						{
							$total_qty++;

							if ($row['ini_type'] == SALE) { $sa_qty++; }
							elseif ($row['ini_type'] == TRADE && $row['ini_trade_type'] == CASH) { $ca_qty++; }
							elseif ($row['ini_type'] == TRADE && $row['ini_trade_type'] == CREDIT) { $cr_qty++; }
							elseif ($row['ini_type'] == RETURNS) { $ret_qty++; }
						}

						$output = array();
						$output[] = array('Total Items',$total_qty);

						$sale_total += $additional;

						if ($sale_total > 0) { $output[] = array('Sale Total',$sa_qty.' item'.($sa_qty!=1?'s':'').' / $'.number_format($sale_total,2)); }
						if ($cash_total > 0) { $output[] = array('Trade - Cash Total',$ca_qty.' item'.($ca_qty!=1?'s':'').' / $'.number_format($cash_total,2)); }
						if ($credit_total > 0) { $output[] = array('Trade - Credit Total',$cr_qty.' item'.($cr_qty!=1?'s':'').' / $'.number_format($credit_total,2)); }
						if ($return_total > 0) { $output[] = array('Returns Total',$ret_qty.' item'.($ret_qty!=1?'s':'').' / $'.number_format($return_total,2)); }
						if ($additional > 0) { $output[] = array('Additional','$'.number_format($additional,2)); }

						$output[] = array('Completed?',($arr['inv_completed']==YES?'YES':'No'));
						$output[] = array('Created',date('M/d/Y h:ia',$arr['inv_time']));
						if ($arr['inv_completed'] == YES)
						{
							$output[] = array('Completed',date('M/d/Y h:ia',$arr['inv_completedtime']));
						}

						?><script type="text/javascript"><?php
						if ($sale_total > 0) { ?>goto_payment[goto_payment.length]=<?php echo $arr['inv_invoiceID'];?><?php }
						else { ?>goto_complete[goto_complete.length]=<?php echo $arr['inv_invoiceID'];?><?php }
						?></script><?php

						?>
						<table border="0" cellspacing="1" cellpadding="0" width="100%">
							<label for="ord<?php echo $arr['inv_invoiceID'];?>">
							<?php
							while (list($a,list($k,$v)) = each($output))
							{
								?>
								<tr style="cursor:hand">
									<td><b><?php echo $k;?>:</b></td>
									<td>&nbsp;</td>
									<td><?php echo $v;?></td>
								</tr>
								<?php
							}
							?>
							</label>
						</table>
					</td>
				</tr>
				</label>
				<?php
			}
			?>

			<?php echo $this->pg->outlineTableFoot();?>

			<p />
			<font size="1"><b>Note:</b> The sale total includes any additional charges.</font>
			<p />

			<input type="button" value="View Invoice Details &gt;" id="v" disabled="disabled" onclick="go('details')" class="btn">
			<input type="button" value="Reopen Invoice &gt;" id="r" disabled="disabled" onclick="go('reopen')" class="btn">
			<input type="button" value="Delete Invoice &gt;" id="d" disabled="disabled" onclick="go('delete')" class="btn">
			</form>
			<?php
		}

		if (count($this->results) > 1 || (count($this->results) == 1 && !$openifone) || !count($this->results)) { $this->pg->foot(); }
	}

	/**
	 * Get all matching invoice items
	 * Right now this is only used with Utah Pawn files and serial number history, so it's tailored to that
	 * @param	integer	$type
	 * @param	integer	$from
	 * @param	integer	$to
	 * @param	string	$serial_number	[optional, default '']
	 * @return	array
	 */
	function getInvoiceItems($type, $from, $to, $serial_number='')
	{
		set_time_limit(120);

		global $db;

		$where = array(
			'inv_invoiceID=ini_invoiceID',
			'inv_customerID=cus_customerID',
			'inv_completed=1',
			'inv_storeID=' . $_SESSION['storeID'],
			'ini_itemID=itm_itemID',
		);

		$base_count = count($where);

		if ($type)
		{
			$where[] = "ini_type=$type";
		}

		if ($from && $to)
		{
			$where[] = "inv_completedtime BETWEEN $from AND $to";
		}

		if ($serial_number)
		{
			$where[] = "ini_serial_number='" . mysql_real_escape_string($serial_number) . "'";
		}

		if (count($where) == $base_count)
		{
			die('ERROR: No criteria provided to ' . __FUNCTION__);
		}

		$sql = "
			SELECT
				inv_invoiceID,
				inv_completedtime,

				invoice_items.*,

				itm_title,
				itm_description,
				itm_upc,

				cus_fname,
				cus_lname,
				cus_address,
				cus_city,
				cus_state,
				cus_zip,
				cus_phone,
				cus_dob,
				cus_gender,
				cus_ethnicity,
				cus_hair_color,
				cus_eye_color,
				cus_height,
				cus_weight,
				cus_idnumber,
				cus_idstate,

				emp_fname,
				emp_lname
			FROM
				invoices
				LEFT OUTER JOIN
					employees
					ON (emp_employeeID=inv_employeeID),
				invoice_items,
				items,
				customers
			WHERE
				" . implode(' AND ', $where) . "
			ORDER BY
				inv_completedtime,
				ini_timeadded,
				ini_idx
		";

		$result = mysql_query($sql, $db);

		$items = array();

		while ($row = mysql_fetch_assoc($result))
		{
			$items[$row['ini_invoiceitemID']] = $row;
		}

		return $items;
	} // end function getInvoiceItems
} // end class invoice_search

/* END OF FILE */
/* Location: ./classes/invoice_search.inc */