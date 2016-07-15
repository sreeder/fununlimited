<?php
/**
* Point-of-sale invoice class - view invoice
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 10/05/2012
* @package Fun Unlimited
*/

class invoice_view extends invoice_search
{
	/**
	* Output the invoice page w/buttons to add items/etc
	*/
	function view()
	{
		global $db;

		$this->t->startTimer('invoice_view_class');

		//$this->pg->pageHead('Current Invoice');

		//unset($_SESSION['cust_items']);
		if (!isset($_SESSION['cust_items'])) { $_SESSION['cust_items'] = array(); }

		// load the cash/credit percentages, box discounts, and minimum prices
		$ccp = new ccpercs($this->pg);
		$bod = new box_discounts($this->pg);
		$mp = new min_prices($this->pg);
		$pcd = new percopy_discounts($this->pg);

		$highlight_color = '#FFDDDD';

		if (NO) { echo "<p />{$_SESSION['search_sql']}<p />"; }

		$dofocus = YES;
		if ($dofocus)
		{
			$focus_type = (isset($_SESSION['invoice_focus_type']) ? $_SESSION['invoice_focus_type'] : SALE);
			$this->pg->addOnload("Field.activate('upctitle" . $focus_type . "')");
		}

		if (isset($_SESSION['invoice_removed_items']) && count($_SESSION['invoice_removed_items']))
		{
			$this->pg->status('The following items have been removed from the invoice:<p />'.implode('<br />',$_SESSION['invoice_removed_items']).'<p /><b>Note:</b> This usually happens because your quantities have changed and these items are no longer in stock.');
			unset($_SESSION['invoice_removed_items']);
		}

		$trade_items = array();
		$sale_items = array();
		$return_items = array();
		reset($_SESSION['cust_items']);
		while (list($a,$arr) = each($_SESSION['cust_items']))
		{
			if (count($arr))
			{
				if ($arr['ini_type'] == TRADE) { $trade_items[] = $arr; }
				elseif ($arr['ini_type'] == SALE) { $sale_items[] = $arr; }
				elseif ($arr['ini_type'] == RETURNS) { $return_items[] = $arr; }
			}
		}
		reset($_SESSION['cust_items']);

		$this->t->startTimer('output_table');
		$forms = array(
			SALE => array(
				'title'  => 'Sale Items',
				'button' => 'Enter Sale Items',
				'items'  => $sale_items
			),
			TRADE => array(
				'title'  => 'Trade Items',
				'button' => 'Enter Trade Items',
				'items'  => $trade_items
			),
			RETURNS => array(
				'title'  => 'Return Items',
				'button' => 'Enter Return Items',
				'items'  => $return_items
			)
		);

		?>
		<script type="text/javascript">
			// output the constants in javascript
			<?php
			while (list($constant,$number) = each($_SESSION['flags']))
			{
				echo strtoupper($constant) . "=$number;";
			}
			?>

			/// data idx arrays (used with functions in /scripts/invoice_dynamic.js)
			var sale_itemIDs = [];
			var trade_itemIDs = [];
			var return_itemIDs = [];

			var sale_used_prices = [];
			var sale_new_prices = [];
			var trade_manual_price = []; // YES/NO flag
			var trade_used_prices = [];
			var trade_new_prices = [];
			var trade_cash_prices = [];
			var trade_credit_prices = [];
			var return_used_prices = [];
			var return_new_prices = [];
			var sale_prices = [];
			var trade_prices = [];
			var return_prices = [];
			var return_cash_prices = [];
			var return_credit_prices = [];

			var sale_newused = [];
			var return_newused = [];

			var trade_types = [];
			var return_types = [];
			var temptype = 0;

			var sale_box = [];
			var trade_box = [];
			var return_box = [];

			var trade_condition = [];

			var sale_serial_numbers = [];
			var trade_serial_numbers = [];
			var return_serial_numbers = [];

			var return_opened = [];
			var return_purchdates = [];
			var return_purchprices = [];
			var return_charged = [];
			var return_occasion = [];
			var return_occasion_date = [];

			var sale_titles = [];
			var trade_titles = [];
			var return_titles = [];

			var all_platformIDs = [];
			var sale_platformIDs = [];
			var trade_platformIDs = [];
			var return_platformIDs = [];
			var sale_platform_abbrs = [];
			var trade_platform_abbrs = [];
			var return_platform_abbrs = [];
			var sale_platform_names = [];
			var trade_platform_names = [];
			var return_platform_names = [];

			var cash_percs = [];
			var credit_percs = [];

			var boxdohalf = [];

			var cond_new = [];
			var cond_fair = [];
			var cond_poor = [];
			var cond_percent = [];

			var min_price = [];

			var percopy = [];
			var percopy_percent = [];

			var qty_itemIDs = [];
			var qty_new_orig = [];
			var qty_used_orig = [];
			var qty_total = [];
			var qty_new = [];
			var qty_used = [];
			var qty_new_counting = [];
			var qty_used_counting = [];
			var sale_invoice_qtys = [];
			var trade_invoice_qtys = [];
			var return_invoice_qtys = [];

			var sale_percent = [];
			var sale_milestone = [];
			var trade_milestone = [];

			var non_percopy = [];

			var pricing_lines = [];
			var sale_showpercentoff = [];

			var type<?php echo SALE;?>_idxs = [];
			var type<?php echo TRADE;?>_idxs = [];
			var type<?php echo RETURNS;?>_idxs = [];

			var timeadded<?php echo SALE;?> = [];
			var timeadded<?php echo TRADE;?> = [];
			var timeadded<?php echo RETURNS;?> = [];

			var sale_box<?php echo NOBOX;?>discount = [];
			var sale_box<?php echo STOREBOX;?>discount = [];
			var trade_box<?php echo NOBOX;?>discount = [];
			var box_percent = [];

			var sale_images_box = [];
			var trade_images_box = [];
			var return_images_box = [];
			var sale_images_nobox = [];
			var trade_images_nobox = [];
			var return_images_nobox = [];

			var max_title_length = <?php echo MAX_TITLE;?>;

			<?php
			$all_platformIDs = array();
			$all_itemIDs = array();
			if (is_array($_SESSION['cust_items'])) // !!! TEMPORARY FIX !!! WHY IS CUST_ITEMS BEING SET TO A NON-ARRAY?!
			{
				while (list($a,$arr) = each($_SESSION['cust_items']))
				{
					if (!in_array($arr['ini_platformID'],$all_platformIDs)) { $all_platformIDs[] = $arr['ini_platformID']; }
					if (!in_array($arr['ini_itemID'],$all_itemIDs)) { $all_itemIDs[] = $arr['ini_itemID']; }
				}
				reset($_SESSION['cust_items']);
			}
			else { ?>alert('CUST_ITEMS NOT ARRAY - TELL SCOTT!');<?php }

			$all_platformIDs = array();
			$pla = new platforms($pg,0);
			$pla->set_item('platforms');
			while (list($a,$arr) = each($pla->values)) { $all_platformIDs[] = $arr[0]; }

			echo $this->getQtyJS($all_itemIDs,NO);

			$pg = new admin_page();
			$ccp = new ccpercs($pg);
			$bod = new box_discounts($pg);
			$cod = new condition_discounts($pg);
			$mp = new min_prices($pg);
			$pcd = new percopy_discounts($pg);

			$def_cash      = $ccp->percs[0][0];
			$def_credit    = $ccp->percs[0][0];
			$def_snbd      = $bod->discounts[0][0];
			$def_ssbd      = $bod->discounts[0][1];
			$def_tnbd      = $bod->discounts[0][2];
			$def_dh        = $bod->discounts[0][3];
			$def_bd_perc   = $bod->discounts[0][4];
			$def_cn        = $cod->discounts[0][0];
			$def_cf        = $cod->discounts[0][1];
			$def_cp        = $cod->discounts[0][2];
			$def_cp_perc   = $cod->discounts[0][3];
			$def_min_price = $mp->prices[0];

			$i = -1;
			while (list($a,$platformID) = each($all_platformIDs))
			{
				$i++;

				$ca = @$ccp->percs[$platformID][0];
				$cr = @$ccp->percs[$platformID][0];
				if (!strlen($ca)) { $ca = $def_cash; }
				if (!strlen($cr)) { $cr = $def_credit; }
				$ca = sprintf('%0.3f',($ca/2));

				$snbd       = @$bod->discounts[$platformID][0];
				$ssbd       = @$bod->discounts[$platformID][1];
				$tnbd       = @$bod->discounts[$platformID][2];
				$dh         = @$bod->discounts[$platformID][3];
				$bd_percent = @$bod->discounts[$platformID][4];
				if (!strlen($snbd)) { $snbd = $def_snbd; }
				if (!strlen($ssbd)) { $ssbd = $def_ssbd; }
				if (!strlen($tnbd)) { $tnbd = $def_tnbd; }
				if (!strlen($dh) || ($dh != YES && $dh != NO)) { $dh = $def_dh; }
				if (!isset($bod->discounts[$platformID]))
				{
					$bd_percent = $def_bd_perc;
				}

				$cn         = @$cod->discounts[$platformID][0];
				$cf         = @$cod->discounts[$platformID][1];
				$cp         = @$cod->discounts[$platformID][2];
				$cd_percent = @$cod->discounts[$platformID][3];
				if (!strlen($cn)) { $cn = $def_cn; }
				if (!strlen($cf)) { $cf = $def_cf; }
				if (!strlen($cp)) { $cp = $def_cp; }
				if (!isset($cod->discounts[$platformID]))
				{
					$cd_percent = $def_cp_perc;
				}

				$min_price = @$mp->prices[$platformID];
				if (!strlen($min_price)) { $min_price = $def_min_price; }

				$percopy         = @$pcd->discounts[$platformID][0];
				$percopy_percent = @$pcd->discounts[$platformID][1];
				if (!strlen($percopy))
				{
					$percopy         = $pcd->discounts[0][0];
					$percopy_percent = $pcd->discounts[0][1];
				}

				?>all_platformIDs[<?php echo $i;?>]=<?php echo $platformID;?>;<?php
				?>cash_percs[<?php echo $i;?>]=<?php echo sprintf('%0.3f',($cr/2));?>;<?php
				?>credit_percs[<?php echo $i;?>]=<?php echo $cr;?>;<?php
				?>sale_box<?php echo NOBOX;?>discount[<?php echo $i;?>]=<?php echo $snbd;?>;<?php
				?>sale_box<?php echo STOREBOX;?>discount[<?php echo $i;?>]=<?php echo $ssbd;?>;<?php
				?>trade_box<?php echo NOBOX;?>discount[<?php echo $i;?>]=<?php echo $tnbd;?>;<?php
				?>box_percent[<?php echo $i;?>]=<?php echo jstf($bd_percent);?>;<?php
				?>boxdohalf[<?php echo $i;?>]=<?php echo $dh;?>;<?php
				?>cond_new[<?php echo $i;?>]=<?php echo $cn;?>;<?php
				?>cond_fair[<?php echo $i;?>]=<?php echo $cf;?>;<?php
				?>cond_poor[<?php echo $i;?>]=<?php echo $cp;?>;<?php
				?>cond_percent[<?php echo $i;?>]=<?php echo jstf($cd_percent);?>;<?php
				?>min_price[<?php echo $i;?>]=<?php echo $min_price;?>;<?php
				?>percopy[<?php echo $i;?>]=<?php echo $percopy;?>;<?php
				?>percopy_percent[<?php echo $i;?>]=<?php echo jstf($percopy_percent);?>;<?php
				echo "\n";
			}
			?>

			var sto_charge_perc = <?php echo $_SESSION['store_info']['sto_charge_perc'];?>;

			/**
			* table header/footer configs
			* header<type>_vals = array of words. each word is in this format: word[|align[|width]]
			* note: if 'align' or 'width' are blank on header, nothing is shown. all footer cells are right-aligned!
			*/
			var header<?php echo SALE;?>_vals    = ['','','Image','Platform','Title||100%','N/U|center','Qty|center','%off|center','Total'];
			var header<?php echo TRADE;?>_vals   = ['','','Image','Platform','Title||100%','N/U|center','Qty|center','CashVal','CredVal','&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;','Cash','Credit'];
			var header<?php echo RETURNS;?>_vals = ['','','Image','Platform','Title||100%','N/U|center','Qty|center','PurchDate','PurchPrice','Price'];

			var color_label = '<?php echo $pg->color('table-label');?>';
			var color_cell  = '<?php echo $pg->color('table-cell');?>';
			var color_cell2 = '<?php echo $pg->color('table-cell2');?>';
		</script>

		<!-- dynamic content tracking/handling -->
		<script language="javascript" src="/scripts/invoice.js"></script>
		<script language="javascript" src="/scripts/invoice_layers.js"></script>
		<script language="javascript" src="/scripts/invoice_dynamic.js"></script>

		<!-- options popup -->
		<div id="optslyr" style="z-index:1;position:absolute;top:300;left:300;visibility:hidden;padding:4px;border:1px solid #000000;background-color:<?php echo $this->pg->color('table-cell');?>;width:150" onmouseover="mousein()" onmouseout="mouseout()"></div>

		<!-- 'database being updated' popup -->
		<div id="lockedlyr" style="z-index:1;position:absolute;top:300;left:300;visibility:hidden;padding:4px;border:1px solid #000000;background-color:<?php echo $this->pg->color('table-cell');?>;width:375"><b>Please hold - the invoice database is being updated...</b></div>

		<!-- trade pricing calculation popup -->
		<div id="pricinglyr" style="z-index:1;position:absolute;top:300;left:300;visibility:hidden;padding:4px;border:1px solid #000000;background-color:<?php echo $this->pg->color('table-cell2');?>;width:100" onmouseover="inpricing=true" onmouseout="inpricing=false"></div>

		<form method="post" id="remfrm" name="remfrm" action="/admin/pos/invoice_update.php" target="updifrm">
			<input type="hidden" name="act" value="remove">
			<input type="hidden" name="remove_sale" value="">
			<input type="hidden" name="remove_trade" value="">
			<input type="hidden" name="remove_return" value="">
		</form>
		<?php

		$hassalemilestone = NO;
		$hastrademilestone = NO;
		$totlinks = 0;

		while (list($type,$arr) = each($forms))
		{
			$this->t->startTimer("table_type$type");

			$typeword = invType($type);
			$title    = $arr['title'];
			$button   = $arr['button'];
			$items    = $arr['items'];

			$total_qty = 0;
			if ($type == SALE) { $total_sale = 0; }
			elseif ($type == TRADE)
			{
				$total_cash = 0;
				$total_credit = 0;
			}
			elseif ($type == RETURNS) { $total_return = 0; }

			?>
			<table border="0" cellspacing="3" cellpadding="1" width="95%">
				<tr height="25">
					<form method="post" action="/admin/pos/invoice_lookup.php" id="lookup<?php echo $type;?>" target="lookupifrm" onsubmit="return srchverify(this)">
					<input type="hidden" name="act" value="search" />
					<input type="hidden" name="type" value="<?php echo $type;?>" />
					<input type="hidden" name="dolast" value="<?php echo NO;?>" />
					<input type="hidden" name="from_quickadd" id="from_quickadd<?php echo $type;?>" value="<?php echo NO;?>" />
                                        <td>                                                           
                                                <font color="#999999"><b>Add <?php echo $title;?></b> &rarr;&nbsp;&nbsp;</font>
						<span id="search<?php echo $type;?>" style="display:inline">
							<b>UPC/Title:</b> <input type="text" name="criteria[upctitle]" id="upctitle<?php echo $type;?>" size="25" />
							&nbsp;&nbsp;
							<b>Platform:</b> <select name="criteria[platformID]" size="1"><option value=""></option><?php
								$pla = new platforms($this->pg,0);
								$pla->set_item('platforms');
								while (list($a,$arr) = each($pla->values)) { ?><option value="<?php echo $arr[0];?>"><?php echo $arr[1];?></option><?php }
							?></select>
							&nbsp;&nbsp;
							<input type="submit" value="Find Item &gt;" class="btn" />
							&nbsp;&nbsp;
							<a href="javascript:lastSearch(<?php echo $type;?>,$('lookup<?php echo $type;?>'))"><img src="/images/redo.jpg" id="redolast<?php echo $type;?>" width="13" height="15" border="0" alt="Redo Last Search: <?php echo @$_SESSION['invoice_last_search'][$type]['tooltip'];?>" align="top" /></a>
						</span>
						<span id="searching<?php echo $type;?>" style="display:none">
							<font color="red"><b>Please hold - searching the item database...</b></font>
						</span>
					</td>
					</form>
					<td align="right"><b><?php echo $title;?></b></td>
				</tr>
				<tr>
					<td colspan="2">
						<div id="data<?php echo $type;?>"></div>
						<script type="text/javascript">
							// item data for all <?php echo $typeword;?> items<?php
							echo "\n"; // this puts the text at the beginning of the line (makes it easier to read these nasty JS arrays)

							/* does this need to be done?
							$origqty_itemIDs = array();
							$origqtys = array();
							while (list($a,$arr) = each($items))
							{
								if ($arr['ini_type'] == $type) { $origqty_itemIDs[] = $arr['ini_itemID']; }
							}
							reset($items);

							if (count($origqty_itemIDs))
							{
								// pull the original quantities
								$sql = "SELECT qty_itemID,qty_new,qty_used FROM quantity WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID IN (".implode(',',$origqty_itemIDs).")";
								$result = mysql_query($sql,$db);
								$this->error->mysql(__FILE__,__LINE__);

								while ($row = mysql_fetch_assoc($result)) { $origqtys[$row['qty_itemID']] = array($row['qty_new'],$row['qty_used']); }
							}
							*/

							$totitems = 0;
							$setIDXs = array(); // format: $setIDXs['timeadded'] = IDX

							while (list($a,$arr) = each($items))
							{
								$totitems++;

								if ($arr['ini_type'] == $type)
								{
									$idx = ($totitems - 1);
									//$idx = $arr['ini_idx'];
									$setIDXs[$arr['ini_invoiceitemID']] = $idx;

									// data used below
									$invoice_type = $arr['ini_type'];
									$trade_type = $arr['ini_trade_type'];
									$percentoff = $arr['ini_percentoff'];
									$salemilestoneoff = $arr['ini_salemilestoneoff'];
									$trademilestoneup = $arr['ini_trademilestoneup'];
									$price = $arr['ini_price'];
									$qty = $arr['ini_qty'];

									if (strlen($salemilestoneoff) && $salemilestoneoff != '0%') { $hassalemilestone = YES; $salemilestoneamount = $salemilestoneoff; }
									if (strlen($trademilestoneup) && $trademilestoneup != '0%') { $hastrademilestone = YES; $trademilestoneamount = $trademilestoneup; }

									$total_qty += $qty;
									if ($invoice_type == SALE) { $total_sale += $price; }
									elseif ($invoice_type == TRADE) { ${($trade_type==CASH?'total_cash':'total_credit')} += $price; }
									elseif ($invoice_type == RETURNS) { ${($trade_type==CASH?'total_cash':'total_credit')} += $price; $total_return += $price; }

									// build and output the javascript string
									echo $this->getAddItemJS($type,$idx,$arr)."\n";
								}
							}

							// update the IDXs in the database
							$this->setIDXs($type,$setIDXs);
							?>
						</script>
					</td>
				</tr>
			</table>
			<?php

			if ($type == TRADE)
			{
				?>
				<table border="0" cellspacing="0" cellpadding="1" width="95%">
					<tr>
						<td>&nbsp;</td>
						<td align="left">
							<a href="javascript:setAllCCN(<?php echo CASH;?>)"><img src="/images/invoice/ccn-cash.gif" border="0" alt="Set All to Cash"></a>
							<a href="javascript:setAllCCN(<?php echo CREDIT;?>)"><img src="/images/invoice/ccn-credit.gif" border="0" alt="Set All to Credit"></a>
							<img style="vertical-align:middle" src="/images/invoice/anb_all.gif">
						</td>
						<td width="100%" align="right">
							<!--<input type="button" value="Customer Printout &gt;" onclick="customer_printout()" class="btn">-->
						</td>
					</tr>
					<?php
					if ($hastrademilestone == YES)
					{
						?>
						<tr>
							<td colspan="2">
								<font color="red"><b>&nbsp;&nbsp;This customer has reached a sales milestone (<a href="javascript:trademilestoneinfo()" style="color:red">view</a>) and is receiving a <?php echo $trademilestoneamount;?> markup on all trade items</b></font>
								<script type="text/javascript">
									function trademilestoneinfo() { alert("Milestone Sales Requirement: $<?php echo $this->milestoneinfo['saleamount'];?>\nCustomer Sales Total: $<?php echo $this->milestoneinfo['customersales'];?>"); }
								</script>
							</td>
						</tr>
						<?php
					}
					?>
				</table>
				<p />
				<?php
			}
			elseif ($type == SALE)
			{
				?>
				<table border="0" cellspacing="0" cellpadding="0" width="95%">
					<tr>
						<td align="left">
							<?php
							if ($hassalemilestone == YES)
							{
								?>
								<font color="red"><b>&nbsp;&nbsp;This customer has reached a sales milestone (<a href="javascript:salemilestoneinfo()" style="color:red">view</a>) and is receiving <?php echo $salemilestoneamount;?> off all sale items</b></font>
								<script type="text/javascript">
									function salemilestoneinfo() { alert("Milestone Sales Requirement: $<?php echo $this->milestoneinfo['saleamount'];?>\nCustomer Sales Total: $<?php echo $this->milestoneinfo['customersales'];?>"); }
								</script>
								<?php
							}
							else { ?>&nbsp;<?php }
							?>
						</td>
						<td align="right" valign="top" width="100%">
							<a href="javascript:setAllPercent()" id="setallperc"><img src="/images/invoice/setpercent.gif" border="0" alt="Set Percent Discount on All"></a>
							<img style="vertical-align:middle" src="/images/invoice/anb_all.gif">
						</td>
						<td valign="top"><img src="/images/blank.gif" width="5" height="1"></td>
					</tr>
				</table>
				<p />
				<?php
			}
			elseif ($type == RETURNS)
			{
				?>
				<table border="0" cellspacing="0" cellpadding="0" width="95%">
					<tr>
						<td colspan="2">
							&nbsp; &nbsp; &nbsp; <font size="1"><b>Note:</b> The <b>new/used</b> value is at the time of <b>purchase</b>; the other values are at the time of <b>return</b>.</font>
						</td>
					</tr>
					<tr>
						<td>
							&nbsp;<br />
							&nbsp;&nbsp;&nbsp;<input type="button" id="rembutton" value="Remove Selected Items &gt;" onclick="removeItems()" disabled="true" class="btn" />
						</td>
						<td align="right">
							&nbsp;<br />
							<table border="0" cellspacing="0" cellpadding="3">
								<tr>
									<td><span style="border:1px solid #000000;background-color:<?php echo $highlight_color;?>;width:25px;height:25px"></span></td>
									<td>= last-added item</td>
								</tr>
							</table>
						</td>
					</tr>
				</table>
				<p />
				<hr width="95%" size="-1" color="#000000" noshade="noshade" />
				<p />
				<?php
			}

			$this->t->stopTimer("table_type$type");
		}

		$this->t->stopTimer('output_table');

		if (!isset($_SESSION['close_after_complete'])) { $_SESSION['close_after_complete'] = YES; }
		$closechecked = ($_SESSION['close_after_complete']?' checked="checked"':'');

		?>
		<table border="0" cellspacing="0" cellpadding="1" width="95%">
			<tr>
				<td width="33%" style="line-height:20px" valign="top">
					<b>Invoice #:</b> <?php echo str_pad($_SESSION['cust_invoiceID'],4,'0',STR_PAD_LEFT);?><br />
					<b>Employee:</b>
					<form id="emplID" name="emplID">
						<select onfocus="check_select_lock(this.form.foo)" name="employeeID" size="1" style="vertical-align:middle" onchange="set_employeeID(this.value)">
							<option value="0"></option>
							<?php
							$empl = new employees($this->pg);
							$empl->get_employees(YES);
							while (list($employeeID,$arr) = each($empl->employees))
							{
								if ($employeeID == @$_SESSION['cust_invoice_info']['inv_employeeID']) { $s = ' selected="selected"'; } else { $s = ''; }
								?><option value="<?php echo $employeeID;?>"<?php echo $s;?>><?php echo $arr['emp_lname'];?>, <?php echo $arr['emp_fname'];?></option><?php
							}
							?>
						</select>
						<input type="text" name="foo" style="display:none">
					</form>
				</td>
				<td width="34%" valign="top">
					<table border="0" cellspacing="3" cellpadding="0">
						<tr>
							<td class="ten">
								<b>Total Cash:</b><br />
								<b>Total Credit:</b><br />
								<b>Total Sale:</b>
							</td>
							<td>&nbsp;</td>
							<td align="right" class="ten">
								<div id="tbl_catot">$<?php echo number_format($total_cash,2);?></div>
								<div id="tbl_crtot">$<?php echo number_format($total_credit,2);?></div>
								<div id="tbl_satot">$<?php echo number_format($total_sale,2);?></div>
							</td>
						</tr>
					</table>
				</td>
				<td width="33%" align="left" valign="top">
					<input type="checkbox" name="closecomplete" id="closecomplete" onclick="set_closecomplete(this.checked)" class="nb"<?php echo $closechecked;?>> <label for="closecomplete" style="cursor:hand">Close customer after complete/save/cancel</label><br />
					<span id="topmnt" style="display:<?php echo ($total_sale>0||($total_sale==0&&!count($_SESSION['cust_items']))?'block':'none');?>">
						<input type="button" value="Continue to Payment Options &gt;" onclick="if (checklocked()==false && check_employee() && check_remove()) { document.location = '/admin/pos/invoice_payment.php'; }" class="btn">
					</span>
					<span id="tocmplt" style="display:<?php echo ($total_sale==0&&count($_SESSION['cust_items'])?'block':'none');?>">
						<input type="button" value="Complete Invoice &gt;" onclick="if (checklocked()==false && check_employee() && check_remove()) { document.location = '/admin/pos/invtradeinfo.php'; }" class="btn">
					</span>
					<span id="blnk" style="display:<?php echo (!count($_SESSION['cust_items'])?'block':'none');?>">
						&nbsp;
					</span>
					<img src="/images/blank.gif" width="1" height="10" /><br />
					<input type="button" value="Save Invoice &gt;" onclick="if (checklocked()==false && confirm('Saving will close the invoice and leave it incomplete.\n\nIn can be re-opened by clicking \'Re-open Existing Invoice\'\n\nAre you sure you would like to do this?') && check_employee() && check_remove()) { document.location = '/admin/pos/invoice.php?act=close&complete=<?php echo NO;?>'; }" class="btn">
					<br /><img src="/images/blank.gif" width="1" height="10" /><br />
					<input type="button" value="Cancel Invoice &gt;" onclick="if (checklocked()==false && confirm('Are you sure you would like to cancel this invoice?')) { document.location='/admin/pos/invoice.php?act=close&cancel=<?php echo YES;?>'; }" class="btn">
				</td>
			</tr>
			<tr><td colspan="3">&nbsp;<br /><hr width="100%" size="1" color="#CCCCCC" /></td></tr>
			<tr>
				<td colspan="3" align="center">
					<font color="#999999"><b><u>Invoice Status</u>:</b> <span id="invstatus" style="width:110px;text-align:left"></span></font>
				</td>
			</tr>
		</table>

		<form method="post" id="itmfrm" name="itmfrm" action="/admin/pos/invoice_update.php" target="updifrm">
			<input type="hidden" name="act" value="change">
			<input type="hidden" name="type" value="">
			<input type="hidden" name="idx" value="">
			<input type="hidden" name="timeadded" value="">
			<input type="hidden" name="field" value="">
			<input type="hidden" name="to" value="">
			<input type="hidden" name="employeeID" value="">
			<input type="hidden" name="closecustomer" value="">
		</form>

		<?php
		// make the iframe visible or not?
		$development = (in_array(@$_ENV['COMPUTERNAME'],array('SCOTT','SCOTTLAPTOP')) ? YES : NO);
		if ($development)
		{
			// development
			$width = 500;
			$height = 500;
			$border = 1;
		}
		else
		{
			// production
			$width = 1;
			$height = 1;
			$border = 0;
		}
		?>
		<table border="0" cellspacing="0" cellpadding="0">
			<tr>
				<td width="50%" align="center">
					<?php echo ($development ? '<b>invoice_update.php</b><br />' : '');?>
					<iframe name="updifrm" id="updifrm" src="/admin/pos/invoice_update.php" width="<?php echo $width;?>" height="<?php echo $height;?>" frameborder="<?php echo $border;?>" marginwidth="0" marginheight="0">
						Your browser does not support iframes. Please upgrade.
					</iframe>
				</td>
				<td width="50%" align="center">
					<?php echo ($development ? '<b>invoice_lookup.php</b><br />' : '');?>
					<iframe name="lookupifrm" id="lookupifrm" src="/admin/pos/invoice_lookup.php" width="<?php echo $width;?>" height="<?php echo $height;?>" frameborder="<?php echo $border;?>" marginwidth="0" marginheight="0">
						Your browser does not support iframes. Please upgrade.
					</iframe>
				</td>
			</tr>
		</table>
		<?php

		$this->pg->addOnload('drawAllTables()');
		$this->pg->addOnload('updateShownPrices()');

		$this->t->stopTimer('invoice_view_class');

		//$this->t->viewTimes();
	}

