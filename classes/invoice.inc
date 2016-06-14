<?php
/**
 * Point-of-sale invoice class
 *
 * @author Scott Carpenter <s-carp@comcast.net>
 * @copyright Copyright � 2003-2007 Scott Carpenter <s-carp@comcast.net>
 * @package Fun Unlimited
 * @version 09/25/2007 - Scott Carpenter
 */

class invoice extends invoice_view
{
	/**
	 * @var reference $pg page class reference
	 * @var identifier $error Error reporting class
	 * @var integer $customerID Current customerID
	 * @var integer $invoiceID Current invoiceID
	 * @var array $info Invoice information
	 * @var array $criteria Invoice search criteria
	 * @var array $saved Saved invoices
	 * @var array $milestoneinfo Milestone discount information
	 * @var array $status Status variable used for error reporting (bad email, bad zip code, success, etc)
	 * @var	array	$best_sellers Best selling items
	 * @var array	$area_data	Area data
	 * @var array	$demo_data	Age/sex data
	 */
	var $pg;
	var $error;
	var $customerID;
	var $invoiceID;
	var $info = array();
	var $criteria = array();
	var $saved = array();
	var $milestoneinfo = array();
	var $status = array();
	var $best_sellers = array();
	var $area_data = array();
	var $demo_data = array();

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	*/
	function invoice(&$pg)
	{
		$this->pg =& $pg;
		$this->customerID = @$_SESSION['customerID'];
		$this->invoiceID = @$_SESSION['cust_invoiceID'];
		$this->t =& $GLOBALS['timer_class'];
		$this->t->startTimer('invoice_class');

		if (!strlen($this->customerID)) { $this->customerID = 0; } // this should only happen for online invoices (AKA: shopping carts) for non-logged in customers

		$this->error = new error('Point-of-Sale Invoice');
	}

	/**
	* Pulls the information from $_POST
	*/
	function pull_post()
	{
		$this->t->startTimer('pull_post');

		$this->invoiceID = @$_POST['invoiceID'];
		if (isset($_POST['info']))
		{
			$this->info = $_POST['info'];
			foreach ($this->info as $key => $val) { $this->info[$key] = stripslashes($val); }
		}
		if (isset($_POST['criteria']))
		{
			$this->criteria = $_POST['criteria'];
			foreach ($this->criteria as $key => $val) { $this->criteria[$key] = stripslashes($val); }
		}

		$this->t->stopTimer('pull_post');
	}

	/**
	* Generate a new invoice (create the database entry and set the needed session variables)
	* @param integer $locale Invoice locale (INSTORE/ONLINE)
	*/
	function create_new($locale=INSTORE)
	{
		global $db;

		if ($locale == INSTORE)
		{
			$storeID = $_SESSION['storeID'];
			$_SESSION['close_after_complete'] = YES;
		}
		else { $storeID = 0; }

		if (isset($_SESSION['last_employeeID'])) { $employeeID = $_SESSION['last_employeeID']; }
		else { $employeeID = 0; }

		// insert the new row and obtain the invoice ID
		$set = array(
			'inv_customerID'    => $this->customerID,
			'inv_storeID'       => $storeID,
			'inv_employeeID'    => $employeeID,
			'inv_locale'        => $locale,
			'inv_active'        => YES,
			'inv_completed'     => NO,
			'inv_time'          => time()
		);

		//$sql = "INSERT INTO invoices VALUES (NULL, $this->customerID, $storeID, $employeeID, $locale,0.00,0.00,0.00,0.00,0.00,0.00, '', " . YES . ", ".NO.", '".time()."', '')";
		$sql = 'INSERT INTO invoices SET ' . createInsertVals($set);
		mysql_query($sql, $db);
		$this->error->mysql(__FILE__,__LINE__);

		$invoiceID = mysql_insert_id();
		$this->set_invoiceID($invoiceID);
	}

	/**
	* Delete an invoice
	* @access public
	*/
	function delete()
	{
		global $db;

		$sqls = array(
			"DELETE FROM invoices WHERE inv_invoiceID=$this->invoiceID",
			"DELETE FROM invoice_items WHERE ini_invoiceID=$this->invoiceID"
		);

		foreach ($sqls as $sql)
		{
			mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);
		}

