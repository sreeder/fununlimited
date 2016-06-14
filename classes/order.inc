<?php
/**
* Order generation class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class order
{
	/**
	* page class reference
	* @var reference $pg
	*/
	var $pg;

	/**
	* Error reporting class
	* @var identifier $error
	*/
	var $error;

	/**
	* Current orderID
	* @var integer $orderID
	*/
	var $orderID;

	/**
	* Order criteria
	* @var array $criteria
	*/
	var $criteria;

	/**
	* All previous orders
	* @var array $orders
	*/
	var $orders;

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	*/
	function order(&$pg)
	{
		$this->pg =& $pg;
		$this->orderID = 0;
		$this->criteria = array();
		$this->orders = array();

		$this->error = new error('Order generation class');
	}

	/**
	* Pull in all previous orders for this store
	*/
	function get_orders()
	{
		global $db;

		$this->orders = array();

		$sql = "SELECT ord_orderID,ord_storeID,ord_criteria,ord_time,COUNT(ori_itemID) AS ord_numitems,SUM(ori_price) AS ord_total FROM orders,order_items WHERE ord_storeID={$_SESSION['storeID']} AND ord_orderID=ori_orderID GROUP BY ori_orderID ORDER BY ord_time";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result)) { $this->orders[] = $row; }
	}

	/**
	* Pulls the information from $_POST
	*/
	function pull_post()
	{
		$this->orderID = @$_POST['orderID'];
		if ($this->orderID) { $this->set_orderID($this->orderID); }

		if (isset($_POST['criteria']))
		{
			$this->criteria = $_POST['criteria'];
			while (list($key,$val) = each($this->criteria)) { $this->criteria[$key] = stripslashes($val); }
			reset($this->criteria);
		}
	}

	/**
	* Output the criteria form
	*/
	function criteria_form()
	{
		global $db;

		// determine the action
		$act = 'platform';
		if (isset($this->criteria['platformID'])) { $act = 'source'; }
		if (isset($this->criteria['sourceID'])) { $act = 'vals'; }
		if (isset($this->criteria['wishlist'])) { $act = 'generate'; }

		if ($act != 'generate')
		{
			$messages = array(
				'platform'=>'Please select a platform',
				'source'=>'Please select a price source',
				'vals'=>'Please enter your desired criteria'
			);
			echo $messages[$act].':<p />';

			?>
			<script type="text/javascript">
				function verify(frm)
				{
					if (<?=($act=='source'?'true':'false');?> && frm.elements['criteria[sourceID]'].selectedIndex == 0) { alert('You must select a price source.'); frm.elements['criteria[sourceID]'].focus(); return false; }
					else if (<?=($act=='vals'?'true':'false');?>)
					{
						if (frm.elements['criteria[amount]'].value == '') { alert('You must enter an order amount.'); frm.elements['criteria[amount]'].focus(); return false; }
						else if (frm.elements['criteria[amount]'].value <= 0) { alert('Order amount must be positive.'); frm.elements['criteria[amount]'].select(); return false; }
						else if (frm.elements['criteria[wishlist]'].value == '') { alert('You must enter the number of points to add each time an item is found in a wishlist.'); frm.elements['criteria[wishlist]'].focus(); return false; }
						else if (frm.elements['criteria[query]'].value == '') { alert('You must enter the number of points to add each time an item is queried in the quick item lookup.'); frm.elements['criteria[wishlist]'].focus(); return false; }
						else if (frm.elements['criteria[order_week]'].value == '') { alert('You must enter the number of points to add each time an item has been ordered in the last 7 days.'); frm.elements['criteria[order_week]'].focus(); return false; }
						else if (frm.elements['criteria[order_month]'].value == '') { alert('You must enter the number of points to add each time an item has been ordered in the last 30 days.'); frm.elements['criteria[order_month]'].focus(); return false; }
						else if (frm.elements['criteria[order_ever]'].value == '') { alert('You must enter the number of points to add each time an item has been ordered.'); frm.elements['criteria[order_ever]'].focus(); return false; }
						else { return confirm('Are you sure you want to submit this order?\n\nOK = YES, Cancel = No\n\nPlease wait after pressing OK - it may take a moment to generate your order!'); }
					}
					else { return true; }
				}
			</script>
			<?php
		}

		$this->pg->outlineTableHead();

		if ($act != 'generate')
		{
			?>
			<form method="post" action="/admin/utilities/order.php" onsubmit="return verify(this)">
			<input type="hidden" name="act" value="<?=($act=='vals'?'generate':'criteria');?>">
			<?php
			while (list($k,$v) = each($this->criteria)) { ?><input type="hidden" name="criteria[<?=$k;?>]" value="<?=$v;?>"><?php }
			reset($this->criteria);
		}

		?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Platform:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<?php
				if ($act == 'platform')
				{
					// show the platform selection form
					$platformIDs = array();
					$platforms = array();
					$pla = new platforms($this->pg,0);
					$pla->set_item('platforms');
					while (list($a,$arr) = each($pla->values))
					{
						$platformIDs[] = $arr[0];
						$platforms[] = $arr;
					}

					$newplatforms = array();
					$sql = "SELECT pla_name,itm_platformID,COUNT(itm_platformID) AS count FROM items,platforms WHERE itm_platformID=pla_platformID GROUP BY itm_platformID ORDER BY pla_name";
					$result = mysql_query($sql,$db);
					$this->error->mysql(__FILE__,__LINE__);
					while ($row = mysql_fetch_assoc($result))
					{
						$platformID = $row['itm_platformID'];
						$count = $row['count'];

						$totprice = 0;
						$pla->platformID = $platformID;
						$pla->set_item('sources');
						while (list($a,$arr) = each($pla->values))
						{
							if ($arr[2] == PRICE) { $totprice++; }
						}

						if ($totprice)
						{
							$idx = array_search($platformID,$platformIDs);
							$newplatforms[] = $platforms[$idx];
						}
					}
					$platforms = $newplatforms;

					?>
					<select name="criteria[platformID]" size="1" style="vertical-align:middle"><option value="0">All</option><?php
						while (list($a,list($platformID,$name)) = each($platforms))
						{
							?><option value="<?=$platformID;?>"><?=$name;?></option><?php
						}
					?></select>
					<?php
				}
				else
				{
					$pla = new platforms($this->pg,$this->criteria['platformID']);
					if ($this->criteria['platformID'] == 0) { echo "All"; }
					else { echo $pla->platform_name(); }
				}
				?>
			</td>
		</tr>
		<?php
		if ($act != 'platform')
		{
			?>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Price Source:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>">
					<?php
					$pla->set_item('sources');
					if ($act == 'source')
					{
						// show the source selection form
						$platformIDs = array();
						if ($this->criteria['platformID'] == 0)
						{
							// get all of the platform IDs
							$pla->set_item('platforms');
							while (list($a,$arr) = each($pla->values)) { $platformIDs[] = $arr[0]; }
						}
						else { $platformIDs[] = $this->criteria['platformID']; }

						$sources = array();
						$source_names = array();
						while (list($a,$platformID) = each($platformIDs))
						{
							$pla->platformID = $platformID;
							$pla->set_item('sources');
							while (list($a,$arr) = each($pla->values))
							{
								if ($arr[2] == PRICE && !in_array($arr[1],$source_names))
								{
									$sources[] = $arr;
									$source_names[] = $arr[1];
								}
							}
						}

						// sort $sources
						$newsources = array();
						sort($source_names);
						while (list($a,$name) = each($source_names))
						{
							while (list($a,$arr) = each($sources))
							{
								if ($arr[1] == $name) { $newsources[] = $arr; break; }
							}
							reset($sources);
						}
						$sources = $newsources;

						?>
						<select name="criteria[sourceID]" size="1" style="vertical-align:middle" onchange="change_affected(this.value)"><option value=""></option><?php
							while (list($a,list($sourceID,$name)) = each($sources))
							{
								?><option value="<?=$sourceID;?>"><?=$name;?></option><?php
							}
							reset($sources);
						?></select>
						<?php
					}
					else
					{
						if ($this->criteria['platformID'] == 0)
						{
							$sql = "SELECT sor_name FROM sources WHERE sor_sourceID={$this->criteria['sourceID']}";
							$result = mysql_query($sql,$db);
							$this->error->mysql(__FILE__,__LINE__);
							if (mysql_num_rows($result))
							{
								$row = mysql_fetch_assoc($result);
								echo $row['sor_name'];
							}
							else { echo 'Unknown'; }
						}
						else
						{
							while (list($a,$arr) = each($pla->values))
							{
								if ($arr[0] == $this->criteria['sourceID']) { echo $arr[1]; break; }
							}
						}
					}
					?>
				</td>
			</tr>
			<?php
		}
		if ($act != 'platform' && $act != 'source')
		{
			// output the item criteria form
			/*
			default criteria: (can change point value on all)
				1-10 for ratings [checkbox to include or not - default checked / cannot change value]
				5 for items in wish lists (each time)
				1 for items ordered in the past 7 days (each time)
				0.5 for items ordered in the past 30 days (each time)
				0.1 for items ordered any time (each time)
			*/

			?>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Order Amount:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>"><?php
					if ($act == 'vals') { ?>$<input type="text" name="criteria[amount]" size="8" value="500" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value)" style="text-align:right"><?php }
					else { ?>$<?=number_format($this->criteria['amount'],2);?><?php }
				?></td>
			</tr>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Criteria:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>">
					<?php
					if ($act == 'vals')
					{
						?>
						<input type="checkbox" name="criteria[checklowest]" value="<?=YES;?>" checked="checked" style="border-width:0px;vertical-align:middle" id="checklowest"> <label for="checklowest">Check/show lowest price (all price sources)</label><br />
						<input type="checkbox" name="criteria[orbest]" value="<?=YES;?>" checked="checked" style="border-width:0px;vertical-align:middle" id="orbest"> <label for="orbest">Show &quot;Or Best Price&quot; if source price not lowest (<b>Check/show...</b> must be checked)</label><br />
						<input type="checkbox" name="criteria[rating]" value="<?=YES;?>" checked="checked" style="border-width:0px;vertical-align:middle" id="rating"> <label for="rating">Include average ratings (1-10 points)</label><br />
						<input type="text" name="criteria[wishlist]" size="3" value="5" onkeypress="return onlynumbers(this.value,event,true)"> points for each time item found in a wish list<br />
						<input type="text" name="criteria[query]" size="3" value="2.5" onkeypress="return onlynumbers(this.value,event,true)"> points for each time item queried in quick item lookup<br />
						<input type="text" name="criteria[order_week]" size="3" value="1" onkeypress="return onlynumbers(this.value,event,true)"> points for each time item ordered in the last 7 days<br />
						<input type="text" name="criteria[order_month]" size="3" value="0.5" onkeypress="return onlynumbers(this.value,event,true)"> points for each time item ordered in the last 30 days<br />
						<input type="text" name="criteria[order_ever]" size="3" value="0.1" onkeypress="return onlynumbers(this.value,event,true)"> points for each time item ever ordered (30+ days ago)
						<?php
					}
					else
					{
						?>
						Include average ratings (1-10 points): <b><?=(@$this->criteria['rating']==YES?'YES':'No');?></b><br />
						<b><?=$this->criteria['wishlist'];?></b> points for each time item found in a wish list<br />
						<b><?=$this->criteria['query'];?></b> points for each time item queried in quick item lookup<br />
						<b><?=$this->criteria['order_week'];?></b> points for each time item ordered in the last 7 days<br />
						<b><?=$this->criteria['order_month'];?></b> points for each time item ordered in the last 30 days<br />
						<b><?=$this->criteria['order_ever'];?></b> points for each time item ever ordered (30+ days ago)
						<?php
					}
					?>
				</td>
			</tr>
			<?php
		}

		$this->pg->outlineTableFoot();

		?><p /><?php

		if ($act == 'source' && $this->criteria['platformID'] == 0)
		{
			?>
			<font size="1"><b>Note:</b> Only platforms with pricing from the selected source will be used.</font>
			<p />
			<?php
		}

		if ($act != 'generate')
		{
			?>
			<script type="text/javascript">
				function change_affected(sourceID)
				{
					if (<?=($act=='source'&&$this->criteria['platformID']==0?'true':'false');?>)
					{
						if (sourceID == '') { val = ''; } else { val = eval('p'+sourceID); }
						document.getElementById('platformlist').innerText = val;
					}
				}

				<?php
				if ($act == 'source' && $this->criteria['platformID'] == 0)
				{
					// output the affected platforms
					while (list($a,$arr) = each($sources))
					{
						$platforms = array();
						$sql = "SELECT pla_name FROM sources,platforms WHERE sor_name='".mysql_escape_string($arr[1])."' AND sor_platformID=pla_platformID ORDER BY pla_name";
						$result = mysql_query($sql,$db);
						$this->error->mysql(__FILE__,__LINE__);

						while ($row = mysql_fetch_assoc($result)) { $platforms[] = $row['pla_name']; }

						?>p<?=$arr[0];?>="<?=implode('; ',$platforms);?>";<?php
					}
					reset($sources);
				}
				?>
			</script>
			<div id="platforms" style="display:<?=($act=='source'&&$this->criteria['platformID']==0?'block':'none');?>;border:solid #000000 1px;width:500px;text-align:left;padding:3px;background:#EEEEEE">
				<font size="1">
					<b>Affected Platforms:</b><br />
					<div id="platformlist">&nbsp;</div>
				</font>
			</div>
			<p />
			<input type="submit" value="Continue &gt;" class="btn" />
			<input type="reset" value="Reset Form" class="btn" />
			<p />
			<?=$this->new_button('Start Over');?>
			</form>
			<?php
		}
	}

	/**
	* Generate the order according to the criteria
	*/
	function generate()
	{
		global $db;

		/*
		Array
		(
		    [platformID] => 33
		    [sourceID] => 1
		    [amount] => 500.00
				[checklowest] => 1
				[orbest] => 1
		    [rating] => 1
		    [wishlist] => 5
				[query] => 2.5
		    [order_week] => 1
		    [order_month] => 0.5
		    [order_ever] => 0.1
		)
		*/

		while (list($k,$v) = each($this->criteria)) { $$k = $v; }
		reset($this->criteria);

		$platformIDs = array();
		$sourceIDs = array();
		if ($platformID == 0)
		{
			$sql = "SELECT sor_name FROM sources WHERE sor_sourceID=$sourceID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			if (mysql_num_rows($result))
			{
				$row = mysql_fetch_assoc($result);
				$desc = $row['sor_name'];
			}
			else { $this->pg->error('Unable to retrieve source description'); $this->pg->foot(); die(); }

			// pull in all sources that match the selected source name (this is so source pricing that affect multiple platforms is used)
			$sql = "SELECT pla_platformID,sor_sourceID FROM sources,platforms WHERE sor_name='".mysql_escape_string($desc)."' AND sor_platformID=pla_platformID ORDER BY pla_name";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			while ($row = mysql_fetch_assoc($result))
			{
				$platformIDs[] = $row['pla_platformID'];
				$sourceIDs[] = $row['sor_sourceID'];
			}
		}
		else
		{
			$platformIDs[] = $platformID;
			$sourceIDs[] = $sourceID;
		}

		$itemIDs = array(); // itemIDs to lookup (none on hand, source has price)
		$items = array(); // format: $items[itemID] = array of info
		$sql = "SELECT * FROM item_source_values,items,platforms,quantity WHERE isv_sourceID IN (".implode(',',$sourceIDs).") AND isv_value>0 AND isv_itemID=itm_itemID AND itm_platformID IN (".implode(',',$platformIDs).") AND itm_platformID=pla_platformID AND qty_storeID={$_SESSION['storeID']} AND qty_itemID=itm_itemID AND (qty_new=0 OR qty_used=0) ORDER BY itm_title";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			$itemIDs[] = $row['itm_itemID'];
			$items[$row['itm_itemID']] = $row;
		}

		// pull in the lowest price from any source
		$lowest = array(); // format: $lowest[itemID] array(sourceID,price)
		if ($checklowest)
		{
			$sql = "SELECT isv_itemID,isv_sourceID,isv_value FROM sources,item_source_values WHERE sor_type=".PRICE." AND sor_sourceID=isv_sourceID AND isv_itemID IN (".implode(',',$itemIDs).") ORDER BY isv_itemID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			while ($row = mysql_fetch_assoc($result))
			{
				if (!isset($lowest[$row['isv_itemID']])) { $lowest[$row['isv_itemID']] = array(0,99999999); }

				list($sourceID,$low) = $lowest[$row['isv_itemID']];

				if ($row['isv_value'] < $low) { $lowest[$row['isv_itemID']] = array($row['isv_sourceID'],$row['isv_value']); }
			}
		}

		// begin calculating points
		$points = array();
		while (list($a,$itemID) = each($itemIDs)) { $points[$itemID] = 0; }
		reset($itemIDs);

		// if chosen, add in average ratings
		if (@$rating == YES)
		{
			$sql = "SELECT AVG(isv_value) AS average,isv_itemID FROM item_source_values WHERE isv_itemID IN (".implode(',',$itemIDs).") GROUP BY isv_itemID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				$points[$row['isv_itemID']] += $row['average'];
			}
		}

		// add in wish list points
		$sql = "SELECT wli_itemID FROM wishlist_items WHERE wli_itemID IN (".implode(',',$itemIDs).")";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result))
		{
			$points[$row['wli_itemID']] += $wishlist;
		}

		// add in search bar query points
		$sql = "SELECT qck_itemID FROM quick_lookups WHERE qck_storeID={$_SESSION['storeID']} AND qck_itemID IN (".implode(',',$itemIDs).")";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result))
		{
			$itemID = $row['qck_itemID'];
			$points[$itemID] += $query;
		}

		// add in order points
		$sql = "SELECT ini_itemID,ini_timeadded FROM invoice_items WHERE ini_itemID IN (".implode(',',$itemIDs).") ORDER BY ini_itemID,ini_timeadded";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result))
		{
			$itemID = $row['ini_itemID'];
			$time = (time()-$row['ini_timeadded']);

			if ($time <= (86400*7)) { $points[$itemID] += $order_week; } // ordered in last 7 days
			elseif ($time <= (86400*30)) { $points[$itemID] += $order_month; } // ordered in the last 30 days
			else { $points[$itemID] += $order_ever; } // order 30+ days ago
		}

		if (NO)
		{
			$this->pg->outlineTableHead('100%');
			?>
			<tr>
				<td bgcolor="#FFFFFF" width="50%">unsorted:<pre><?=print_r($points);?></pre></td>
				<td bgcolor="#FFFFFF" width="50%"><?php arsort($points); ?>sorted:<pre><?=print_r($points);?></pre></td>
			</tr>
			<?php
			$this->pg->outlineTableFoot();
		}
		else { arsort($points); }

		// build list of items to buy up to the amount
		$total = 0;
		$buy_itemIDs = array();

		while (list($itemID,$pnt) = each($points))
		{
			$info = $items[$itemID];
			$price = $info['isv_value'];

			if (($total+$price) > $amount) { break; }
			else
			{
				$total += $price;
				$buy_itemIDs[] = $itemID;
			}
		}

		// set the necessary session variables
		$_SESSION['order_allitems'] = $items;
		$_SESSION['order_itemIDs'] = $buy_itemIDs;
		$_SESSION['order_criteria'] = $this->criteria;
		$_SESSION['order_lowest'] = $lowest;

		// output the total/javascript to open the print-list window
		?>
		Total Items: <b><?=count($buy_itemIDs);?></b><br />
		Total Price: <b>$<?=number_format($total,2);?></b>
		<p />
		Please hold while the printable order list is generated...
		<p />
		If a window does not popup with your printable list, <a href="javascript:print_window()">click here</a>.
		<p />
		<?=$this->new_button('Create Another Order');?>
		<input type="button" value="&lt; Return to Previous Order List" onclick="document.location='/admin/utilities/order.php'" class="btn" />

		<script type="text/javascript">
			function print_window() { open_window('/admin/utilities/order_print.php','print',750,450,'YES',true); }
		</script>
		<?php
		$this->pg->addOnload('print_window()');
	}

	/**
	* Put the order in the database
	*/
	function put_in_database()
	{
		global $db;

		$itemIDs = @$_SESSION['order_itemIDs'];
		$items = @$_SESSION['order_allitems'];
		$this->criteria = @$_SESSION['order_criteria'];
		$ser_crit = serialize($this->criteria); // serialize criteria

		$sql = "INSERT INTO orders VALUES (NULL,{$_SESSION['storeID']},'".mysql_escape_string($ser_crit)."','".time()."')";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		$orderID = mysql_insert_id();

		$vals = array();
		$byplatform = $this->get_by_platform($itemIDs,$items);

		while (list($platform_name,$pla_items) = each($byplatform))
		{
			while (list($a,$arr) = each($pla_items))
			{
				$vals[] = "($orderID,{$arr['itm_itemID']},'".mysql_escape_string($arr['pla_name'])."','".mysql_escape_string($arr['itm_title'])."',{$arr['isv_value']})";
			}
		}

		$sql = "INSERT INTO order_items VALUES ".implode(',',$vals);
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
	}

	/**
	* Output the printable list of items stored in $_SESSION['order_itemIDs']
	* @param integer $table_only output only the table? [optional, default NO]
	*/
	function printable_list($table_only=NO)
	{
		global $db;

		$itemIDs = @$_SESSION['order_itemIDs'];
		$items = @$_SESSION['order_allitems'];
		$this->criteria = @$_SESSION['order_criteria'];
		$lowest = @$_SESSION['order_lowest'];

		if (!is_array($itemIDs) || !is_array($items) || !is_array($this->criteria)) { echo "Session variables do not exist"; }
		else
		{
			$byplatform = $this->get_by_platform($itemIDs,$items);

			$cols = 3; // !!! TRY 2 FOR PLATFORMS WITH LONGER ITEM TITLES !!!

			if ($table_only == NO)
			{
				$pla = new platforms($this->pg,$this->criteria['platformID']);
				?>
				<font size="3"><b><?=$_SESSION['store_info']['sto_name'];?> - Order</b></font>
				<p />

				<b>Platform:</b> <?php
				if ($this->criteria['platformID'] == 0) { echo "All"; }
				else { echo $pla->platform_name(); }
				?><br />

				<b>Source:</b> <?php
				$pla->set_item('sources');
				if ($this->criteria['platformID'] == 0)
				{
					$sql = "SELECT sor_name FROM sources WHERE sor_sourceID={$this->criteria['sourceID']}";
					$result = mysql_query($sql,$db);
					$this->error->mysql(__FILE__,__LINE__);
					if (mysql_num_rows($result))
					{
						$row = mysql_fetch_assoc($result);
						echo $row['sor_name'];
					}
					else { echo 'Unknown'; }
				}
				else
				{
					while (list($a,$arr) = each($pla->values))
					{
						if ($arr[0] == $this->criteria['sourceID']) { echo $arr[1]; break; }
					}
				}
				?><br />

				<b>Date/Time:</b> <?=date('m/d/Y h:ia');?>

				<p />
				<?php

				if (@$this->criteria['checklowest'])
				{
					?><font size="1"><b>Note:</b> Values in parentheses represent the lowest price<br />we can purchase this item for from another source.</font><?php
					if (@$this->criteria['orbest'])
					{
						?><br /><font size="1"><b>Note:</b> Or Best Price = <img src="/images/obparrow.gif" width="72" height="10" /></font><?php
					}
					?><p /><?php
				}
			}

			?>
			<table border="0" cellspacing="0" cellpadding="2">
			<tr><td colspan="<?=($cols*2);?>"><hr width="100%" size="-1" color="#000000" noshade="noshade" /></td></tr>
			<?php
			$alltotal = 0;
			while (list($platform_name,$pla_items) = each($byplatform))
			{
				$total = 0;
				?><tr><td colspan="<?=($cols*2);?>" align="center"><b><?=$platform_name;?></b></td></tr><?php

				$num = ceil(count($pla_items)/$cols);

				for ($i=0; $i<$num; $i++)
				{
					$show = array();
					for ($j=0; $j<$cols; $j++) { $show[] = ($i+($num*$j)); }

					?>
					<tr>
						<?php
						while (list($a,$idx) = each($show))
						{
							if (isset($pla_items[$idx]))
							{
								$itemID = $pla_items[$idx]['itm_itemID'];
								$info = $items[$itemID];

								// if necessary, find/show the lowest available price
								$low = "";
								$obp1 = ""; $obp2 = "";
								if (@$this->criteria['checklowest'] && isset($lowest[$itemID]))
								{
									list($srcID,$lowprice) = $lowest[$itemID];
									if ($srcID != $this->criteria['sourceID'] && $lowprice < $info['isv_value'])
									{
										$low = " (".number_format($lowprice,2).")";

										if (@$this->criteria['orbest'])
										{
											//$obp1 = '<br /><span style="width:100%;text-align:right">Or Best Price</span>';
											$obp2 = '<br /><img src="/images/obparrow.gif" width="72" height="10" />';
										}
									}
								}

								?>
								<td valign="top"><?=$info['itm_title'].$obp1;?></td>
								<td align="right" valign="top">&nbsp;&nbsp;<?=number_format($info['isv_value'],2).$low;?>&nbsp;&nbsp;<?=$obp2;?></td>
								<?php

								$total += $info['isv_value'];
								$alltotal += $info['isv_value'];
							}
							else { ?><td colspan="2">&nbsp;</td><?php }
						}
						?>
					</tr>
					<?php
				}

				?>
				<tr>
					<td colspan="<?=($cols*2);?>" align="right">
						<b><?=$platform_name;?> Total: &nbsp; $<?=number_format($total,2);?></b><br />
						<hr width="100%" size="-1" color="#000000" noshade="noshade" />
					</td>
				</tr>
				<?php
			}
			?>
			</table>
			<p />
			<b>Grand Total: $<?=number_format($alltotal,2);?> (<?=count($itemIDs);?> item<?=(count($itemIDs)==1?'':'s');?>)</b>
			<?php

			//if ($table_only == NO) { $this->pg->addOnload('window.print()'); }
		}
	}

	/**
	* Return the items sorted by platform/title
	* @param array $itemIDs
	* @param array $items array of item information
	*/
	function get_by_platform($itemIDs,$items)
	{
		$byplatform = array(); // format: $byplatform[platform_name] = array of items

		while (list($a,$itemID) = each($itemIDs))
		{
			$arr = @$items[$itemID];
			if (count($arr))
			{
				if (!isset($byplatform[$arr['pla_name']])) { $byplatform[$arr['pla_name']] = array(); }
				$byplatform[$arr['pla_name']][] = $arr;
			}
		}

		// sort the items alphabetically
		ksort($byplatform);
		$sort = new sort();
		while (list($platform_name,$pla_items) = each($byplatform))
		{
			$byplatform[$platform_name] = $sort->doSort($byplatform[$platform_name],'itm_title',SORT_ASC,SORT_STRING);
		}
		reset($byplatform);

		return $byplatform;
	}

	/**
	* Output the button to take the user back to the new criteria page
	* @param string $text
	*/
	function new_button($text)
	{
		?><input type="button" value="<?=$text;?> &gt;" onclick="document.location='/admin/utilities/order.php?act=criteria'" class="btn" /><?php
	}

	/**
	* Output the list of previous orders with buttons to view/delete
	*/
	function show_list()
	{
		global $db;

		$this->get_orders();

		$this->new_button('Generate New Order');
		?><p /><?php

		?><b>Previous Orders</b><p /><?php

		$this->pg->outlineTableHead();
		?>
		<form name="ordsel" id="ordsel" onsubmit="return false">
		<tr>
			<td bgcolor="<?=$this->pg->color('table-head');?>">&nbsp;</td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b>Date/Time</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b>Platform</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b>Price Source</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b># Items</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b>Total</b></td>
		</tr>
		<?php
		while (list($a,$arr) = each($this->orders))
		{
			$bg = (($a%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

			$crit = unserialize($arr['ord_criteria']);

			$pla = new platforms($this->pg,$crit['platformID']);
			if ($crit['platformID'] == 0) { $platform_name = "All"; }
			else { $platform_name = $pla->platform_name(); }

			$pla->set_item('sources');
			if ($crit['platformID'] == 0)
			{
				$sql = "SELECT sor_name FROM sources WHERE sor_sourceID={$crit['sourceID']}";
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
				if (mysql_num_rows($result))
				{
					$row = mysql_fetch_assoc($result);
					$source_name = $row['sor_name'];
				}
				else { $source_name = 'Unknown'; }
			}
			else
			{
				while (list($b,$sarr) = each($pla->values))
				{
					if ($sarr[0] == $crit['sourceID']) { $source_name = $sarr[1]; break; }
				}
			}

			?>
			<label for="ord<?=$arr['ord_orderID'];?>">
			<tr onclick="set_orderID(<?=$arr['ord_orderID'];?>)" style="cursor:hand">
				<td bgcolor="<?=$bg;?>"><input type="radio" name="orderID" id="ord<?=$arr['ord_orderID'];?>" style="border-width:0" onclick="set_orderID(<?=$arr['ord_orderID'];?>)"></td>
				<td bgcolor="<?=$bg;?>"><?=date('m/d/Y h:ia',$arr['ord_time']);?></td>
				<td bgcolor="<?=$bg;?>"><?=$platform_name;?></td>
				<td bgcolor="<?=$bg;?>"><?=$source_name;?></td>
				<td bgcolor="<?=$bg;?>"><?=$arr['ord_numitems'];?></td>
				<td bgcolor="<?=$bg;?>">$<?=number_format($arr['ord_total'],2);?></td>
			</tr>
			</label>
			<?php
		}
		if (!count($this->orders)) { ?><tr><td colspan="6" align="center" bgcolor="<?=$this->pg->color('table-cell');?>">No previous orders found</td></tr><?php }

		?>
		</form>
		<?php
		$this->pg->outlineTableFoot();

		if (count($this->orders))
		{
			?>
			<p />
			<script type="text/javascript">
				var orderID=0;
				function set_orderID(id)
				{
					if (isNS) { document.getElementById('ordsel').eval('ord'+id).checked = true; }

					orderID = id;
					document.orders.v.disabled = false;
					document.orders.d.disabled = false;
				}

				function go(where)
				{
					if ((orderID && where == 'view') || (orderID && where == 'delete' && confirm('Are you sure you want to delete the selected order?')))
					{
						var frm = document.orders;
						frm.act.value = where;
						frm.orderID.value = orderID;
						frm.submit();
					}
				}
			</script>

			<form method="post" action="/admin/utilities/order.php" name="orders">
			<input type="hidden" name="act" value="">
			<input type="hidden" name="orderID" value="">
			<input type="button" value="View Order Details &gt;" id="v" onclick="go('view')" disabled="true" class="btn">
			<input type="button" value="Delete Order &gt;" id="d" onclick="go('delete')" disabled="true" class="btn">
			</form>
			<?php
		}
	}

	/**
	* View a previous order
	* @param integer $orderID
	*/
	function view($orderID)
	{
		global $db;

		$sql = "SELECT * FROM orders WHERE ord_storeID={$_SESSION['storeID']} AND ord_orderID=$orderID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		if (!mysql_num_rows($result)) { echo "Invalid orderID: $orderID"; }
		else
		{
			$info = mysql_fetch_assoc($result);
			$this->criteria = unserialize($info['ord_criteria']);
			$itemIDs = array();

			$items = array(); // format: $items[itemID] = array of info
			$sql = "SELECT * FROM order_items WHERE ori_orderID=$orderID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				// trick $this->printable_list()
				$row['itm_itemID'] = $row['ori_itemID'];
				$row['itm_title'] = $row['ori_title'];
				$row['pla_name'] = $row['ori_platform'];
				$row['isv_value'] = $row['ori_price'];

				$itemIDs[] = $row['ori_itemID'];
				$items[$row['ori_itemID']] = $row;
			}

			$_SESSION['order_itemIDs'] = $itemIDs;
			$_SESSION['order_allitems'] = $items;
			$_SESSION['order_criteria'] = $this->criteria;

			?>
			<input type="button" value="&lt; Return to Previous Order List" onclick="document.location='/admin/utilities/order.php'" class="btn" />
			<p />
			<?php

			$this->criteria_form();
			$this->printable_list(YES);

			?>
			<p />
			<input type="button" value="&lt; Return to Previous Order List" onclick="document.location='/admin/utilities/order.php'" class="btn" />
			<?php
		}
	}

	/**
	* Delete a previous order
	* @param integer $orderID
	*/
	function delete($orderID)
	{
		global $db;

		$sql = "DELETE FROM orders WHERE ord_storeID={$_SESSION['storeID']} AND ord_orderID=$orderID";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		if (mysql_affected_rows())
		{
			$sql = "DELETE FROM order_items WHERE ori_orderID=$orderID";
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$this->pg->status('Order deleted');
		}
		else { $this->pg->error('Unable to delete order'); }

	}
}

?>