	/**
	* Given an invoice_items row and item type, parse out the applicable values and return the javascript string
	* @param integer $type SALE/TRADE/RETURNS
	* @param array $arr invoice_items row
	* @param boolean $parent prepend 'parent.' to the vars? [optional, default NO]
	* @return string
	*/
	function getAddItemJS($type,$idx,$arr,$parent=NO)
	{
		global $ccp,$pcd;

		$typeword = invType($type);

		// parse out the item data and return the generated javascript
		$itemID               = $arr['ini_itemID'];
		$title                = $arr['ini_title'];
		$newused              = $arr['ini_newused'];
		$invoice_type         = $arr['ini_type'];
		$trade_type           = $arr['ini_trade_type'];
		$box                  = $arr['ini_box'];
		$trade_condition      = $arr['ini_condition'];
		$serial_number        = $arr['ini_serial_number'];
		$return_opened        = $arr['ini_opened'];
		$return_purchdate     = $arr['ini_return_purchdate'];
		$return_purchprice    = $arr['ini_return_purchprice'];
		$return_charged       = $arr['ini_return_charged'];
		$return_occasion      = $arr['ini_return_occasion'];
		$return_occasion_date = $arr['ini_return_occasion_date'];
		$platformID           = $arr['ini_platformID'];
		$platform_name        = $arr['ini_platform_name'];
		$platform_abbr        = $arr['ini_platform_abbr'];
		$cash_perc            = sprintf('%0.3f',((@$ccp->percs[$platformID][0]?$ccp->percs[$platformID][0]:$ccp->percs[0][0])/2));
		$credit_perc          = (@$ccp->percs[$platformID][0]?$ccp->percs[$platformID][0]:$ccp->percs[0][0]);
		$price_manual         = $arr['ini_price_manual'];
		$price_new            = $arr['ini_price_new'];
		$price_used           = $arr['ini_price_used'];
		$price                = $arr['ini_price'];
		$qty                  = $arr['ini_qty'];
		$percentoff           = $arr['ini_percentoff'];
		$salemilestoneoff     = $arr['ini_salemilestoneoff'];
		$trademilestoneup     = $arr['ini_trademilestoneup'];
		$timeadded            = $arr['ini_timeadded'];
		$image_box            = $arr['image_box']['THUMB'];
		$image_nobox          = $arr['image_nobox']['THUMB'];

		if ($percentoff > 0)
		{
			if ($percentoff < 0)
			{
				$pre = '+';
				$percentoff *= -1;
			}
			else
			{
				$pre = '-';
			}
			$showpercentoff = "$pre" . rtrim(rtrim($percentoff,'0'),'.') . '%';
		}
		else
		{
			$showpercentoff = '<label for="perc' . $idx . '"><img src="/images/invoice/setpercent.gif" width="35" height="11" border="0"></label>';
		}

		if ($type == TRADE)
		{
			//$price_new = ($price_new - @$pcd->discounts[$arr['ini_platformID']][0]);
			$price_new = prices::applyDiscount(
				$price_new,
				@$pcd->discounts[$arr['ini_platformID']][1],
				@$pcd->discounts[$arr['ini_platformID']][0]
			);
		}

		// output the javascript data for this item
		// NOTE: the extra space on some of the values fixes numeric values like "54"
		$data = array(
			"type{$type}_idxs"           => $idx,
			"{$typeword}_itemIDs"        => $itemID,
			"timeadded$type"             => $timeadded,
			"{$typeword}_invoice_qtys"   => $qty,
			"{$typeword}_platformIDs"    => $platformID,
			"{$typeword}_platform_abbrs" => $platform_abbr . ' ',
			"{$typeword}_platform_names" => $platform_name . ' ',
			"{$typeword}_titles"         => $title . ' ',
			"{$typeword}_prices"         => $price,
			"{$typeword}_box"            => $box,
			"{$typeword}_new_prices"     => $price_new,
			"{$typeword}_used_prices"    => $price_used,
			"{$typeword}_images_box"     => $image_box,
			"{$typeword}_images_nobox"   => $image_nobox
		);

		if ($type == SALE)
		{
			$merge = array(
				'sale_showpercentoff' => $showpercentoff,
				'sale_newused'        => $newused,
				'sale_percent'        => $percentoff,
				'sale_milestone'      => $salemilestoneoff,
				'sale_serial_numbers' => $serial_number,
			);
		}
		elseif ($type == TRADE)
		{
			$merge = array(
				'trade_types'          => $trade_type,
				'trade_condition'      => $trade_condition,
				'trade_manual_price'   => $price_manual,
				'trade_milestone'      => $trademilestoneup,
				'trade_serial_numbers' => $serial_number,
			);
		}
		else
		{
			$merge = array(
				'return_types'          => $trade_type,
				'return_newused'        => $newused,
				'return_opened'         => $return_opened,
				'return_purchdates'     => $return_purchdate,
				'return_purchprices'    => $return_purchprice,
				'return_credit_prices'  => $_SESSION['return_credit_prices'][$timeadded],
				'return_cash_prices'    => $_SESSION['return_cash_prices'][$timeadded],
				'return_charged'        => $return_charged,
				'return_occasion'       => $return_occasion,
				'return_occasion_date'  => $return_occasion_date,
				'return_serial_numbers' => $serial_number,
			);
		}
		$data = array_merge($data,$merge);

		$js = $this->buildItemJSString($idx,$data,$parent);
		$js .= ($parent ? 'parent.' : '');
		$js .= 'qty_' . ($newused==ITEM_NEW ? 'new' : 'used') . "[array_search($itemID," . ($parent ? 'parent.' : '') . "qty_itemIDs)]" . ($type==SALE ? '-=' : '+=') . "$qty;";

		return $js;
	}