		$_SESSION['cust_invoiceID'] = 0;
	}

	/**
	* Set the active invoice ID and pull the information
	* @param integer $invoiceID
	* @param integer $doupdate update the pricing? [optional, default YES]
	* @param integer $limit_customerID limit to customerID [optional, default 0]
	* @return integer
	*/
	function set_invoiceID($invoiceID=-1, $doupdate=YES, $limit_customerID=0)
	{
		global $db;

		$this->t->startTimer('set_invoiceID');

		if ($invoiceID == -1) { $invoiceID = $_SESSION['cust_invoiceID']; }

		$this->invoiceID = $invoiceID;
		$_SESSION['cust_invoiceID'] = $invoiceID;

		$continue = YES;
		if ($limit_customerID)
		{
			// make sure the invoiceID belongs to the given customer
			$sql = "SELECT * FROM invoices WHERE inv_invoiceID=$invoiceID AND inv_customerID=$limit_customerID";
			$result = mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);

			if (!mysql_num_rows($result)) { $continue = NO; }
		}

		if ($continue)
		{
			// set the invoice as active
			$sql = "UPDATE invoices SET inv_active=" . YES . " WHERE inv_invoiceID=$invoiceID";
			mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);

			// pull in the information for the session variable
			$sql = "SELECT * FROM invoices WHERE inv_invoiceID=$invoiceID";
			$result = mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);
			$row = mysql_fetch_assoc($result);
			$_SESSION['cust_invoice_info'] = $row;
			// remove 'inv_'
			if (is_array($row))
			{
				foreach ($row as $key => $val) { $this->info[substr($key,4)] = $val; }
			}

			// pull in the invoice items
			$_SESSION['cust_items'] = array();
			$sql = "SELECT * FROM invoice_items WHERE ini_invoiceID=$invoiceID ORDER BY ini_timeadded DESC"; // ORDER BY ini_timeadded
			$result = mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				foreach ($row as $k => $v) { $row[$k] = stripslashes($v); }
				$row['image'] = array('FULL'=>'', 'THUMB'=>'');
				$_SESSION['cust_items'][] = $row;
			}
			reset($_SESSION['cust_items']);

			$this->getImages();

			//$this->sort_items();
			if ($doupdate) { $this->update_prices(); }

			return YES;
		}
		else { return NO; }

		$this->t->stopTimer('set_invoiceID');
	}

	/**
	* Get images for all of the items in the session array
	* @access	public
	*/
	function getImages()
	{
		global $db;

		$itemIDs = array();
		foreach ($_SESSION['cust_items'] as $a => $arr)
		{
			$_SESSION['cust_items'][$a]['image'] = array('FULL'=>'', 'THUMB'=>'');
			$itemIDs[] = $arr['ini_itemID'];
		}

		if (!array_sum($itemIDs))
		{
			// no items
			return;
		}

		$itm = new items($this->pg);
		$itm->setItems($itemIDs,YES);
		$items = $itm->getItems();
		foreach ($_SESSION['cust_items'] as $a => $arr)
		{
			$_SESSION['cust_items'][$a]['image_box'] = $items[$arr['ini_itemID']]['image_box'];
			$_SESSION['cust_items'][$a]['image_nobox'] = $items[$arr['ini_itemID']]['image_nobox'];
		}
		reset($_SESSION['cust_items']);
	}

	/**
	* Updates the invoice_items database
	*/
	function update_db_items()
	{
		global $db;

		$this->t->startTimer('update_db_items');

		if ($this->invoiceID)
		{
			foreach ($_SESSION['cust_items'] as $a => $arr)
			{
				$vals = array();
				foreach ($arr as $k => $v)
				{
					if ($k != 'image' && $k != 'image_box' && $k != 'image_nobox' && (1 || $k != 'ini_price' || ($k == 'ini_price' && !$arr['ini_price_manual'])))
					{
						$vals[] = "$k='" . mysql_escape_string($v) . "'";
					}
				}

				if (@$arr['ini_invoiceitemID'])
				{
					// existing item - update
					$sql = 'UPDATE invoice_items SET ' . implode(', ', $vals) . " WHERE ini_invoiceID=$this->invoiceID AND ini_invoiceitemID=" . $arr['ini_invoiceitemID'];
					mysql_query($sql, $db);
					$this->error->mysql(__FILE__,__LINE__);
				}
				else
				{
					// new item - insert
					$sql = 'INSERT INTO invoice_items SET ' . implode(', ', $vals);
					mysql_query($sql, $db);
					$this->error->mysql(__FILE__,__LINE__);

					$invoiceitemID = mysql_insert_id();
					$_SESSION['cust_items'][$a]['ini_invoiceitemID'] = $invoiceitemID;
				}
				$sqls[] = $sql;
			}
		}

		$this->t->stopTimer('update_db_items');
		reset($_SESSION['cust_items']);
	}

	/**
	* Resorts $_SESSION['cust_items'] into trades first (ordered by title) then sales (ordered by title)
	*/
	function sort_items()
	{
		$this->t->startTimer('sort_items');

		if (count($_SESSION['cust_items']))
		{
			// sort the items by type->platform->title
			$sort = new sort();
			$_SESSION['cust_items'] = $sort->doSort(
				$_SESSION['cust_items'],
				'ini_type',
				SORT_ASC,
				'ini_platform_abbr',
				SORT_ASC,
				SORT_STRING,
				'ini_title',
				SORT_ASC,
				SORT_STRING
			);
			reset($_SESSION['cust_items']);

			// break out by type and put the last-added item at the top
			$bytype = array();
			foreach ($_SESSION['cust_items'] as $a => $arr)
			{
				@$bytype[$arr['ini_type']][] = $arr;
			}

			$cust_items = array();
			foreach ($bytype as $type => $items)
			{
				if (count($items) > 1)
				{
					$latestIDX = -1;
					$latest_time = 0;
					foreach ($items as $a => $arr)
					{
						if ($arr['ini_timeadded'] > $latest_time)
						{
							$latestIDX = $a;
							$latest_time = $arr['ini_timeadded'];
						}
					}

					$new_items = array_merge(array($items[$latestIDX]), $items);
					array_splice($new_items,($latestIDX+1),1);
				}
				else { $new_items = $items; }

				$cust_items = array_merge($cust_items, $new_items);
			}

			$_SESSION['cust_items'] = $cust_items;
		}

		$this->t->stopTimer('sort_items');
		reset($_SESSION['cust_items']);
	}

	/**
	* Close the current invoice and add credit to the customer if needed
	* @param integer $complete set the invoice as completed? [optional, default NO]
	* @param integer $cancel cancel the invoice? [optional, default NO]
	*/
	function close($complete=NO, $cancel=NO)
	{
		global $db;

		if ($complete != YES && $complete != NO) { $complete = NO; }
		if ($cancel != YES && $cancel != NO) { $cancel = NO; }

		if ($cancel)
		{
			// delete the invoice and it's items
			$sqls = array(
				"DELETE FROM invoices WHERE inv_invoiceID={$_SESSION['cust_invoiceID']}",
				"DELETE FROM invoice_items WHERE ini_invoiceID={$_SESSION['cust_invoiceID']}"
			);

			foreach ($sqls as $sql)
			{
				mysql_query($sql, $db);
				$this->error->mysql(__FILE__,__LINE__);
			}
		} // if cancel
		else
		{
			// update credit/quantities and close the invoice
			if (isset($_SESSION['cust_close_options']))
			{
				$sess_opts = $_SESSION['cust_close_options'];
				unset($_SESSION['cust_close_options']);
			}
			else
			{
				$sess_opts = array();
			}

			$default_opts = array(
				'set_credit' => '',
				'tax'        => 0,
				'additional' => 0,
				'cash'       => 0,
				'cash_out'   => 0,
				'check'      => 0,
				'credit'     => 0,
				'card_type'  => '',
				'charge'     => 0
			);

			$opts = array();
			foreach ($default_opts as $k => $v)
			{
				if (isset($sess_opts[$k])) { $v = $sess_opts[$k]; }
				$opts[$k] = $v;
			}

			$set_credit = $opts['set_credit'];
			$tax        = (strlen($opts['tax']) ? $opts['tax'] : 0);
			$additional = (strlen($opts['additional']) ? $opts['additional'] : 0);
			$cash       = (strlen($opts['cash']) ? $opts['cash'] : 0);
			$cash_out   = (strlen($opts['cash_out']) ? $opts['cash_out'] : 0);
			$check      = (strlen($opts['check']) ? $opts['check'] : 0);
			$credit     = (strlen($opts['credit']) ? $opts['credit'] : 0);
			$card_type  = $opts['card_type'];
			$charge     = (strlen($opts['charge']) ? $opts['charge'] : 0);

			// set the customer credit if they are completing the invoice
			if (strlen($set_credit) && $complete)
			{
				$sql = "UPDATE customers SET cus_creditamount=$set_credit WHERE cus_customerID={$_SESSION['customerID']}";
				mysql_query($sql, $db);
				//@mail('a@b.com', 'query', "$sql\nerror: " . mysql_error());
				$this->error->mysql(__FILE__,__LINE__);
			}

			if ($complete)
			{
				$completedtime = time();
			}
			else
			{
				$completedtime = 0;
			}

			if (@$_SESSION['cust_invoiceID'])
			{
				$vals = array(
					'inv_tax'           => $tax,
					'inv_additional'    => $additional,
					'inv_cash'          => $cash,
					'inv_cash_out'      => $cash_out,
					'inv_check'         => $check,
					'inv_credit'        => $credit,
					'inv_charge'        => $charge,
					'inv_card_type'     => $card_type,
					'inv_active'        => NO,
					'inv_completed'     => $complete,
					'inv_completedtime' => $completedtime
				);
				$sql = 'UPDATE invoices SET ' . createUpdateVals($vals) . " WHERE inv_invoiceID={$_SESSION['cust_invoiceID']}";
				mysql_query($sql, $db);
				$this->error->mysql(__FILE__,__LINE__);

				// if they are completing the invoice, update the store's quantities
				if ($complete)
				{
					$newadd = array(); // format: $newadd[itemID] = # to add
					$usedadd = array(); // format: $usedadd[itemID] = # to add
					$newsubtract = array(); // format: $newsubtract[itemID] = # to subtract
					$usedsubtract = array(); // format: $usedsubtract[itemID] = # to subtract
					foreach ($_SESSION['cust_items'] as $a => $arr)
					{
						$itemID = $arr['ini_itemID'];

						if (!isset($newadd[$itemID]))
						{
							$newadd[$itemID] = 0;
							$newsubtract[$itemID] = 0;
							$usedadd[$itemID] = 0;
							$usedsubtract[$itemID] = 0;
						}

						$qty = $arr['ini_qty'];

						if ($arr['ini_type'] == SALE)
						{
							if ($arr['ini_newused'] == ITEM_NEW)
							{
								$newsubtract[$itemID] += $qty;
							}
							elseif ($arr['ini_newused'] == ITEM_USED)
							{
								$usedsubtract[$itemID] += $qty;
							}
						} // if sale
						elseif ($arr['ini_type'] == TRADE)
						{
							if ($arr['ini_condition'] == CNEW)
							{
								$newadd[$itemID] += $qty;
							}
							else
							{
								$usedadd[$itemID] += $qty;
							}
						} // elseif trade
						elseif ($arr['ini_type'] == RETURNS)
						{
							if ($arr['ini_opened'] != BROKEN)
							{
								if ($arr['ini_newused'] == ITEM_NEW)
								{
									if ($arr['ini_opened'] == UNOPENED)
									{
										$newadd[$itemID] += $qty;
									}
									else
									{
										// opened
										$usedadd[$itemID] += $qty;
									}
								} // if new
								elseif ($arr['ini_newused'] == ITEM_USED)
								{
									$usedadd[$itemID] += $qty;
								}
							} // if not broken
						} // elseif return
					} // each item
					reset($_SESSION['cust_items']);

					$itemIDs = array_keys($newadd);

					$sqls = array();
					foreach ($itemIDs as $itemID)
					{
						$to_new = ($newadd[$itemID] - $newsubtract[$itemID]);
						$to_used = ($usedadd[$itemID] - $usedsubtract[$itemID]);
						$sqls[] = "UPDATE quantity SET qty_new=(qty_new + $to_new),qty_used=(qty_used + $to_used) WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID=$itemID";
					}

					foreach ($sqls as $sql)
					{
						mysql_query($sql, $db);
						$this->error->mysql(__FILE__,__LINE__);
					}
				} // if complete
			} // if there's an invoice
			else
			{
				?>
				<script type="text/javascript">
					alert('Error closing invoice - maybe you opened and completed another invoice?\nPlease try very hard not to do this!\nYour invoice history will be negatively affected...');
				</script>
				<?php
			}
		} // else close

		$_SESSION['cust_invoiceID'] = 0;

		// update the customer rankings
		// THIS HAPPENS WITH THE HOURLY DATABASE DUMPS!!!
		//$rnk = new rankings($this->pg);
		//$rnk->updateRankings();
	} // end function close

	/**
	* Updates the invoice item prices in the database/session variable according to the current discounts/etc
	* @param integer $invoiceID
	*/
	function update_prices($invoiceID=-1)
	{
		global $db;

		$this->t->startTimer('update_prices');

		if ($invoiceID == -1) { $invoiceID = $_SESSION['cust_invoiceID']; }

		if (@$_SESSION['cust_invoiceID'] && $invoiceID != @$_SESSION['cust_invoiceID']) { $old_invoiceID = $_SESSION['cust_invoiceID']; }
		if (!@$_SESSION['cust_invoiceID']) { $unset = YES; } else { $unset = NO; }
		if ($invoiceID != @$_SESSION['cust_invoiceID']) { $this->set_invoiceID($invoiceID,NO); }

		if ($invoiceID && count(@$_SESSION['cust_items']))
		{
			// first, remove any items with none on hand (sale)/too many on hand (trade)
			$mc = new max_copies($this->pg);

			$this->t->startTimer('update_prices_sec1');
			$mile = new milestone($this->pg);
			list($milestoneID, $range, $saledisctype, $salediscamount, $tradedisctype, $tradediscamount, $custsales) = $mile->get_customer_discount($this->customerID);
			if ($milestoneID != 0)
			{
				$salemilestoneoff = ($saledisctype==DISC_DOLLAR?'$':'').$salediscamount.($saledisctype==DISC_PERCENT?'%':'');
				$trademilestoneup = ($tradedisctype==DISC_DOLLAR?'$':'').$tradediscamount.($tradedisctype==DISC_PERCENT?'%':'');

				$mile->set_milestoneID($milestoneID);
				$this->milestoneinfo = $mile->info;
				$this->milestoneinfo['customersales'] = $custsales;
			}
			$this->t->stopTimer('update_prices_sec1');

			$this->t->startTimer('update_prices_sec2');
			$cust_item_idxs = array(); // format: $cust_item_idxs[itemID] = idx
			$all_itemIDs = array();
			$sale_itemIDs = array();
			$trade_itemIDs = array();
			$return_itemIDs = array();
			$sale_platformIDs = array();
			$trade_platformIDs = array();
			$return_platformIDs = array();
			$original_price_new = array();
			$original_price_used = array();
			foreach ($_SESSION['cust_items'] as $a => $arr)
			{
				if ($arr['ini_type'] == SALE && !in_array($arr['ini_itemID'], $sale_itemIDs)) { $sale_itemIDs[] = $arr['ini_itemID']; $sale_platformIDs[] = $arr['ini_platformID']; }
				if ($arr['ini_type'] == TRADE && !in_array($arr['ini_itemID'], $trade_itemIDs)) { $trade_itemIDs[] = $arr['ini_itemID']; $trade_platformIDs[] = $arr['ini_platformID']; }
				if ($arr['ini_type'] == RETURNS && !in_array($arr['ini_itemID'], $return_itemIDs)) { $return_itemIDs[] = $arr['ini_itemID']; $return_platformIDs[] = $arr['ini_platformID']; }
				if (!in_array($arr['ini_itemID'], $all_itemIDs)) { $all_itemIDs[] = $arr['ini_itemID']; }
				$cust_item_idxs[$arr['ini_itemID']] = $a;

				$original_price_new[$arr['ini_itemID']] = $arr['ini_price_new'];
				$original_price_used[$arr['ini_itemID']] = $arr['ini_price_used'];
			}
			$this->t->stopTimer('update_prices_sec2');
			reset($_SESSION['cust_items']);

			$this->t->startTimer('update_prices_sec3');
			$all_qtys = array(); // format: $all_qtys[itemID] = array(new,used)
			$base_qtys = array(); // format: same as above
			$prices = array(); // format: $prices[itemID][ITEM_NEW/ITEM_USED] = price
			$price_sqls = array();
			$sql = "SELECT qty_itemID,qty_new,qty_used,prc_new,prc_used FROM quantity,prices WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID IN (".implode(', ', $all_itemIDs).") AND qty_itemID=prc_itemID";
			$result = mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				$all_qtys[$row['qty_itemID']] = array($row['qty_new'], $row['qty_used']);
				$base_qtys[$row['qty_itemID']] = array($row['qty_new'], $row['qty_used']);
				$prices[$row['qty_itemID']] = array(ITEM_NEW=>$row['prc_new'],ITEM_USED=>$row['prc_used']);
			}
			$this->t->stopTimer('update_prices_sec3');

			// update the prices in both the database and the session variable
			$this->t->startTimer('update_prices_sec4');
			/*
			!!! GET ORIGINAL NEW/USED AND ONLY CHANGE IN DATABASE IF IT HAS BEEN CHANGED !!!
			*/
			foreach ($prices as $itemID => $arr)
			{
				if ($arr[ITEM_NEW] != $original_price_new[$itemID] || $arr[ITEM_USED] != $original_price_used[$itemID])
				{
					$sql = "UPDATE invoice_items SET ini_price_new=".$arr[ITEM_NEW].",ini_price_used=".$arr[ITEM_USED]." WHERE ini_itemID=$itemID AND ini_invoiceID={$_SESSION['cust_invoiceID']}";
					mysql_query($sql, $db);
					$this->error->mysql(__FILE__,__LINE__);

					$original_price_new[$itemID] = $arr[ITEM_NEW];
					$original_price_used[$itemID] = $arr[ITEM_USED];
				}

				$idx = $cust_item_idxs[$itemID];
				$_SESSION['cust_items'][$idx]['ini_price_new'] = $arr[ITEM_NEW];
				$_SESSION['cust_items'][$idx]['ini_price_used'] = $arr[ITEM_USED];
			}

			// calculate the actual quantities (orig qty +/- invoice qty)
			// don't include returns in this total - they HAVE to be allowed onto the invoice
			foreach ($_SESSION['cust_items'] as $a => $arr)
			{
				if ($arr['ini_type'] == SALE) { $all_qtys[$arr['ini_itemID']][($arr['ini_newused']==ITEM_NEW?0:1)]--; }
				if ($arr['ini_type'] == TRADE) { $all_qtys[$arr['ini_itemID']][1]++; }
			}
			$this->t->stopTimer('update_prices_sec4');

			// remove any items if needed (no longer on hand, max has changed, etc)
			$this->t->startTimer('update_prices_sec5');
			while (list($itemID,list($qnew, $qused)) = each($all_qtys))
			{
				$itemIDarr = array_merge($sale_itemIDs, $trade_itemIDs, $return_itemIDs);
				$platformIDarr = array_merge($sale_platformIDs, $trade_platformIDs, $return_platformIDs);
				$platformID = $platformIDarr[array_search($itemID, $itemIDarr)];
				list($nc, $uc) = @$mc->copies[$platformID];
				if (!strlen($nc)) { $nc = -1; }
				if (!strlen($uc)) { $uc = -1; }

				if ($qnew < 0)
				{
					$where = array('ini_itemID'=>$itemID, 'ini_type'=>SALE, 'ini_newused'=>ITEM_NEW);
					$total = ($qnew*-1);
					$all_qtys[$itemID][0] = 0;
				}
				if ($qused < 0)
				{
					$where = array('ini_itemID'=>$itemID, 'ini_type'=>SALE, 'ini_newused'=>ITEM_USED);
					$total = ($qused*-1);
					$all_qtys[$itemID][1] = 0;
				}
				if ($qnew > $nc && $nc > 0)
				{
					$where = array('ini_itemID'=>$itemID, 'ini_type'=>TRADE, 'ini_newused'=>ITEM_NEW);
					$total = ($qnew-$nc);
					$all_qtys[$itemID][0] = $nc;
				}
				if ($qused > $uc && $uc > 0)
				{
					$where = array('ini_itemID'=>$itemID, 'ini_type'=>TRADE, 'ini_newused'=>ITEM_USED);
					$total = ($qused-$uc);
					$all_qtys[$itemID][0] = $uc;
				}
				if (isset($where)) { $this->remove_items($where, $total); }
			}
			$this->t->stopTimer('update_prices_sec5');

			$this->t->startTimer('update_prices_sec6');
			$invoice_qtys = array(); // format: $invoice_qtys[itemID] = #
			$_SESSION['return_credit_prices'] = array();
			$_SESSION['return_cash_prices'] = array();
			foreach ($_SESSION['cust_items'] as $a => $arr)
			{
				if ($arr['ini_type'] == TRADE)
				{
					if (!isset($invoice_qtys[$arr['ini_itemID']])) { $invoice_qtys[$arr['ini_itemID']] = 0; }
					$invoice_qtys[$arr['ini_itemID']]++;
					$addqty = $invoice_qtys[$arr['ini_itemID']]-1; // the '-1' ignores the first copy
				} else { $addqty = 0; }

				// only include 1 new in total quantities if there are 2+ new
				if ($base_qtys[$arr['ini_itemID']][0] > 0) { $newqty = 1; } else { $newqty = 0; }
				$totcopies = ($newqty+$base_qtys[$arr['ini_itemID']][1]+$addqty);

				$mods = array(
					'customerID'       => $_SESSION['customerID'],
					'itemID'           => $arr['ini_itemID'],
					'platformID'       => $arr['ini_platformID'],
					'type'             => $arr['ini_type'],
					'trade_type'       => $arr['ini_trade_type'],
					'newused'          => $arr['ini_newused'],
					'box_type'         => $arr['ini_box'],
					'trade_condition'  => $arr['ini_condition'],
					'opened'           => $arr['ini_opened'],
					'purchdate'        => $arr['ini_return_purchdate'],
					'purchprice'       => $arr['ini_return_purchprice'],
					'charged'          => $arr['ini_return_charged'],
					'occasion'         => $arr['ini_return_occasion'],
					'occasion_date'    => $arr['ini_return_occasion_date'],
					'percentoff'       => $arr['ini_percentoff'],
					'salemilestoneoff' => @$salemilestoneoff,
					'trademilestoneup' => @$trademilestoneup,
					'totcopies'        => $totcopies,
					'invoiceqty'       => $arr['ini_qty'],
					'prices'           => $prices[$arr['ini_itemID']],
					'timeadded'        => $arr['ini_timeadded']
				);
				//$base = ($arr['ini_type']==SALE?($arr['ini_newused']==ITEM_NEW?$arr['ini_price_new']:$arr['ini_price_used']):$arr['ini_price_used']);
				if ($arr['ini_type'] == RETURNS)
				{
					//if ($arr['ini_newused'] == ITEM_NEW) { $base = $prices[$arr['ini_itemID']][ITEM_NEW]; }
					//elseif ($arr['ini_newused'] == ITEM_USED) { $base = $arr['ini_return_purchprice']; }
					$base = $arr['ini_return_purchprice'];
				}
				else { $base = ($arr['ini_type']==SALE?($arr['ini_newused']==ITEM_NEW?$prices[$arr['ini_itemID']][ITEM_NEW]:$prices[$arr['ini_itemID']][ITEM_USED]):$prices[$arr['ini_itemID']][ITEM_USED]); }

				if (!$arr['ini_price_manual']) { $mod_price = $this->apply_price_mods($base, $mods); } else { $mod_price = $arr['ini_price']; }

				$_SESSION['cust_items'][$a]['ini_salemilestoneoff'] = @$salemilestoneoff;
				$_SESSION['cust_items'][$a]['ini_trademilestoneup'] = @$trademilestoneup;
				$_SESSION['cust_items'][$a]['ini_price'] = $mod_price;
			}
			$this->t->stopTimer('update_prices_sec6');
		}
		if ($invoiceID) { $this->update_db_items(); }

		if (isset($old_invoiceID)) { $this->set_invoiceID($old_invoiceID); }
		if ($unset) { $_SESSION['cust_invoiceID'] = 0; unset($_SESSION['cust_invoiceID']); }

		$this->t->stopTimer('update_prices');
	}

	/**
	* Removes items from $_SESSION['cust_items'] that match certain criteria
	* @param array $where format: array('ini_itemID'=>itemID, 'ini_type'=>[SALE/TRADE], 'ini_newused'=>[ITEM_NEW/ITEM_USED])
	* @param integer $total number of items that fit to remove
	*/
	function remove_items($where, $total)
	{
		$this->t->startTimer('remove_items');
		$_SESSION['invoice_removed_items'] = array();

		if ($total > 0)
		{
			$removed = 0;
			$newitems = array();
			for ($i=(count($_SESSION['cust_items'])-1); $i>-1; $i--)
			{
				$arr = $_SESSION['cust_items'][$i];
				$fits = YES;
				foreach ($where as $k => $v)
				{
					if ($arr[$k] != $v) { $fits = NO; break; }
				}

				if ($fits && $removed < $total)
				{
					// it fits the criteria and all haven't been removed yet
					$removed++;
					$_SESSION['invoice_removed_items'][] = invType($arr['ini_type'],YES).'/'.($arr['ini_newused']==ITEM_NEW?'New':'Used')." - {$arr['ini_platform_name']} - {$arr['ini_title']}";
				}
				else { $newitems[] = $arr; } // it does not fit the criteria or all have been remove already
			}

			$_SESSION['cust_items'] = $newitems;

			$this->update_db_items();
			//$this->sort_items();
		}
		$this->t->stopTimer('remove_items');
	}

	/**
	* Takes a base price and modifiers and returns the modified price (price minus all applicable discounts)
	* !!! ANYTHING CHANGED IN THIS FUNCTION NEEDS TO BE CHANGED IN /SCRIPTS/INVOICE.PHP !!!
	* @param float $base
	* @param array $mods see $this->update_prices $mods array to see valid keys
	*/
	function apply_price_mods($base, $mods)
	{
		global $db;

		if (!isset($mods['platformID'])) { return $base; }
		foreach ($mods as $k => $v) { ${$k} = $v; }

		// see if per-copy discounts applies/get quantity
		$sql = "SELECT itm_percopy,(qty_new+qty_used) AS qty_total FROM items,quantity WHERE itm_itemID=$itemID AND itm_itemID=qty_itemID AND qty_storeID=".$_SESSION['storeID'];
		$result = mysql_query($sql, $db);
		$this->error->mysql(__FILE__,__LINE__);
		$row = mysql_fetch_assoc($result);
		$dopercopy = $row['itm_percopy'];
		$qtytotal = $row['qty_total'];

		// load the cash/credit percentages, box discounts, and minimum prices

		if ($type == TRADE)
		{
			$ccp = new ccpercs($this->pg);
			$ca = @$ccp->percs[$platformID][0];
			$cr = @$ccp->percs[$platformID][0];
			if (!strlen($ca)) { $ca = $ccp->percs[0][0]; }
			if (!strlen($cr)) { $cr = $ccp->percs[0][0]; }
			$ca = sprintf('%0.3f',($ca/2));

			$cash_perc = $ca;
			$credit_perc = $cr;
		}

		if ($type == SALE || $type == TRADE)
		{
			$bod        = new box_discounts($this->pg);
			$snbd       = @$bod->discounts[$platformID][0];
			$ssbd       = @$bod->discounts[$platformID][1];
			$tnbd       = @$bod->discounts[$platformID][2];
			$dh         = @$bod->discounts[$platformID][3];
			$bd_percent = @$bod->discounts[$platformID][4];
			if (!strlen($snbd)) { $snbd = $bod->discounts[0][0]; }
			if (!strlen($ssbd)) { $ssbd = $bod->discounts[0][1]; }
			if (!strlen($tnbd)) { $tnbd = $bod->discounts[0][2]; }
			if (!strlen($dh) || ($dh != YES && $dh != NO)) { $dh = $bod->discounts[0][3]; }
			if (!isset($bod->discounts[$platformID]))
			{
				$bd_percent = $bod->discounts[0][4];
			}

			$sale_nobox    = $snbd;
			$sale_storebox = $ssbd;
			$trade_nobox   = $tnbd;
			$boxdohalf     = $dh;
			$box_percent   = $bd_percent;
		}

		if ($type == TRADE)
		{
			$cod        = new condition_discounts($this->pg);
			$cn         = @$cod->discounts[$platformID][0];
			$cf         = @$cod->discounts[$platformID][1];
			$cp         = @$cod->discounts[$platformID][2];
			$cd_percent = @$cod->discounts[$platformID][3];
			if (!strlen($cn)) { $cn = $cod->discounts[0][0]; }
			if (!strlen($cf)) { $cf = $cod->discounts[0][1]; }
			if (!strlen($cp)) { $cp = $cod->discounts[0][2]; }
			if (!isset($cod->discounts[$platformID]))
			{
				$cd_percent = $cod->discounts[0][3];
			}

			$cond_new     = $cn;
			$cond_fair    = $cf;
			$cond_poor    = $cp;
			$cond_percent = $cd_percent;
		}

		if ($type == TRADE)
		{
			$pcd             = new percopy_discounts($this->pg);
			$percopy         = @$pcd->discounts[$platformID][0];
			$percopy_percent = @$pcd->discounts[$platformID][1];
			if (!strlen($percopy))
			{
				$percopy         = $pcd->discounts[0][0];
				$percopy_percent = $pcd->discounts[0][1];
			}
		}

		$mp = new min_prices($this->pg);
		$min_price = @$mp->prices[$platformID];
		if (!strlen($min_price)) { $min_price = 0.00; }

		$mod_price = $base;
		$check_minprice = YES;
		$continue = YES;

		// TRADE: apply cash/credit/neither
		if ($type == TRADE)
		{
			if ($trade_type == NEITHER) { $mod_price = 0; $check_minprice = NO; }
			else
			{
				if ($trade_type == CREDIT)
				{
					$perc = $credit_perc;
					$mod_price *= ($perc/100);
				}
				else
				{
					// cash is 1/2 of credit - set the trade_type and obtain the credit price
					$mods['trade_type'] = CREDIT;
					$mod_price = ($this->apply_price_mods($base, $mods)/2);
					$mods['trade_type'] = CASH;
					$continue = NO;
				}
			}
		}

		if ($continue)
		{
			// TRADE: apply per-copy discounts
			if ($mod_price > 0 && $type == TRADE && $percopy > 0 && $dopercopy)
			{
				// calculate the amount to subtract
				$sub_amount = 0;
				if ($percopy_percent)
				{
					// determine the amount that is the percentage of the whole price
					// IE: 10% per-copy, $10.00 base = $1.00 off per-copy
					$sub_amount = sprintf('%0.2f',($mod_price * ($percopy / 100)));
				}
				else
				{
					// dollar amount
					$sub_amount = $percopy;
				}
				for ($i=0; $i<$totcopies; $i++)
				{
					$after_mod = prices::applyDiscount($mod_price,NO, $sub_amount);
					if ($after_mod <= ($mod_price / 2))
					{
						$mod_price = sprintf('%0.2f',($mod_price / 2));
					}
					else
					{
						//$mod_price -= $percopy;
						$mod_price = $after_mod;
					}
				}
			}

			// SALE/TRADE: apply box discount (or 1/2 original, if needed)
			if ($mod_price > 0 && ($type == SALE || $type == TRADE))
			{
				$discount = 0;
				if ($box_type != BOX)
				{
					$discount = ($type==SALE ? ($box_type==NOBOX ? $sale_nobox : $sale_storebox) : $trade_nobox);

					$after_mod = prices::applyDiscount($mod_price, $box_percent, $discount);
					if ($boxdohalf && $after_mod < sprintf('%0.2f',($mod_price / 2)))
					{
						$mod_price = sprintf('%0.2f',($mod_price / 2));
					}
					else
					{
						$mod_price = $after_mod;
					}
				}
			}

			// TRADE: apply condition discount
			if ($type == TRADE && $mod_price > 0)
			{
				$cond = $trade_condition;
				$discount = 0;
				if ($cond != GOOD)
				{
					$discount = ($trade_condition==FAIR ? $cond_fair : ($trade_condition==POOR ? $cond_poor : $cond_new));

					$use_discount = ($discount * ($trade_condition==CNEW ? -1 : 1)); // if negative, it is added (price minus a negative discount = added discount)
					$after_mod = prices::applyDiscount($mod_price, $cond_percent, $use_discount);
					if ($after_mod < sprintf('%0.2f',($mod_price / 2)))
					{
						$mod_price = sprintf('%0.2f',($mod_price / 2));

						// if the condition is poor, 1/2 again
						if ($trade_condition == POOR)
						{
							$mod_price = sprintf('%0.2f',($mod_price / 2));
						}
					}
					else
					{
						$mod_price = $after_mod;
					}
				}
			}

			// SALE: subtract a percentage, if applicable
			if ($type == SALE)
			{
				$mod_price = prices::applyDiscount($mod_price,YES, $percentoff);
				//$timesby = 1-($percentoff/100);
				//$mod_price *= $timesby;
			}

			// SALE/TRADE: apply milestone discount/markup, if applicable
			if ($type == SALE && strlen(@$salemilestoneoff))
			{
				if (substr($salemilestoneoff,0,1) == '$') { $disctype = DISC_DOLLAR; $discamount = substr($salemilestoneoff,1); }
				else { $disctype = DISC_PERCENT; $discamount = substr($salemilestoneoff,0,(strlen($salemilestoneoff)-1)); }

				if ($discamount)
				{
					if ($disctype == DISC_PERCENT)
					{
						$timesby = 1-($discamount/100);
						$mod_price *= $timesby;
					}
					elseif ($disctype == DISC_DOLLAR)
					{
						$mod_price -= $discamount;
					}
				}
			}
			elseif ($type == TRADE && strlen(@$trademilestoneup))
			{
				if (substr($trademilestoneup,0,1) == '$') { $disctype = DISC_DOLLAR; $discamount = substr($trademilestoneup,1); }
				else { $disctype = DISC_PERCENT; $discamount = substr($trademilestoneup,0,(strlen($trademilestoneup)-1)); }

				if ($discamount)
				{
					if ($disctype == DISC_PERCENT)
					{
						$timesby = 1+($discamount/100);
						$mod_price *= $timesby;
					}
					elseif ($disctype == DISC_DOLLAR)
					{
						$mod_price += $discamount;
					}
				}
			}

			// RETURNS: get cash/credit value
			if ($type == RETURNS)
			{
				$credit_base = $prices[ITEM_USED];
				$mods['type'] = TRADE;
				$mods['newused'] = ITEM_USED;
				$mods['trade_type'] = CREDIT;
				$credit_price = $this->apply_price_mods($credit_base, $mods);
				$_SESSION['return_credit_prices'][$mods['timeadded']] = $credit_price;

				$mods['trade_type'] = CASH;
				$cash_price = $this->apply_price_mods($credit_base, $mods);
				$_SESSION['return_cash_prices'][$mods['timeadded']] = $cash_price;
			}

			// RETURNS: if new & unopened, give full purchase value back
			// RETURNS: if new & opened, see if past (purchase_price-used_price) days since purchase (IE: $50 new, $35 used, see if within 15 days)
			// if new & opened, price is automatically current used price (minus $1/day below if applicable)
			if ($type == RETURNS && $opened != BROKEN)
			{
				// RETURNS: if they have chosen cash for a used/opened item, give them the normal cash trade value
				if ($trade_type == CASH && $newused == ITEM_USED && $opened == OPENED)
				{
					$mod_price = $cash_price;
				}
				else
				{
					$in_first = NO;
					if ($opened == OPENED)
					{
						if ($occasion == NONE)
						{
							if ($newused == ITEM_NEW)
							{
								$price_diff = (sprintf('%0.0f', $purchprice)-sprintf('%0.0f', $prices[ITEM_USED]));
								$days_diff = $this->pg->daysBetween($purchdate,time(),NO);
								if ($price_diff >= $days_diff) { $in_first = YES; }
							}
							elseif ($newused == ITEM_USED)
							{
								$price_diff = 0;
								$days_diff = $this->pg->daysBetween($purchdate,time(),NO);
							}
						}
						else
						{
							/*
							Special occasion discounts:
							- popup box and ask for date (popped up when occasion clicked)
							- don't subtract for that day/those days
							- if date is Saturday, Monday doesn't count (it becomes their first day)

							For birthdays, the default date in the popup is today
							For Christmas, the default date in the popup is Christmas

							Get difference between today and $occasion_date
							if Birthday, subtract 1
							if Christmas, subtract 7
							*/
							if ($occasion_date != '')
							{
								$days_diff = $this->pg->daysBetween(strtotime($occasion_date),time(),NO);
								if ($occasion == BIRTHDAY) { $days_diff -= 1; }
								elseif ($occasion == CHRISTMAS) { $days_diff -= 7; }
							}
							$price_diff = 0;
						}

						$mod_price = ($in_first?$prices[ITEM_USED]:$purchprice);
					}
					else { $days_diff = 0; $price_diff = 0; }

					if ($days_diff < 0) { $days_diff = 0; }

					// RETURNS: take off $1 per-day, if applicable
					if (!$in_first)
					{
						$mod_price -= ($days_diff-$price_diff);

						// RETURNS: if $mod_price is less than credit value, set to credit value
						if ($mod_price < $credit_price) { $mod_price = $credit_price; }
					}

					// RETURNS: if the item is used/new, opened, and not for an occasion, make sure at least $1 is subtracted
					if ($mod_price == $purchprice && !$occasion && ($newused == ITEM_USED || ($newused == ITEM_NEW && ($opened == OPENED || $opened == BROKEN))))
					{
						$mod_price -= 1;
					}
				}
			}

			// RETURNS: if_applicable, subtract credit card charge percentage (round up to next-highest dollar amount)
			$sub_charged = NO;
			if ($type == RETURNS && $charged)
			{
				if ($opened == BROKEN && $trade_type == CASH) { $sub_charged = YES; }
				else if ($opened == UNOPENED && $newused == ITEM_NEW && $trade_type == CASH) { $sub_charged = YES; }
				else if ($opened == UNOPENED && $newused == ITEM_NEW && $trade_type == CREDIT) { $sub_charged = NO; }
				else if ($opened != BROKEN) { $sub_charged = YES; }
			}
			if ($sub_charged)
			{
				$subperc = $_SESSION['store_info']['sto_charge_perc'];
				if ($subperc > 0)
				{
					$mod_price -= ceil(($subperc/100)*$mod_price);
				}
			}
		}

		// check minimum price
		if ($check_minprice)
		{
			if ($mod_price < $min_price) { $mod_price = $min_price; }
		}

		if ($mod_price < 0) { $mod_price = 0; }

		if ($type == TRADE || ($type == RETURNS && $mod_price != $purchprice))
		{
			// round if greater than $1 or make it an even coin amount if less than $1
			if ($mod_price >= 1)
			{
				$mod_price = round(round($mod_price,2),0); // IE: 1.497 = 1.50 = 2.00
			}
			else
			{
				if ($mod_price >= 0.75)     { $mod_price = 0.75; }
				elseif ($mod_price >= 0.5)  { $mod_price = 0.5; }
				elseif ($mod_price >= 0.25) { $mod_price = 0.25; }
				elseif ($mod_price >= 0.1)  { $mod_price = 0.1; }
				elseif ($mod_price >= 0.05) { $mod_price = 0.05; }
				else { $mod_price = 0.01; }
			}
		}
		else
		{
			$mod_price = sprintf('%0.2f', $mod_price);
		}

		return ($mod_price * $invoiceqty);
	}

	/**
	* Make sure $_SESSION['cust_items'] is populated
	*/
	function check_cust_items()
	{
		if (isset($_SESSION['customerID']) && @$_SESSION['cust_invoiceID'] && (!is_array(@$_SESSION['cust_items']) || !count(@$_SESSION['cust_items'])))
		{
			$this->set_invoiceID($_SESSION['cust_invoiceID']);
		}
	}

	/**
	* Retrieve all saved invoices for the current store and the given customerID (if provided)
	* @param integer $customerID customerID to pull saved invoices for [optional, default 0]
	*/
	function setSaved($customerID=0)
	{
		global $db;

		$custwhere = ($customerID?"inv_customerID=$customerID AND ":'');

		$this->saved = array();

		$sql = "SELECT * FROM invoices,customers WHERE {$custwhere}inv_storeID={$_SESSION['storeID']} AND inv_completed=".NO." AND inv_customerID=cus_customerID ORDER BY inv_time DESC";
		$result = mysql_query($sql, $db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result)) { $this->saved[] = $row; }
	}

	/**
	* Return all saved invoices
	*/
	function getSaved() { return $this->saved; }

	/**
	* Deletes all 15+ day old saved invoices
	* @return array array(#deleted_invoices,#deleted_items)
	*/
	function deleteOldSavedInvoices()
	{
		global $db;

		$twoweeksago = strtotime(date('m/d/Y').' -14 days'); // -14 means that anything at or before 11:59:59pm on the 15th day gets deleted
		$this->setSaved();
		$saved = $this->getSaved();

		$old_invoiceIDs = array();
		foreach ($saved as $a => $arr)
		{
			if ($arr['inv_time'] < $twoweeksago) { $old_invoiceIDs[] = $arr['inv_invoiceID']; }
		}

		if (count($old_invoiceIDs))
		{
			// delete the invoices and their respective entries in invoice_items
			$sql = "DELETE FROM invoices WHERE inv_invoiceID IN (".implode(', ', $old_invoiceIDs).")";
			mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);
			$del_invoices = mysql_affected_rows();

			$sql = "DELETE FROM invoice_items WHERE ini_invoiceID IN (".implode(', ', $old_invoiceIDs).")";
			mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);
			$del_items = mysql_affected_rows();

			return array($del_invoices, $del_items);
		}
		else { return array(0,0); }
	}

	/**
	* Return the date range of all invoices
	* @return	string
	*/
	function getInvoiceRange()
	{
		global $db;

		$sql = "SELECT MIN(inv_completedtime) AS min,MAX(inv_completedtime) AS max FROM invoices WHERE inv_completedtime>0 AND inv_storeID=" . $_SESSION['storeID'];
		$result = mysql_query($sql, $db);
		$this->error->mysql(__FILE__,__LINE__);
		$row = mysql_fetch_assoc($result);

		return array(date('m/d/Y', $row['min']),date('m/d/Y', $row['max']));
	}

	/**
	* Get the 50 best sellers for the given platformID in the given date range
	* @param	integer	$platformID
	* @param	integer	$from
	* @param	integer	$to
	* @param	integer	$num number of items to pull
	* @param	string	$order [optional, default 'qty']
	* @param	string	$stock [optional, defeault 'either']
	* @access	public
	*/
	function setBestSellers($platformID, $from, $to, $num=50, $order='qty', $stock='either')
	{
		global $db;

		set_time_limit(120);
		$orderbys = array(
			'qty'   => 'tot_qty DESC,tot_price DESC',
			'price' => 'tot_price DESC,tot_qty DESC'
		);

		$order = (in_array($order,array('qty', 'price')) ? $order : 'qty');
		$orderby = $orderbys[$order];

		$this->best_sellers = array();

		$tables = array(
			'invoices',
			'invoice_items',
			'quantity'
		);
		$where = array(
			'inv_storeID=1',
			"inv_completedtime BETWEEN $from AND $to",
			'inv_invoiceID=ini_invoiceID',
			'ini_itemID=qty_itemID',
			'qty_storeID=1'
		);

		if ($platformID)
		{
			$where[] = "ini_platformID=$platformID";
		}
		if ($stock != 'either')
		{
			$where[] = '(qty_new+qty_used)' . ($stock=='in' ? '>' : '<=') . '0';
		}

		$place = 0;
		$limit = ($num!='all' ? " LIMIT $num" : '');
		$sql = '
			SELECT
				*,
				SUM(ini_qty) AS tot_qty,
				SUM(ini_price) AS tot_price
			FROM
				' . implode(', ', $tables) . '
			WHERE
				' . implode(' AND ', $where) . '
			GROUP BY
				ini_itemID
			ORDER BY
				' . $orderby
			. $limit
		;
		$result = mysql_query($sql, $db);
		while ($row = mysql_fetch_assoc($result))
		{
			$place++;
			$row['PLACE'] = $place;
			$this->best_sellers[$row['ini_itemID']] = $row;
		}

		$itemIDs = array_keys($this->best_sellers);
		$itm = new items($this->pg);
		$itm->setItems($itemIDs);
		$items = $itm->getItems();
		foreach ($items as $itemID => $arr)
		{
			$this->best_sellers[$itemID] = array_merge($this->best_sellers[$itemID], $arr);
		}
	}

	/**
	* Return the best sellers
	* @return	array
	* @access	public
	*/
	function getBestSellers()
	{
		return $this->best_sellers;
	}

	/**
	* Get the area sales/trades data for the given platformID in the given date range
	* @param	integer	$platformID
	* @param	integer	$from
	* @param	integer	$to
	* @param 	string	$type
	* @param 	string	$sort	[optional, default 'area_zip']
	* @access	public
	*/
	function setAreaData($platformID, $from, $to, $type, $sort='area_zip')
	{
		global $db;

		set_time_limit(120);
		$this->area_data = array();

		$tables = array(
			'invoices',
			'invoice_items',
			'customers'
		);
		$where = array(
			'inv_storeID=1',
			"inv_completedtime BETWEEN $from AND $to",
			'inv_invoiceID=ini_invoiceID',
			"ini_type=$type",
			'inv_customerID=cus_customerID'
		);

		if ($platformID)
		{
			$where[] = "ini_platformID=$platformID";
		}

		$selects = array(
			array(
				'select'    => 'SUBSTR(cus_phone, 1, 3) AS area_code, COUNT(DISTINCT(inv_invoiceID)) AS count',
				'var'       => 'area_code',
				'add_where' => 'LENGTH(cus_phone)=10'
			),
			array(
				'select'    => 'SUBSTR(cus_phone, 2, 3) AS area_code, COUNT(DISTINCT(inv_invoiceID)) AS count',
				'var'       => 'area_code',
				'add_where' => 'LENGTH(cus_phone)=11'
			),
			array(
				'select'    => 'cus_zip AS zip_code, COUNT(DISTINCT(inv_invoiceID)) AS count',
				'var'       => 'zip_code',
				'add_where' => 'LENGTH(cus_zip)=5'
			),
			array(
				'select'    => 'SUBSTR(cus_zip, 1, 5) AS zip_code, COUNT(DISTINCT(inv_invoiceID)) AS count',
				'var'       => 'zip_code',
				'add_where' => 'LENGTH(cus_zip)=9'
			),
		);

		foreach ($selects as $a => $arr)
		{
			extract($arr);
			$this_where = $where;
			$this_where[] = $add_where;

			if (!isset($this->area_data[$var]))
			{
				$this->area_data[$var] = array();
				$this->area_data[$var . '_total'] = 0;
			}

			$sql = "SELECT $select FROM " . implode(', ', $tables) . ' WHERE ' . implode(' AND ', $this_where) . " GROUP BY $var ORDER BY $var";
			$result = mysql_query($sql, $db);
			while ($row = mysql_fetch_assoc($result))
			{
				@$this->area_data[$var][$row[$var]] += $row['count'];
				$this->area_data[$var . '_total'] += $row['count'];
			}

			// $sort=='area_zip' is what happens by default in the query
			if ($sort == 'high_low')
			{
				arsort($this->area_data[$var]);
			}
			elseif ($sort == 'low_high')
			{
				asort($this->area_data[$var]);
			}
		}
	} // end function setAreaData

	/**
	* Return the area sales/trades data
	* @return	array
	* @access	public
	*/
	function getAreaData()
	{
		return $this->area_data;
	}

	/**
	* Get the age/sex sales/trades data for the given platformID in the given date range
	* @param	integer	$platformID
	* @param	integer	$from
	* @param	integer	$to
	* @param 	string	$type
	* @access	public
	*/
	function setDemoData($platformID, $from, $to, $type)
	{
		global $db;

		set_time_limit(120);
		$this->demo_data = array();

		$tables = array(
			'invoices',
			'invoice_items',
			'customers'
		);
		$where = array(
			'inv_storeID=1',
			"inv_completedtime BETWEEN $from AND $to",
			'inv_invoiceID=ini_invoiceID',
			"ini_type=$type",
			'inv_customerID=cus_customerID'
		);

		if ($platformID)
		{
			$where[] = "ini_platformID=$platformID";
		}

		$totals = "COUNT(DISTINCT(inv_invoiceID)) AS tot_invoice, SUM(ini_qty) AS tot_qty, SUM(ini_price) AS tot_price";

		$date_formats = array(
			1 => '%/%/%',
			2 => '%-%-%'
		);
		$selects = array();
		foreach ($date_formats as $format => $like)
		{
			$selects[] = array(
				'select'    => "cus_dob AS dob, cus_dob, $format AS format, $totals",
				'var'       => 'dob',
				'add_where' => "cus_dob LIKE '$like'",
				'having'    => '',
				'age'       => true
			);
		}
		$selects[] = array(
			'select'    => "cus_gender AS gender, $totals",
			'var'       => 'gender',
			'add_where' => 'cus_gender!=0',
			'having'    => '',
			'age'       => false
		);

		foreach ($selects as $a => $arr)
		{
			extract($arr);
			$this_where = $where;
			$this_where[] = $add_where;

			$having = (strlen($having) ? " HAVING $having" : '');

			if (!isset($this->demo_data[$var]))
			{
				$this->demo_data[$var] = array();
			}

			$sql = "SELECT $select FROM " . implode(', ', $tables) . ' WHERE ' . implode(' AND ', $this_where) . " GROUP BY $var$having ORDER BY $var";
			$result = mysql_query($sql, $db);
			$this->error->mysql(__FILE__, __LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				$key = $row[$var];

				if ($age)
				{
					$dob = $row['cus_dob'];
					$format = $row['format'];
					$key = $this->_getDemoAge($dob, $format);
					if ($key <= 0)
					{
						// not yet born?
						continue;
					}
					// uncomment to view what DOBs lead to the ages specified
					/*
					if ($key >= 90)
					{
						echo "$dob = $key<br />";
					}
					*/
				} // if age select

				@$this->demo_data[$var][$key]['tot_invoice'] += $row['tot_invoice'];
				@$this->demo_data[$var][$key]['tot_qty'] += $row['tot_qty'];
				@$this->demo_data[$var][$key]['tot_price'] += $row['tot_price'];

				@$this->demo_data["total_$var"]['tot_invoice'] += $row['tot_invoice'];
				@$this->demo_data["total_$var"]['tot_qty'] += $row['tot_qty'];
				@$this->demo_data["total_$var"]['tot_price'] += $row['tot_price'];
			}
		}

		foreach ($this->demo_data as $key => $arr)
		{
			ksort($this->demo_data[$key]);
		}
	} // end function setDemoData

	/**
	 * Given a timestamp, return an age
	 * @param	string	$dob
	 * @param	integer	$format	1 (%/%/%) or 2 (%-%-%)
	 * @return	integer
	 * @access	private
	 */
	function _getDemoAge($dob, $format)
	{
		$sep = ($format==1 ? '/' : '-');
		$boom = explode($sep, $dob);
		if (count($boom) != 3 || !$boom[0] || !$boom[1] || !$boom[2])
		{
			// invalid format
			return 0;
		}

		$month = $boom[0];
		$day   = $boom[1];
		$year  = (int)$boom[2];
		$orig_year = $year;

		if ($year <= date('y'))
		{
			$year += 2000;
		}
		elseif ($year < 100)
		{
			$year += 1900;
		}

		$now = $this->_getTime(date('m'), date('d'), date('Y'));
		$time = $this->_getTime($month, $day, $year);

		$days = (($now - $time) / (60 * 60 * 24));
		$years = ($days / 365);

		return floor($years);
	} // end function _getDemoAge

	/**
	 * Return a subtractable number of seconds (number since 1/1/1900, rather than 1/1/1969)
	 * @param	integer	$month
	 * @param	integer	$day
	 * @param	integer	$year
	 * @return	integer
	 */
	function _getTime($month, $day, $year)
	{
		$orig_year = $year;
		$year -= 1900;

		$time  = (60 * 60 * 24 * 365 * $year);
		$time += (60 * 60 * 24 * $day);

		$dim = array(31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31);
		for ($i=0; $i<$month && $i<12; $i++)
		{
			$time += (60 * 60 * 24 * $dim[$i]);
		}

		return $time;
	}

	/**
	* Return the age/sex sales/trades data
	* @return	array
	* @access	public
	*/
	function getDemoData()
	{
		return $this->demo_data;
	}

	/**
	* Return the javascript commands that are needed to update the time added
	* @return	array
	* @access	public
	*/
	function getNewTimeAdded()
	{
		$new_timeadded = array(
			SALE    => array(),
			TRADE   => array(),
			RETURNS => array()
		);
		foreach ($_SESSION['cust_items'] as $a => $arr)
		{
			$new_timeadded[$arr['ini_type']][$arr['ini_idx']] = $arr['ini_timeadded'];
		}

		$commands = array();
		foreach ($new_timeadded as $type => $arr)
		{
			ksort($arr);
			$commands[] = "parent.timeadded$type = [" . implode(', ', $arr) . ']';
		}

		return $commands;
	} // end function getNewTimeAdded

	/**
	 * Pull in the summary report data
	 * @param array $crit
	 */
	function setSummaryData($crit)
	{
		global $db;

		$blank_data = array(
			'platform'            => '',
			'item'                => '',
			'employeeID'          => 0,
			'sales'               => 0,
			'count_sales'         => 0,
			'trades_cash'         => 0,
			'count_trades_cash'   => 0,
			'trades_credit'       => 0,
			'count_trades_credit' => 0,
			'returns'             => 0,
			'count_returns'       => 0,
			'net_sales'           => 0
		);
		$this->summary_data = array(
			'_all' => $blank_data
		);

		$platformID     = getA($crit, 'platformID');
		$employeeID     = getA($crit, 'employeeID');
		$from           = strtotime(getA($crit, 'fromdate'));
		$to             = strtotime(getA($crit, 'todate') . ' 11:59:59pm');
		$show           = getA($crit, 'show');
		$groupby        = getA($crit, 'groupby');

		$this->summary_range = date('m/d/Y', $from) . ' to ' . date('m/d/Y', $to);

		$empl = new employees($this->pg);
		$empl->get_employees(BOTH);
		$employeeIDs = array();
		foreach ($empl->employees as $emplID => $arr)
		{
			if
			(
				!$employeeID
				||
				$employeeID == $emplID
				||
				($employeeID == 'active' && $arr['emp_active'])
				||
				($employeeID == 'inactive' && !$arr['emp_active'])
			)
			{
				$employeeIDs[] = $emplID;
			}
		}

		$groupbys = array(
			'item'     => 'ini_itemID',
			'platform' => 'ini_platform_name',
			'employee' => 'inv_employeeID'
		);
		$sql_groupby = $groupbys[$groupby];

		/*
		TO DO:
		- get rid of 'employees' table - we need to handle inv_employeeID=0
		  !! NOT YET - ONLY 0.73% of invoices have no employee, and as of 11/4/2007, the last one was 10 months ago !!
		- validate all output data
		- mark off the reports that this covers
		*/

		$sortbys = array(
			'item'     => 'ini_platform_name, ini_title',
			'platform' => 'ini_platform_name',
			'employee' => 'emp_lname, emp_fname'
		);
		$sql_sortby = $sortbys[$groupby];

		$select = array(
			'*',
			'ini_price AS sales',
			//'(ini_price - inv_tax - IF(inv_cash>0, 0, inv_credit) - inv_cash_out) AS sales',
			'ini_price AS trades_cash',
			'ini_price AS trades_credit',
			'ini_price AS returns'
		);

		$tables = array(
			'invoices',
			'invoice_items',
			'employees'
		);

		$where = array(
			'inv_storeID=' . $_SESSION['storeID'],
			"inv_completedtime BETWEEN $from AND $to",
			'inv_employeeID IN ' . getIn($employeeIDs),
			'inv_employeeID=emp_employeeID',
			'inv_invoiceID=ini_invoiceID'
		);

		if ($platformID)
		{
			$where[] = "ini_platformID=$platformID";
		}

		set_time_limit(120);
		$sql = "
			SELECT
				" . implode(', ', $select) . "
			FROM
				" . implode(', ', $tables) . "
			WHERE
				" . implode("\nAND ", $where) . "
		";
		$result = mysql_query($sql, $db);
		while ($row = mysql_fetch_assoc($result))
		{
			$key = $row[$sql_groupby];
			$data = getA($this->summary_data, $key, $blank_data);
			$data['platform'] = $row['ini_platform_name'];
			$data['item'] = $row['ini_title'];
			$data['employeeID'] = $row['inv_employeeID'];

			$ini_type = $row['ini_type'];
			$ini_trade_type = $row['ini_trade_type'];

			if ($ini_type == SALE)
			{
				$data['sales'] += $row['sales'];
				$data['net_sales'] += $row['sales'];
				$data['count_sales'] += $row['ini_qty'];
			}
			elseif ($ini_type == TRADE && $ini_trade_type == CASH)
			{
				$data['trades_cash'] += $row['trades_cash'];
				$data['net_sales'] -= $row['trades_cash'];
				$data['count_trades_cash'] += $row['ini_qty'];
			}
			elseif ($ini_type == TRADE && $ini_trade_type == CREDIT)
			{
				$data['trades_credit'] += $row['trades_credit'];
				$data['net_sales'] -= $row['trades_credit'];
				$data['count_trades_credit'] += $row['ini_qty'];
			}
			elseif ($ini_type == RETURNS)
			{
				$data['returns'] += $row['returns'];
				$data['net_sales'] -= $row['returns'];
				$data['count_returns'] += $row['ini_qty'];
			}

			$this->summary_data[$key] = $data;
		}

		foreach ($this->summary_data as $key => $totals)
		{
			if (!$key == '_all')
			{
				continue;
			}
			foreach ($totals as $k => $v)
			{
				if (!in_array($k, array('platform', 'item', 'employeeID')))
				{
					$this->summary_data['_all'][$k] += $v;
				}
			}
		}

		if ($show == 'average')
		{
			// calculate the average for all totals
			foreach ($this->summary_data as $key => $totals)
			{
				$totals['sales']         /= ifNotZero($totals['count_sales'], 1);
				$totals['net_sales']     /= ifNotZero($totals['count_sales'], 1);
				$totals['trades_cash']   /= ifNotZero($totals['count_trades_cash'], 1);
				$totals['trades_credit'] /= ifNotZero($totals['count_trades_credit'], 1);
				$totals['returns']       /= ifNotZero($totals['count_returns'], 1);
				$this->summary_data[$key] = $totals;
			}
		}
	} // end function setSummaryData

	function getSummaryData()
	{
		return $this->summary_data;
	}

	function getSummaryRange()
	{
		return $this->summary_range;
	}
} // end class invoice

?>