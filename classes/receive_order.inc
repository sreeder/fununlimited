<?php
/**
* Receive order class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class receive_order
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
	* Current order information
	* @var array $info
	*/
	var $info;

	/**
	* Items in the current order
	* @var array $items
	*/
	var $items;

	/**
	* Item quantities to add to the store's quantity when an order is completed
	* @var array $quantity
	*/
	var $quantity;

	/**
	* All previous received orders
	* @var array $orders
	*/
	var $orders;

	/**
	* Default shown order limit
	* @var integer $default_limit
	*/
	var $default_limit;

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	*/
	function receive_order(&$pg)
	{
		$this->pg =& $pg;
		$this->orderID = 0;
		$this->orders = array();
		$this->info = array();
		$this->items = array();
		$this->default_limit = 10;

		$this->error = new error('Receive order class');
	}

	/**
	* Pull in all previous received orders for this store
	* @param integer $limit how many orders to pull in?
	*/
	function get_orders($limit)
	{
		global $db;

		$this->orders = array();

		$sql = "SELECT * FROM received_orders WHERE ror_storeID={$_SESSION['storeID']} ORDER BY ror_time DESC".(is_numeric($limit)?" LIMIT $limit":'');
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			$isql = "SELECT SUM(roi_qtynew) AS roi_qtynew,SUM(roi_qtyused) AS roi_qtyused FROM received_order_items WHERE roi_orderID={$row['ror_orderID']}";
			$iresult = mysql_query($isql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			if (mysql_num_rows($iresult))
			{
				$qtys = mysql_fetch_assoc($iresult);
				if (!$qtys['roi_qtynew']) { $qtys['roi_qtynew'] = 0; }
				if (!$qtys['roi_qtyused']) { $qtys['roi_qtyused'] = 0; }

				$row = array_merge($row,$qtys);
			}

			$this->orders[] = $row;
		}
	}

	/**
	* Pulls the information from $_POST
	*/
	function pull_post()
	{
		$this->orderID = @$_POST['orderID'];
		if ($this->orderID) { $this->set_orderID($this->orderID); }

		$this->quantity = @$_POST['quantity'];
		if (!is_array($this->quantity)) { $this->quantity = array(); }
	}

	/**
	* Set the active order ID and pull the information
	* @param integer $orderID
	*/
	function set_orderID($orderID=-1)
	{
		global $db;

		$_SESSION['receive_orderID'] = 0;
		$_SESSION['receive_info'] = array();
		$_SESSION['receive_items'] = array();

		if ($orderID)
		{
			$this->orderID = $orderID;
			$this->info = array();
			$this->items = array();

			$sql = "SELECT * FROM received_orders WHERE ror_orderID=$orderID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			if (mysql_num_rows($result))
			{
				$_SESSION['receive_orderID'] = $orderID;
				$this->info = mysql_fetch_assoc($result);

				$sql = "SELECT * FROM received_order_items WHERE roi_orderID=$orderID ORDER BY roi_platform,roi_title";
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);

				while ($row = mysql_fetch_assoc($result)) { $this->items[] = $row; }

				$_SESSION['receive_info'] = $this->info;
				$_SESSION['receive_items'] = $this->items;
			}
		}
	}

	/**
	* Output the list of previously received orders with buttons to view/delete
	* @param integer $limit how many orders to show? [optional, default $this->default_limit]
	*/
	function show_list($limit='')
	{
		global $db;

		if (!strlen($limit)) { $limit = $this->default_limit; }
		$this->get_orders($limit);

		if (isset($_SESSION['receive_order_status'])) { $this->pg->status($_SESSION['receive_order_status']); unset($_SESSION['receive_order_status']); }
		if (isset($_SESSION['receive_order_error'])) { $this->pg->error($_SESSION['receive_order_error']); unset($_SESSION['receive_order_error']); }

		// get previous distributors
		$prev_distribs = array();
		$sql = "SELECT DISTINCT ror_distributor FROM received_orders WHERE ror_storeID={$_SESSION['storeID']} ORDER BY ror_distributor";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result)) { $prev_distribs[] = $row['ror_distributor']; }

		?>
		<script type="text/javascript">
			var incompleteids = new Array();

			function verify(frm)
			{
				if (frm.distributor.value == '') { alert('Please enter a distributor.'); frm.distributor.focus(); return false; }
				else { return true; }
			}
		</script>
		<?php
		$this->pg->outlineTableHead();
		?>
		<form method="post" action="/admin/utilities/receive_order.php" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="new">
		<tr><td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Create New Order</b></td></tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>">
				Distributor: <input type="text" name="distributor" size="30" /><br />
				Previous Distributors: <select size="1" name="previous" onchange="this.form.distributor.value=this.value"><option value="">- Select Distributor -</option><?php
					while (list($a,$distrib) = each($prev_distribs)) { ?><option value="<?php echo htmlspecialchars($distrib);?>"><?php echo htmlspecialchars($distrib);?></option><?php }
				?></select>
			</td>
		</tr>
		<tr>
			<td align="center" bgcolor="<?php echo $this->pg->color('table-label');?>">
				<input type="submit" value="Create New Order &gt;" class="btn" />
			</td>
		</tr>
		<?php echo $this->pg->outlineTableFoot();?>
		<p />
		<b>Previous Orders<?php echo (!is_numeric($limit)?' ('.count($this->orders).')':'');?></b>
		<p />
		<?php
		if (is_numeric($limit)) { ?>Only the last <?php echo $limit;?> orders are being shown. Click <a href="receive_order.php?limit=all">here</a> to view all previous orders.<?php }
		else { ?>All orders are being shown. Click <a href="receive_order.php?limit=<?php echo $this->default_limit;?>">here</a> to limit to the last <?php echo $this->default_limit;?> orders.<?php }
		?><p /><?php

		$this->pg->outlineTableHead();
		?>
		<form name="ordsel" id="ordsel" onsubmit="return false">
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-head');?>">&nbsp;</td>
			<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Date/Time</b></td>
			<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Distributor</b></td>
			<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b># New</b></td>
			<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b># Used</b></td>
		</tr>
		<?php
		$hasincomplete = NO;
		while (list($a,$arr) = each($this->orders))
		{
			$bg = (($a%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

			if ($arr['ror_completed'] == NO)
			{
				$hasincomplete = YES;
				$f1 = '<font color="red">'; $f2 = '</font>';
				?><script type="text/javascript">incompleteids[incompleteids.length]=<?php echo $arr['ror_orderID'];?></script><?php
			}
			else { $f1 = ''; $f2 = ''; }
			?>
			<label for="ord<?php echo $arr['ror_orderID'];?>">
			<tr onclick="set_orderID(<?php echo $arr['ror_orderID'];?>)" style="cursor:hand">
				<td bgcolor="<?php echo $bg;?>"><input type="radio" name="orderID" id="ord<?php echo $arr['ror_orderID'];?>" style="border-width:0" onclick="set_orderID(<?php echo $arr['ror_orderID'];?>)"></td>
				<td bgcolor="<?php echo $bg;?>"><?php echo $f1;?><?php echo date('m/d/Y h:ia',$arr['ror_time']);?><?php echo $f2;?></td>
				<td bgcolor="<?php echo $bg;?>"><?php echo $f1;?><?php echo $arr['ror_distributor'];?><?php echo $f2;?></td>
				<td bgcolor="<?php echo $bg;?>" align="right"><?php echo $f1;?><?php echo $arr['roi_qtynew'];?><?php echo $f2;?></td>
				<td bgcolor="<?php echo $bg;?>" align="right"><?php echo $f1;?><?php echo $arr['roi_qtyused'];?><?php echo $f2;?></td>
			</tr>
			</label>
			<?php
		}
		if (!count($this->orders)) { ?><tr><td colspan="5" align="center" bgcolor="<?php echo $this->pg->color('table-cell');?>">No previous received orders found</td></tr><?php }

		?>
		</form>
		<?php
		$this->pg->outlineTableFoot();

		if (count($this->orders))
		{
			if ($hasincomplete) { ?><p /><font size="1"><b>Note:</b> <font color="red">Orders in red</font> were never completed.<br />Either delete them or reopen and complete them.</font><?php }

			?>
			<p />
			<script type="text/javascript">
				var orderID=0;
				function set_orderID(id)
				{
					if (isNS) { document.getElementById('ordsel').eval('ord'+id).checked = true; }

					orderID = id;
					if (in_array(id,incompleteids)) { document.orders.r.disabled = false; }
					document.orders.v.disabled = false;
					document.orders.d.disabled = false;
				}

				function go(where)
				{
					if ((orderID && (where == 'view' || where == 'reopen')) || (orderID && where == 'delete' && confirm('Are you sure you want to delete the selected order?')))
					{
						var frm = document.orders;
						frm.method = (where=='view'?'get':'post');
						frm.act.value = where;
						frm.orderID.value = orderID;
						frm.submit();
					}
				}
			</script>

			<form method="post" action="/admin/utilities/receive_order.php" name="orders">
			<input type="hidden" name="act" value="">
			<input type="hidden" name="orderID" value="">
			<input type="button" value="View Order Details &gt;" id="v" onclick="go('view')" disabled="true" class="btn">
			<input type="button" value="Reopen Order &gt;" id="r" onclick="go('reopen')" disabled="true" class="btn">
			<input type="button" value="Delete Order &gt;" id="d" onclick="go('delete')" disabled="true" class="btn">
			</form>
			<?php
		}
	}

	/**
	* Create a new order
	*/
	function create_new()
	{
		global $db;

		if (@$_SESSION['receive_orderID']) { $this->pg->error('An order has already been created.'); }
		else
		{
			$sql = "INSERT INTO received_orders VALUES (NULL,{$_SESSION['storeID']},'".mysql_escape_string(@$_POST['distributor'])."','".time()."',".NO.")";
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$orderID = mysql_insert_id();
			$this->set_orderID($orderID);
		}
	}

	/**
	* Output the item form
	*/
	function form()
	{
		/*
		100860114630
		100860720400
		497436563523
		788687500166
		722242515660
		752919520000
		735009401328
		404218301830
		786936207958
		276168799980
		628960135260
		786936125146
		197034604000
		146331462190
		888064112680
		788687500053
		100860101920
		613455701042
		960090872270
		853914192280
		*/
		$this->set_orderID(@$_SESSION['receive_orderID']);

		if (@$_SESSION['receive_orderID'])
		{
			if (!isset($_SESSION['receive_last_newused']) || $_SESSION['receive_last_newused'] == ITEM_NEW)
			{
				$c1 = ' checked="checked"';
				$c2 = '';
			}
			else
			{
				$c1 = '';
				$c2 = ' checked="checked"';
			}

			?>
			<b>Distributor:</b> <?php echo $_SESSION['receive_info']['ror_distributor'];?>
			<p />
			<?php
			if (isset($_SESSION['receive_order_status'])) { $this->pg->status($_SESSION['receive_order_status']); unset($_SESSION['receive_order_status']); }
			if (isset($_SESSION['receive_order_error'])) { $this->pg->error($_SESSION['receive_order_error']); unset($_SESSION['receive_order_error']); }
			?>
			<font size="1"><b>Note:</b> Your on-hand quantities will not be updated until you press <b>Complete Order</b></font>
			<p />

			<script type="text/javascript">
				// checks key presses in the UPC box
				// if enter, check the UPC
				// if n/u, change to new/used
				var upcs = new Array();
				var itemIDs = new Array();

				// go to the add new item page
				function newitem(upc)
				{
					document.location = '/admin/setup_items/items.php?act=add&setupc='+upc+'&fromreceiveorder=<?php echo YES;?>';
				}

				// lock the UPC/title textboxes
				var locked = false;
				function lock_form()
				{
					var frm = document.upc;

					frm.upctitle.value = '- Please Hold -';
					frm.upctitle.style.textAlign = 'center';
					frm.upctitle.disabled = true;
					frm.platformID.disabled = true;

					locked = true;
				}

				// unlock the UPC/title textboxes
				function unlock_form()
				{
					var frm = document.upc;

					frm.upctitle.value = '';
					frm.upctitle.style.textAlign = 'left';
					frm.upctitle.disabled = false;
					frm.platformID.disabled = false;

					frm.upctitle.focus();

					locked = false;
				}

				// ask if they would like to add a new item
				// if YES, go to the add new item page
				function ask_addnew(upc)
				{
					if (confirm('UPC number '+upc+' not found.\n\nWould you like to add this item?\n-------------------------\nOK/Enter = YES\nCancel/ESC = no'))
					{
						newitem(upc);
					}
					else
					{
						unlock_form();
					}
				}

				// if there is a UPC entered, check that it exists
				// if there is a title entered, output the item search box
				function do_item(frm)
				{
					if (!locked)
					{
						var val = frm.upctitle.value;
						var nu = (frm.newused[0].checked ? <?php echo ITEM_NEW;?> : <?php echo ITEM_USED;?>)
						var platformID = $F('rec_platformID');

						if (!val.length)
						{
							alert('Please enter a UPC or title.');
						}
						else if (val.length == 12)
						{
							check_upc(val,nu,platformID);
						}
						else
						{
							check_title(val,nu,platformID);
						}
					}
				}

				// lock the form and submit the UPC to the UPC check iframe
				function check_upc(upc,nu,platformID)
				{
					var idx = array_search(upc,upcs);

					if (idx == -1)
					{
						// the item hasn't already been added to this order - check the UPC
						lock_form();
						checkupcifrm.location = '/admin/utilities/receive_order_checkupc.php?upc=' + upc + '&newused=' + nu + '&platformID=' + platformID;
					}
					else
					{
						// the item has already been added to this order - skip the UPC check
						add_item(itemIDs[idx],nu,platformID);
					}
				}

				// open the item search window
				function check_title(title,nu,platformID)
				{
					lock_form();
					var frm = document.getElementById('recfrm');
					frm.action = '/admin/utilities/receive_order.php';
					frm.act.value = 'search';
					frm.upctitle.value = title;
					frm.newused.value = nu;
					frm.platformID.value = platformID;
					frm.submit();
				}

				// add the item
				function add_item(itemID,nu)
				{
					lock_form();
					var frm = document.getElementById('recfrm');
					frm.act.value = 'select';
					frm.itemID.value = itemID;
					frm.newused.value = nu;
					frm.submit();
				}

				// remove the item
				function remove_item(itemID)
				{
					if (confirm('Are you sure you want to remove this item?\n\nNote: this will completely remove the item from the order,\nincluding any new/used quanitites you may have scanned!'))
					{
						lock_form();
						var frm = document.getElementById('recfrm');
						frm.act.value = 'remove';
						frm.itemID.value = itemID;
						frm.submit();
					}
				}

				// cancel the order
				function cancel_order()
				{
					if (confirm('Are you sure you want to cancel this order?'))
					{
						var frm = document.getElementById('recfrm');
						frm.action = '/admin/utilities/receive_order.php';
						frm.act.value = 'delete';
						frm.submit();
					}
				}

				// put 0s in any empty fields
				function verify(frm)
				{
					var price_blank = false;
					var total = 0;
					for (var i=0; i<frm.elements.length; i++)
					{
						var obj = frm.elements[i];
						if (obj.type == 'text')
						{
							if (obj.name.substring(0,8) == 'quantity')
							{
								obj.value = parseInt(obj.value);
								if (!obj.value || isNaN(obj.value)) { obj.value = 0; }
								total += parseInt(obj.value);
							}
							else if (obj.name.substring(0,5) == 'price' && (isNaN(obj.value) || !obj.value.length)) { price_blank = true; }
						}
					}

					if (price_blank) { alert('Please enter a price for every item.'); return false; }
					else if ((total && confirm('Are you sure you want to complete this order?')) || (!total && confirm('The total quantity is 0; are you sure you want to complete this order?'))) { return true; }
					else { return false; }
				}

				function update_upcprcqty()
				{
					var frm = document.getElementById('compfrm');
					frm.act.value = 'update';
					frm.submit();
				}
			</script>

			<!-- form for adding/removing/searching items -->
			<form method="post" action="/admin/utilities/receive_orderUpdate.php" id="recfrm">
				<input type="hidden" name="act" value="" />
				<input type="hidden" name="itemID" value="" />
				<input type="hidden" name="upctitle" value="" />
				<input type="hidden" name="newused" value="" />
				<input type="hidden" name="platformID" value="" />
			</form>

			<?php echo $this->pg->outlineTableHead(175);?>
			<tr><td align="center" bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Add Item to Order</b></td></tr>
			<tr>
				<td align="center" bgcolor="<?php echo $this->pg->color('table-cell');?>">
					<form name="upc" onsubmit="do_item(this);return false">
						<input type="radio" name="newused" id="n" class="nb"<?php echo $c1;?>> <label for="n">New</label> <input type="radio" name="newused" id="u" class="nb"<?php echo $c2;?>> <label for="u">Used</label><br />
						UPC/Title: <input type="text" name="upctitle" size="25"><br />
						Platform:
						<select name="platformID" id="rec_platformID" size="1">
							<option value=""></option>
							<?php
							$pla = new platforms($this->pg,0);
							$pla->set_item('platforms');
							while (list($a,$arr) = each($pla->values))
							{
								$sel = ($arr[0]==@$_SESSION['receive_last_platformID'] ? ' selected="selected"' : '');
								?>
								<option value="<?php echo $arr[0];?>"<?php echo $sel;?>><?php echo $arr[1];?></option>
								<?php
							}
							?>
						</select><br />
						<input type="submit" value="Add Item to Order &gt;" class="btn mtop" />
					</form>
				</td>
			</tr>
			<?php echo $this->pg->outlineTableFoot();?>

			<iframe name="checkupcifrm" src="/admin/utilities/receive_order_checkupc.php" width="1" height="1" frameborder="0" marginwidth="0" marginheight="0">
				Your browser does not support iframes. Please upgrade.
			</iframe>

			<p />

			<?php echo $this->pg->outlineTableHead();?>
			<form method="post" action="/admin/utilities/receive_orderUpdate.php" name="compfrm" id="compfrm" onsubmit="return verify(this)">
			<input type="hidden" name="act" value="complete">
			<tr bgcolor="<?php echo $this->pg->color('table-head');?>">
				<td align="center"><b>Platform</b></td>
				<td align="center"><b>Title</b></td>
				<td align="center"><b>UPC</b></td>
				<td align="center"><b># New</b></td>
				<td align="center"><b>Price</b></td>
				<td align="center"><b>Total</b></td>
				<td align="center"><b># Used</b></td>
				<td align="center"><b>Price</b></td>
				<td align="center"><b>Total</b></td>
				<td align="center"><b>Remove</b></td>
			</tr>
			<?php
			$total_qty = array(ITEM_NEW=>0,ITEM_USED=>0);
			$total_price_each = array(ITEM_NEW=>0,ITEM_USED=>0);
			$total_price_total = array(ITEM_NEW=>0,ITEM_USED=>0);

			while (list($a,$arr) = each($this->items))
			{
				$bg = (($a%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

				$total_qty[ITEM_NEW]          += $arr['roi_qtynew'];
				$total_qty[ITEM_USED]         += $arr['roi_qtyused'];
				$total_price_each[ITEM_NEW]   += ($arr['roi_qtynew'] ? $arr['roi_price_new'] : 0);
				$total_price_each[ITEM_USED]  += ($arr['roi_qtyused'] ? $arr['roi_price_used'] : 0);
				$total_price_total[ITEM_NEW]  += ($arr['roi_qtynew'] * $arr['roi_price_new']);
				$total_price_total[ITEM_USED] += ($arr['roi_qtyused'] * $arr['roi_price_used']);

				?>
				<script type="text/javascript">
					upcs[<?php echo $a;?>] = '<?php echo $arr['roi_upc'];?>';
					itemIDs[<?php echo $a;?>] = <?php echo $arr['roi_itemID'];?>;
				</script>
				<tr bgcolor="<?php echo $bg;?>">
					<td><?php echo $arr['roi_platform'];?></td>
					<td><?php echo $arr['roi_title'];?></td>
					<td><input type="text" name="upc[<?php echo $arr['roi_itemID'];?>]" size="25" value="<?php echo $arr['roi_upc'];?>" onkeypress="return onlynumbers(this.value,event,true)" /></td>
					<td align="center"><input type="text" name="quantity[<?php echo $arr['roi_itemID'];?>][<?php echo ITEM_NEW;?>]" size="3" value="<?php echo $arr['roi_qtynew'];?>" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right" /></td>
					<td align="center"><input type="text" name="price[<?php echo $arr['roi_itemID'];?>][<?php echo ITEM_NEW;?>]" size="6" value="<?php echo $arr['roi_price_new'];?>" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right" /></td>
					<td align="right"><?php echo number_format(($arr['roi_qtynew']*$arr['roi_price_new']),2);?></td>
					<td align="center"><input type="text" name="quantity[<?php echo $arr['roi_itemID'];?>][<?php echo ITEM_USED;?>]" size="3" value="<?php echo $arr['roi_qtyused'];?>" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right" /></td>
					<td align="center"><input type="text" name="price[<?php echo $arr['roi_itemID'];?>][<?php echo ITEM_USED;?>]" size="6" value="<?php echo $arr['roi_price_used'];?>" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right" /></td>
					<td align="right"><?php echo number_format(($arr['roi_qtyused']*$arr['roi_price_used']),2);?></td>
					<td><a href="javascript:remove_item(<?php echo $arr['roi_itemID'];?>)">Remove Item &gt;</a></td>
				</tr>
				<?php
			}

			if (!count($this->items))
			{
				?>
				<tr>
					<td colspan="10" align="center" bgcolor="<?php echo $this->pg->color('table-cell');?>">--- No Items ---</td>
				</tr>
				<?php
			}
			else
			{
				?>
				<tr bgcolor="<?php echo $this->pg->color('table-head');?>">
					<td colspan="3" align="right"><b>Totals:</b></td>
					<td align="right"><?php echo $total_qty[ITEM_NEW];?></td>
					<td align="right">$<?php echo number_format($total_price_each[ITEM_NEW],2);?></td>
					<td align="right">$<?php echo number_format($total_price_total[ITEM_NEW],2);?></td>
					<td align="right"><?php echo $total_qty[ITEM_USED];?></td>
					<td align="right">$<?php echo number_format($total_price_each[ITEM_USED],2);?></td>
					<td align="right">$<?php echo number_format($total_price_total[ITEM_USED],2);?></td>
					<td>&nbsp;</td>
				</tr>
				<?php
			}
			?>
			<?php echo $this->pg->outlineTableFoot();?>

			<p />

			<b>Overall Total Quantity:</b> <?php echo array_sum($total_qty);?> /
			<b>Overall Total Price:</b> $<?php echo number_format(array_sum($total_price_total),2);?>

			<p />

			<font size="1">
				<b>Note:</b> Changed UPCs/prices will be updated in the database, which will affect invoices, etc.<br />
				They will be changed when you complete the order. <b>Please be careful!</b><br />
				The <b>Totals</b> only include items with a quantity of 1 or more.
			</font>
			<p />
			<input type="button" value="Update UPCs/Prices/Quantities &gt;" onclick="update_upcprcqty()" class="btn" />
			<p />
			<input type="submit" value="Complete Order &gt;" class="btn" />
			<input type="button" value="Cancel Order &gt;" onclick="cancel_order()" class="btn" />
			</form>
			<?php

			$this->pg->addOnload('document.upc.upctitle.focus()');

			return YES;
		}
		else { return NO; }
	}

	/**
	* Add an item to the current order
	* @param	integer	$itemID
	* @param	integer	$newused
	* @param	integer	$platformID	[optional, default 0]
	*/
	function add($itemID,$newused,$platformID=0)
	{
		global $db;

		if ($itemID && in_array($newused,array(ITEM_NEW,ITEM_USED)))
		{
			$_SESSION['receive_last_newused'] = $newused;
			$_SESSION['receive_last_platformID'] = $platformID;

			// first, see if it exists in the current order
			$sql = "SELECT * FROM received_order_items WHERE roi_orderID={$_SESSION['receive_orderID']} AND roi_itemID=$itemID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			if (mysql_num_rows($result))
			{
				// it exists - update the new/used quantity
				$row = mysql_fetch_assoc($result);
				$word = ($newused==ITEM_NEW?'new':'used');
				$field = "roi_qty$word";
				$qty = ($row[$field]+1);

				$platform = $row['roi_platform'];
				$title = $row['roi_title'];

				$sql = "UPDATE received_order_items SET $field=$qty WHERE roi_orderID={$_SESSION['receive_orderID']} AND roi_itemID=$itemID";
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);

				$_SESSION['receive_order_status'] = "<b>".ucwords($word)."</b> quantity increased to <b>$qty</b> for item <b>$platform</b> / <b>$title</b>";
			}
			else
			{
				// it doesn't exist - add the new row
				$sql = "SELECT itm_title,itm_upc,pla_name,prc_new,prc_used FROM items,platforms,prices WHERE itm_itemID=$itemID AND itm_platformID=pla_platformID AND itm_itemID=prc_itemID";
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);

				if (mysql_num_rows($result))
				{
					$row = mysql_fetch_assoc($result);
					$platform = $row['pla_name'];
					$title = $row['itm_title'];
					$upc = $row['itm_upc'];
					$price_new = $row['prc_new'];
					$price_used = $row['prc_used'];

					// add the item to received_order_items
					$sql = "INSERT INTO received_order_items VALUES ({$_SESSION['receive_orderID']},$itemID,'".mysql_escape_string($platform)."','".mysql_escape_string($title)."','$upc',".($newused==ITEM_NEW?1:0).",$price_new,".($newused==ITEM_USED?1:0).",$price_used)";
					mysql_query($sql,$db);
					$this->error->mysql(__FILE__,__LINE__);

					$_SESSION['receive_order_status'] = "<b>".($newused==ITEM_NEW?'New':'Used')."</b> item added to current order: <b>$platform</b> / <b>$title</b>";
				}
				else { $_SESSION['receive_order_error'] = "Unable to retrive platform/title for item <b>$itemID</b> - tell Scott!"; }
			}
		}
	}

	/**
	* Remove an item from the current order
	*/
	function remove()
	{
		global $db;

		if (@$_POST['act'] == "remove" && @$_POST['itemID'])
		{
			// first, see if it exists in the current order
			$sql = "SELECT * FROM received_order_items WHERE roi_orderID={$_SESSION['receive_orderID']} AND roi_itemID={$_POST['itemID']}";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			if (mysql_num_rows($result))
			{
				// it exists - remove it
				$row = mysql_fetch_assoc($result);

				$platform = $row['roi_platform'];
				$title = $row['roi_title'];

				$sql = "DELETE FROM received_order_items WHERE roi_orderID={$_SESSION['receive_orderID']} AND roi_itemID={$_POST['itemID']}";
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);

				$_SESSION['receive_order_status'] = "Removed item <b>$platform</b> / <b>$title</b> from order";
			}
		}
	}

	/**
	* Complete the active order (update the store's quantities and set it as complete)
	*/
	function complete()
	{
		global $db;

		if (@$_SESSION['receive_orderID'])
		{
			$itemIDs = array();
			$newused = array();
			$newqty = 0;
			$usedqty = 0;

			$this->updateValues(YES);

			$sqls = array();
			while (list($itemID,$arr) = each($this->quantity))
			{
				$newqty += $arr[ITEM_NEW];
				$usedqty += $arr[ITEM_USED];

				if ($arr[ITEM_NEW]) { $itemIDs[] = $itemID; $newused[] = ITEM_NEW; }
				if ($arr[ITEM_USED]) { $itemIDs[] = $itemID; $newused[] = ITEM_USED; }

				$sqls[] = "UPDATE quantity SET qty_new=(qty_new+".$arr[ITEM_NEW]."),qty_used=(qty_used+".$arr[ITEM_USED].") WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID=$itemID";
			}
			$sqls[] = "UPDATE received_orders SET ror_completed=" . YES . " WHERE ror_orderID={$_SESSION['receive_orderID']}";

			while (list($a,$sql) = each($sqls))
			{
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}

			$wsh = new wishlist($pg);
			$wsh->check($itemIDs,$newused);

			$_SESSION['receive_orderID'] = 0;
			unset($_SESSION['receive_last_newused']);
			unset($_SESSION['receive_last_platformID']);
			$_SESSION['receive_order_status'] = "Order completed; $newqty new item".($newqty!=1?'s':'')." / $usedqty used item".($usedqty!=1?'s':'')." added to inventory";
		}
		else { $_SESSION['receive_order_error'] = 'There is no open order'; }
	}

	/**
	* View a previous received order
	* @param integer $orderID
	*/
	function view($orderID)
	{
		global $db;

		$sql = "SELECT * FROM received_orders WHERE ror_storeID={$_SESSION['storeID']} AND ror_orderID=$orderID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		if (!mysql_num_rows($result)) { echo "Invalid orderID: $orderID"; }
		else
		{
			$info = mysql_fetch_assoc($result);

			$items = array(); // format: $items[] = array of info
			$sql = "SELECT * FROM received_order_items WHERE roi_orderID=$orderID ORDER BY roi_platform,roi_title";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result)) { $items[] = $row; }

			?>
			<input type="button" value="&lt; Return to Previous Order List" onclick="document.location='/admin/utilities/receive_order.php'" class="btn" />
			<p />
			<b>Distributor:</b> <?php echo $info['ror_distributor'];?>
			<p />
			<font size="1"><b>Note:</b> UPCs/prices shown were at the time the order was generated. They may be different from actual values.</font>
			<p />
			<?php echo $this->pg->outlineTableHead();?>
			<tr>
				<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Platform</b></td>
				<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Title</b></td>
				<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>UPC</b></td>
				<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b># New</b></td>
				<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Price</b></td>
				<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Total</b></td>
				<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b># Used</b></td>
				<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Price</b></td>
				<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Total</b></td>
			</tr>
			<?php
			$total_qty = array(ITEM_NEW=>0,ITEM_USED=>0);
			$total_price_total = array(ITEM_NEW=>0,ITEM_USED=>0);

			while (list($a,$arr) = each($items))
			{
				$bg = (($a%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

				$total_qty[ITEM_NEW] += $arr['roi_qtynew'];
				$total_qty[ITEM_USED] += $arr['roi_qtyused'];
				$total_price_total[ITEM_NEW] += ($arr['roi_qtynew']*$arr['roi_price_new']);
				$total_price_total[ITEM_USED] += ($arr['roi_qtyused']*$arr['roi_price_used']);

				?>
				<tr>
					<td bgcolor="<?php echo $bg;?>"><?php echo $arr['roi_platform'];?></td>
					<td bgcolor="<?php echo $bg;?>"><?php echo $arr['roi_title'];?></td>
					<td bgcolor="<?php echo $bg;?>"><?php echo $arr['roi_upc'];?></td>
					<td bgcolor="<?php echo $bg;?>" align="right"><?php echo $arr['roi_qtynew'];?></td>
					<td bgcolor="<?php echo $bg;?>" align="right"><?php echo number_format($arr['roi_price_new'],2);?></td>
					<td bgcolor="<?php echo $bg;?>" align="right"><?php echo number_format(($arr['roi_qtynew']*$arr['roi_price_new']),2);?></td>
					<td bgcolor="<?php echo $bg;?>" align="right"><?php echo $arr['roi_qtyused'];?></td>
					<td bgcolor="<?php echo $bg;?>" align="right"><?php echo number_format($arr['roi_price_used'],2);?></td>
					<td bgcolor="<?php echo $bg;?>" align="right"><?php echo number_format(($arr['roi_qtyused']*$arr['roi_price_used']),2);?></td>
				</tr>
				<?php
			}
			if (!count($items)) { ?><tr><td colspan="9" align="center" bgcolor="<?php echo $this->pg->color('table-cell');?>">--- No Items ---</td></tr><?php }
			else
			{
				?>
				<tr bgcolor="<?php echo $this->pg->color('table-head');?>">
					<td colspan="3" align="right"><b>Totals:</b></td>
					<td align="right"><b><?php echo $total_qty[ITEM_NEW];?></b></td>
					<td align="right">&nbsp;</td>
					<td align="right"><b>$<?php echo number_format($total_price_total[ITEM_NEW],2);?></b></td>
					<td align="right"><b><?php echo $total_qty[ITEM_USED];?></b></td>
					<td align="right">&nbsp;</td>
					<td align="right"><b>$<?php echo number_format($total_price_total[ITEM_USED],2);?></b></td>
				</tr>
				<?php
			}

			?>
			<?php echo $this->pg->outlineTableFoot();?>
			<p />
			<input type="button" value="&lt; Return to Previous Order List" onclick="document.location='/admin/utilities/receive_order.php'" class="btn" />
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

		$sql = "DELETE FROM received_orders WHERE ror_storeID={$_SESSION['storeID']} AND ror_orderID=$orderID";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		if (mysql_affected_rows())
		{
			$sql = "DELETE FROM received_order_items WHERE roi_orderID=$orderID";
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$this->pg->status('Order cancelled/deleted');
		}
		else { $this->pg->error('Unable to delete order (it may not exist anymore)'); }
	}

	/**
	* Update the UPCs/quantities/prices in received_order_items and the items/prices table if necessary
	* @param integer $itmprc_tables Update the UPCs/prices in the items/prices tables? [optional, default NO]
	*/
	function updateValues($itmprc_tables=NO)
	{
		global $db;

		$upcs = @$_POST['upc'];
		$qtys = @$_POST['quantity'];
		$prices = @$_POST['price'];

		if (@$_SESSION['receive_orderID'] && is_array($upcs) && is_array($qtys) && is_array($prices))
		{
			$sqls = array();
			$itemIDs = array_keys($upcs);

			while (list($a,$itemID) = each($itemIDs))
			{
				$upc = $upcs[$itemID];
				$qtynew = $qtys[$itemID][ITEM_NEW];
				$qtyused = $qtys[$itemID][ITEM_USED];
				$prcnew = $prices[$itemID][ITEM_NEW];
				$prcused = $prices[$itemID][ITEM_USED];

				$sqls[] = "UPDATE received_order_items SET roi_upc='$upc',roi_qtynew=$qtynew,roi_qtyused=$qtyused,roi_price_new=$prcnew,roi_price_used=$prcused WHERE roi_orderID={$_SESSION['receive_orderID']} AND roi_itemID=$itemID";

				if ($itmprc_tables)
				{
					$sqls[] = "UPDATE items SET itm_upc='$upc' WHERE itm_itemID=$itemID";
					$sqls[] = "UPDATE prices SET prc_new=$prcnew,prc_used=$prcused WHERE prc_itemID=$itemID";
				}
			}

			while (list($a,$sql) = each($sqls))
			{
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}
		}
	}
}

?>