	/**
	* Output the javascript to add an item's details to the appropriate arrays
	* @param integer $idx
	* @param array $arr array of values to set (key will have '[$idx]' appended to it!)
	* @param boolean $parent prepend 'parent.' to the vars? [optional, default NO]
	* @return string
	*/
	function buildItemJSString($idx,$arr,$parent=NO)
	{
		$vals = array();
		while (list($k,$v) = each($arr))
		{
			$vals[] = ($parent ? 'parent.' : '') . "$k" . "[$idx]=" . (is_numeric($v) ? $v : "'" . mysql_real_escape_string(trim($v)) . "'") . ';';
		}

		return implode('',$vals);
	}

	/**
	* Output the quantity info javascript
	* @param array $itemIDs
	* @param boolean $append append to end of arrays? [optional, default NO]
	* @param boolean $parent prepend 'parent.' to the vars? [optional, default NO]
	*/
	function getQtyJS($itemIDs,$append=NO,$parent=NO)
	{
		global $db;

		if (count($itemIDs))
		{
			$sql = "SELECT itm_percopy,qty_itemID,qty_new,qty_used FROM items,quantity WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID IN (".implode(',',$itemIDs).") AND qty_itemID=itm_itemID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			ob_start(); // capture output

			$par = ($parent?'parent.':'');

			$i = -1;
			$npc = -1;
			while ($row = mysql_fetch_assoc($result))
			{
				$i++;
				$itemID = $row['qty_itemID'];
				?>var itemIDX = array_search(<?php echo $itemID;?>,<?php echo $par;?>qty_itemIDs);<?php
				?><?php echo $par;?>qty_itemIDs[(itemIDX!=-1?itemIDX:<?php echo (!$append?$i:"{$par}qty_itemIDs.length");?>)]=<?php echo $row['qty_itemID'];?>;<?php
				?><?php echo $par;?>qty_new_orig[(itemIDX!=-1?itemIDX:<?php echo (!$append?$i:"{$par}qty_new_orig.length");?>)]=<?php echo $row['qty_new'];?>;<?php
				?><?php echo $par;?>qty_used_orig[(itemIDX!=-1?itemIDX:<?php echo (!$append?$i:"{$par}qty_used_orig.length");?>)]=<?php echo $row['qty_used'];?>;<?php
				?><?php echo $par;?>qty_total[(itemIDX!=-1?itemIDX:<?php echo (!$append?$i:"{$par}qty_total.length");?>)]=<?php echo ($row['qty_new']+$row['qty_used']);?>;<?php
				?><?php echo $par;?>qty_new[(itemIDX!=-1?itemIDX:<?php echo (!$append?$i:"{$par}qty_new.length");?>)]=<?php echo $row['qty_new'];?>;<?php
				?><?php echo $par;?>qty_used[(itemIDX!=-1?itemIDX:<?php echo (!$append?$i:"{$par}qty_used.length");?>)]=<?php echo $row['qty_used'];?>;<?php
				?><?php echo $par;?>qty_new_counting[(itemIDX!=-1?itemIDX:<?php echo (!$append?$i:"{$par}qty_new_counting.length");?>)]=0;<?php
				?><?php echo $par;?>qty_used_counting[(itemIDX!=-1?itemIDX:<?php echo (!$append?$i:"{$par}qty_used_counting.length");?>)]=0;<?php

				if (!$row['itm_percopy'])
				{
					$npc++;
					?><?php echo $par;?>non_percopy[(itemIDX!=-1?itemIDX:<?php echo (!$append?$npc:"{$par}non_percopy.length");?>)]=<?php echo $row['qty_itemID'];?>;<?php
				}
				echo "\n";
			}

			return ob_get_clean();
		}

		return;
	}

	/**
	* Set the IDXs for the given type/items in the database
	* @param integer $type
	* @param array $idxs format: $idxs[invoiceitemID] = idx
	*/
	function setIDXs($type,$idxs)
	{
		global $db;

		while (list($invoiceitemID,$idx) = each($idxs))
		{
			$sql = "UPDATE invoice_items SET ini_idx=$idx WHERE ini_invoiceID={$_SESSION['cust_invoiceID']} AND ini_invoiceitemID=$invoiceitemID";
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}
	}

	/**
	* Return the next idx for the given type
	* @param integer $type
	* @return integer
	*/
	function getNextIDX($type)
	{
		global $db;

		$sql = "SELECT (MAX(ini_idx)+1) AS nextIDX FROM invoice_items WHERE ini_invoiceID={$_SESSION['cust_invoiceID']} AND ini_type=$type";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		$row = mysql_fetch_assoc($result);
		if (!$row['nextIDX']) { return 0; }
		else { return $row['nextIDX']; }
	}
}

/* END OF FILE */
/* Location: ./classes/invoice_view.inc */
