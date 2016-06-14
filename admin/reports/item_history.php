<?php
include('../../include/include.inc');

$cl = new check_login();

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$itemID = (isset($_GET['itemID'])?$_GET['itemID']:@$_POST['itemID']);
$print = (isset($_GET['print'])?$_GET['print']:@$_POST['print']);

$pg = new admin_page();
$pg->setFull((!$print ? YES : NO));
$pg->setTitle('Item History');
$pg->head('Item History');

$its = new item_search($pg);
$its->action = "{$_SESSION['root_admin']}reports/item_history.php";
$error = new error('Item History');

if ($act == "" && !$itemID)
{
	// show search form
	?>
	To view an item's history, enter as much criteria as possible.
	<p />
	<?php

	$its->form(YES,array(),NO);
}
elseif ($act == "search")
{
	$its->pull_post();
	$_SESSION['search_criteria'] = $its->criteria;

	$results = $its->search();

	if (!count($results) && count($its->criteria) == 2 && strlen(@$its->criteria['upc'])) { $onlyupc = YES; }
	else { $onlyupc = NO; }

	if (count($its->results) == 1)
	{
		// found 1 item
		$itemID = $its->results[0]['itm_itemID'];
	}
	elseif (count($its->results) > 1) { $multiple = YES; }
	else
	{
		// display the add page
		?>
		Your search returned no results. Please try again.
		<p />
		<?php
		$its->form(YES,array(),NO);
	}
}

if (@$multiple)
{
	$only = '';
	$count = count($its->results);
	if ($count >= $its->max_results)
	{
		$only = " Only the first $its->max_results are shown.";

		$its->results = array_slice($its->results,0,$its->max_results);
		$_SESSION['search_results'] = $its->results;
	}

	?>
	<?php echo $count;?> item<?php echo ($count==1?'':'s');?> matched your criteria.<?php echo $only;?>
	<p />
	<input type="button" value="&lt; Search Again" onclick="document.location='/admin/reports/item_history.php'" class="btn" />
	<?php

	$its->showSmallResults(NO,array(),YES,NO);
}

if (strlen($itemID))
{
	$itm = new items($pg);
	$itm->set_itemID($itemID);
	$info = $itm->info;

	?>
	<font size="2"><b><?php echo $info['title'];?> - <?php echo $info['name'];?></b></font>
	<p />
	<input type="button" value="&lt; Select Different Item" onclick="document.location='/admin/reports/item_history.php'" class="btn" />
	<?php
	if (!$print) { ?><input type="button" value="Printable Format &gt;" onclick="document.location='/admin/reports/item_history.php?itemID=<?php echo $itemID;?>&print=<?php echo YES;?>'" class="btn" /><?php }
	else
	{
		?>
		<input type="button" value="Print List" onclick="window.print()" class="btn" />
		<input type="button" value="&lt; Return to Normal Format" onclick="document.location='/admin/reports/item_history.php?itemID=<?php echo $itemID;?>'" class="btn" />
		<?php
	}
	?>
	<p />
	<?php

	// get the item's history
	$history = array();

	$sql = "SELECT * FROM invoices,invoice_items,customers WHERE inv_storeID={$_SESSION['storeID']} AND inv_invoiceID=ini_invoiceID AND ini_itemID=$itemID AND inv_customerID=cus_customerID ORDER BY ini_timeadded DESC";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);
	while ($row = mysql_fetch_assoc($result))
	{
		$time = $row['ini_timeadded'];
		$row['type'] = $row['ini_type'];
		@$history[$time][] = $row;
	}

	$sql = "SELECT * FROM received_orders,received_order_items WHERE ror_storeID={$_SESSION['storeID']} AND ror_completed=" . YES . " AND ror_orderID=roi_orderID AND roi_itemID=$itemID ORDER BY ror_time DESC";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);
	while ($row = mysql_fetch_assoc($result))
	{
		$time = $row['ror_time'];
		$row['type'] = RECEIVED;
		@$history[$time][] = $row;
	}

	if (!count($history))
	{
		echo 'There were no invoices/received items found within this store containing this item.';
	}
	else
	{
		// output the history
		krsort($history);
		?>
		<script type="text/javascript">
			function showAllCustomers()
			{
				for (var i=0; i<item_types.length; i++)
				{
					if (item_types[i] != <?php echo RECEIVED;?>)
					{
						$('cusinfo' + i).style.display = 'block';
					}
				}
			}

			function hideAllCustomers()
			{
				for (var i=0; i<item_types.length; i++)
				{
					if (item_types[i] != <?php echo RECEIVED;?>)
					{
						$('cusinfo' + i).style.display = 'none';
					}
				}
			}

			function showCustomerInfo(idx)
			{
				var obj1 = $('cusinfo'+idx);
				obj1.style.display = (obj1.style.display=='none' ? 'block' : 'none');
			}

			function showType(type)
			{
				hideAllCustomers();

				var bgs = ['<?php echo $pg->color('table-cell');?>', '<?php echo $pg->color('table-cell2');?>'];
				var count = 0;
				for (var i=0; i<item_types.length; i++)
				{
					var obj = $('item'+i);
					var itmtype = item_types[i];
					if (type == '' || itmtype == type)
					{
						count++;
						if (!<?php echo jstf($print);?>)
						{
							obj.bgColor = bgs[(count%2)];
						}
						obj.style.display = 'block';
					}
					else
					{
						obj.style.display = 'none';
					}
				}

				$('itemnone').style.display = (count ? 'none' : 'block');
			}
		</script>

		<p>
			Click on a customer's name to show/hide their details.
		</p>

		<p>
			<b>Show Type:</b>
			<select name="type" size="1" onchange="showType(this.value)">
				<option value="">All Types</option>
				<option value="<?php echo SALE;?>">Sales</option>
				<option value="<?php echo TRADE;?>">Trades</option>
				<option value="<?php echo RETURNS;?>">Returns</option>
				<option value="<?php echo RECEIVED;?>">Received Items</option>
			</select>
		</p>

		<p>
			<font size="1">
				<a href="javascript:void(0)" onclick="showAllCustomers()">Show All Customer Data</a>
				/
				<a href="javascript:void(0)" onclick="hideAllCustomers()">Hide All Customer Data</a>
			</font>
		</p>

		<?php
		$pg->outlineTableHead();
		?>
		<tr bgcolor="<?php echo ($print?'#FFFFFF':$pg->color('table-head'));?>">
			<td align="center" width="40"><b>Type</b></td>
			<td align="center"><b>Date/Time</b></td>
			<td align="center" width="150"><b>Customer/Distributor</b></td>
			<td align="center"><b>Qty</b></td>
			<td align="center"><b>Price</b></td>
			<td align="center" width="225"><b>Misc. Details</b></td>
		</tr>
		<?php
		$idx = -1;
		$types = array();
		while (list($time, $history_items) = each($history))
		{
			while (list($a, $arr) = each($history_items))
			{
				$idx++;
				$bg = ($print ? '#FFFFFF' : (($idx % 2) ? $pg->color('table-cell') : $pg->color('table-cell2')));
				$type = $arr['type'];
				$types[] = $type;

				?>
				<tr bgcolor="<?php echo $bg;?>" id="item<?php echo $idx;?>" style="display:block">
					<td><b><?php echo getTypeName($type);?></b></td>
					<td><?php echo date('m/d/Y h:ia', $time);?></td>
					<td>
						<?php
						if ($type == RECEIVED)
						{
							echo $arr['ror_distributor'];
						}
						else
						{
							?>
							<a href="javascript:void(0)" onclick="showCustomerInfo(<?php echo $idx;?>)"><?php echo "{$arr['cus_fname']} {$arr['cus_lname']}";?></a>
							<span id="cusinfo<?php echo $idx;?>" style="display:none">
								<?php echo $arr['cus_address'];?><br />
								<?php echo "{$arr['cus_city']}, {$arr['cus_state']} {$arr['cus_zip']}";?><br />
								Phone: <?php echo $pg->format('phone',$arr['cus_phone']);?><br />
								Cell Phone: <?php echo $pg->format('phone',$arr['cus_cellphone']);?>
								<p />
								Ethnicity: <?php echo $arr['cus_ethnicity'];?><br />
								Height: <?php echo $arr['cus_height'];?><br />
								Weight: <?php echo $arr['cus_weight'];?><br />
								DOB: <?php echo $arr['cus_dob'];?><br />
								Gender: <?php echo $arr['cus_gender'];?><br />
								ID Number: <?php echo $arr['cus_idnumber'];?><br />
								ID Expiration: <?php echo $arr['cus_idexpiration'];?>
							</span>
							<?php
						}
						?>
					</td>
					<td align="right">
						<?php
						if ($type == RECEIVED)
						{
							$newqty = $arr['roi_qtynew'];
							$usedqty = $arr['roi_qtyused'];
							if ($newqty)
							{
								echo "New: $newqty";
							}
							if ($usedqty)
							{
								echo ($newqty ? '<br />' : '') . "Used: $usedqty";
							}
						}
						else
						{
							echo $arr['ini_qty'];
						}
						?>
					</td>
					<td align="right">
						<?php
						if ($type == RECEIVED)
						{
							$new = $arr['roi_price_new'];
							$used = $arr['roi_price_used'];
							if ($newqty)
							{
								echo sprintf('%0.2f', $new);
							}
							if ($usedqty)
							{
								echo ($new ? '<br />' : '') . sprintf('%0.2f', $used);
							}
						}
						else
						{
							echo sprintf('%0.2f',$arr['ini_price']);
						}
						?>
					</td>
					<td>
						<?php
						// show misc. details
						$misc = array();
						if ($type == SALE)
						{
							$misc[] = ($arr['ini_newused']==ITEM_NEW?'New':'Used');
							$misc[] = ($arr['ini_box']==BOX?'Box':($arr['ini_box']==NOBOX?'No Box':'Store Box'));
						}
						elseif ($type == TRADE)
						{
							$misc[] = ($arr['ini_trade_type']==CASH?'Cash':($arr['ini_trade_type']==CREDIT?'Credit':'Neither'));
							$misc[] = ($arr['ini_box']==BOX?'Box':($arr['ini_box']==NOBOX?'No Box':'Store Box'));
							$misc[] = ($arr['ini_condition']==GOOD?'Good':($arr['ini_condition']==FAIR?'Fair':'Poor'));
						}
						elseif ($type == RETURNS)
						{
							$misc[] = ($arr['ini_trade_type']==CASH?'Cash':($arr['ini_trade_type']==CREDIT?'Credit':'Neither'));
							$misc[] = ($arr['ini_newused']==ITEM_NEW?'New':'Used');
							$misc[] = ($arr['ini_opened']==OPENED?'Opened':($arr['ini_opened']==UNOPENED?'Unopened':'Broken'));
							$misc[] = ($arr['ini_return_charged']==YES?'Charged':'Not Charged');
							$misc[] = '<br />Orig Purch Date: '.date('m/d/Y',$arr['ini_return_purchdate']);
							$misc[] = '<br />Orig Purch Price: $'.sprintf('%0.2f',$arr['ini_return_purchprice']);
						}

						echo str_replace(' / <br />','<br />',implode(' / ',$misc));
						?>
					</td>
				</tr>
				<?php
			} // each history item
		} // each time->history items

		?>
		<tr bgcolor="<?php echo $pg->color('table-cell2');?>" id="itemnone" style="display:none">
			<td colspan="6" align="center">- None -</td>
		</tr>
		<?php
		$pg->outlineTableFoot();
		?>

		<p>
			<font size="1">
				<a href="javascript:void(0)" onclick="showAllCustomers()">Show All Customer Data</a>
				/
				<a href="javascript:void(0)" onclick="hideAllCustomers()">Hide All Customer Data</a>
			</font>
		</p>

		<script type="text/javascript">
			var item_types = [<?php echo implode(',', $types);?>];
		</script>
		<?php
	}
}

$pg->foot();

function getTypeName($type)
{
	$types = array(
		SALE     => 'Sales',
		TRADE    => 'Trades',
		RETURNS  => 'Returns',
		RECEIVED => 'Received Items'
	);
	return @$types[$type];
}
?>