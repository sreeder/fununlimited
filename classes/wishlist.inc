<?php
/**
* Wishlist class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class wishlist
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
	* Customers class
	* @var identifier $cust
	*/
	var $cust;

	/**
	* Item search class
	* @var identifier $its
	*/
	var $its;

	/**
	* Current wishlistID
	* @var integer $wishlistID
	*/
	var $wishlistID;

	/**
	* Current wishlist information
	* @var array $info
	*/
	var $info;

	/**
	* Array of wishlists
	* @var array $wishlists
	*/
	var $wishlists;

	/**
	* Customer/item criteria
	* @var array $criteria
	*/
	var $criteria;

	/**
	* Array of email information
	* @var array $email
	*/
	var $email;

	/**
	* itemID to force selection of
	* @var integer $use_itemID
	*/
	var $use_itemID;

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	*/
	function wishlist(&$pg)
	{
		$this->pg =& $pg;
		$this->wishlistsID = 0;
		$this->info = array();
		$this->wishlists = array();
		$this->criteria = array();
		$this->email = array();
		$this->use_itemID = (isset($_GET['use_itemID'])?$_GET['use_itemID']:@$_POST['use_itemID']);

		$this->cust = new customers($this->pg);
		$this->its = new item_search($this->pg);
		$this->its->action = "{$_SESSION['root_admin']}utilities/wishlist.php";
		$this->error = new error('Wishlist class');
	}

	/**
	* Sets the wishlistID and pulls the wishlist information into $this->info
	* @param integer $wishlistID
	*/
	function set_wishlistID($wishlistID)
	{
		global $db;

		$this->wishlistID = $wishlistID;
		$this->info = array();

		$sql = "SELECT * FROM wishlists,wishlist_items,customers WHERE wsh_storeID={$_SESSION['storeID']} AND wsh_wishlistID=wli_wishlistID AND wsh_customerID=cus_customerID AND wsh_wishlistID=$wishlistID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		if (mysql_num_rows($result))
		{
			$row = mysql_fetch_assoc($result);

			$items = array();

			$isql = "SELECT * FROM wishlist_items,items,platforms WHERE wli_wishlistID={$row['wsh_wishlistID']} AND wli_itemID=itm_itemID AND itm_platformID=pla_platformID ORDER BY pla_name,itm_title";
			$iresult = mysql_query($isql,$db);
			while ($irow = mysql_fetch_assoc($iresult)) { $items[$row['wli_itemID']] = $irow; }

			$row['items'] = $items;
			$this->info = $row;
		}
	}

	/**
	* Return the number of wishlist customers for this store
	*/
	function get_total_wishlists()
	{
		global $db;

		$sql = "SELECT COUNT(*) AS count FROM wishlists,wishlist_items,customers WHERE wsh_storeID={$_SESSION['storeID']} AND wsh_wishlistID=wli_wishlistID AND wsh_customerID=cus_customerID GROUP BY wsh_wishlistID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		return mysql_num_rows($result);
	}

	/**
	* Pull in all wishlist customers for this store
	* @param integer $start index of customer to start at [optional, default -1]
	* @param integer $end index of customer to end at [optional, default -1]
	*/
	function get_wishlists($start=-1,$end=-1,$customerID=0)
	{
		global $db;

		$this->wishlists = array();

		set_time_limit(60);
		$sql = "SELECT * FROM wishlists,wishlist_items,customers WHERE wsh_storeID={$_SESSION['storeID']} AND wsh_wishlistID=wli_wishlistID AND wsh_customerID=cus_customerID" . ($customerID ? " AND cus_customerID=$customerID" : '') . " GROUP BY wli_wishlistID ORDER BY cus_lname,cus_fname";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		$idx = -1;
		$get_wishlistIDs = array();
		while ($row = mysql_fetch_assoc($result))
		{
			set_time_limit(30);
			$idx++;
			$items = array();

			// only pull the items if the wishlist is going to be shown
			if (($start == -1 || $end == -1) || ($idx >= $start && $idx <= $end))
			{
				$get_wishlistIDs[] = $row['wsh_wishlistID'];
			}

			$row['items'] = array();
			$this->wishlists[$row['wsh_wishlistID']] = $row;
		}

		if (count($get_wishlistIDs))
		{
			$sql = "SELECT * FROM wishlist_items,items,platforms,quantity WHERE wli_wishlistID IN " . getIn($get_wishlistIDs) . " AND wli_itemID=itm_itemID AND itm_platformID=pla_platformID AND itm_itemID=qty_itemID AND qty_storeID={$_SESSION['storeID']} ORDER BY pla_name,itm_title";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				$this->wishlists[$row['wli_wishlistID']]['items'][$row['wli_itemID']] = $row;
			}
		}
	}

	/**
	* Pulls the information from $_POST
	*/
	function pull_post()
	{
		$this->criteria = (isset($_POST['criteria']) ? $_POST['criteria'] : $_POST);
		while (list($key,$val) = each($this->criteria)) { $this->criteria[$key] = stripslashes($val); }
		reset($this->criteria);

		if (isset($_POST['email']))
		{
			$this->email = $_POST['email'];
			while (list($key,$val) = each($this->email)) { $this->email[$key] = stripslashes($val); }
			reset($this->email);
		}

		if (isset($_POST['wishlistID'])) { $this->set_wishlistID($_POST['wishlistID']); }
	}

	/**
	* Output the add new form and the first page of wishlist items
	* @param integer $page Customer list page number [optional, default 1]
	*/
	function main_page($page=1)
	{
		if (!is_numeric($page)) { $page = 1; }

		$this->add_new_form($this->use_itemID);
		$this->show_list($page);
	}

	/**
	* Output the add new form (customer search)
	*/
	function add_new_form()
	{
		?>
		<script type="text/javascript">
			function show_search()
			{
				var obj = document.getElementById('newsearch');
				obj.style.display = (obj.style.display=='none'?'block':'none');
				if (obj.style.display == 'block')
				{
					var frm = document.getElementById('custsrch');
					for (var i=0; i<frm.elements.length; i++)
					{
						if (frm.elements[i].type == 'text') { frm.elements[i].value = ''; }
					}
					frm.elements['criteria[phone]'].focus();
				}
			}
		</script>

		<input type="button" value="Add New Wishlist Items &gt;" onclick="show_search()" class="btn" />
		<div id="newsearch" style="display:<?php echo ($this->use_itemID?'block':'none');?>">
			&nbsp;<br />
			To add a new wishlist item, search for a customer below:
			<p />
			<?php
			if (isset($_SESSION['customerID']))
			{
				?>
				Click <a href="/admin/utilities/wishlist.php?act=select&customerID=<?php echo $_SESSION['customerID'];?>&use_itemID=<?php echo $this->use_itemID;?>">here</a> to select the current point-of-sale customer (<?php echo "{$_SESSION['cust_info']['fname']} {$_SESSION['cust_info']['lname']}";?>)
				<p />
				<?php
			}
			if (isset($_SESSION['wishlist_customerID']))
			{
				?>
				Click <a href="/admin/utilities/wishlist.php?act=select&customerID=<?php echo $_SESSION['wishlist_customerID'];?>&use_itemID=<?php echo $this->use_itemID;?>">here</a> to select the last wishlist customer (<?php echo $_SESSION['wishlist_customer_name'];?>)
				<p />
				<?php
			}

			$this->cust->search_form("{$_SESSION['root_admin']}utilities/wishlist.php",YES,NO,array('use_itemID'=>$this->use_itemID));
			?>
		</div>
		<p />
		<input type="button" value="View In-Stock Wishlist Items &gt;" onclick="document.location='/admin/utilities/wishlist.php?act=viewinstock'" class="btn" />
		<p />
		<input type="button" value="Limit to Item &gt;" onclick="document.location='/admin/utilities/wishlist.php?act=limit'" class="btn" />
		<p />
		<hr width="100%" size="-1" color="#000000" noshade="noshade" />
		<p />
		<?php
	}

	/**
	* Output the list of wishlist items
	* @param integer $page Customer list page number
	*/
	function show_list($page)
	{
		global $db;

		$per_page = 25;
		$total = $this->get_total_wishlists();
		$pages = ceil($total/$per_page);

		$start = (($page-1)*$per_page);
		$end = ($start+$per_page-1);
		if ($end > ($total-1)) { $end = ($total-1); }

		$this->get_wishlists($start,$end);
		$wishlistIDs = array_keys($this->wishlists);

		// output the result navigation bar
		?>
		<script type="text/javascript">
			var page = <?php echo $page;?>;
			var pages = <?php echo $pages;?>;
			function loadpage(num) { if (num != page) { var frm = document.lp; frm.page.value = num; frm.submit(); } }
			function goprev() { if ((page-1)) { loadpage((page-1)); } }
			function gonext() { if ((page+1) <= <?php echo $pages;?>) { loadpage((page+1)); } }
		</script>

		<table border="0" cellspacing="0" cellpadding="2">
			<tr>
				<td align="right">
					<a href="javascript:loadpage(1)" title="First Page"><img src="/images/search_first.gif" width="60" height="18" border="0" /></a>
					<a href="javascript:goprev()" title="Previous Page"><img src="/images/search_prev.gif" width="48" height="18" border="0" /></a>
				</td>
				<td align="center" valign="middle" style="font-size:12">
					<?php

					$padby = 3;

					$ps = (($page-$padby)<1?1:($page-$padby));
					$pe = (($page+$padby)>$pages?$pages:($page+$padby));

					if ($pe > $pages) { $pe = $pages; }

					if ($ps > 1)
					{
						?><a href="javascript:loadpage(1)" style="text-decoration:underline" title="Wishlists 1-<?php echo $per_page;?>">1</a> <?php
						if (($page-$padby) > 2) { echo "... "; }
					}

					$links = array();
					for ($j=$ps; $j<=$pe; $j++)
					{
						$resnum = ((($j-1)*$per_page)+1).'-'.(($j*$per_page)>$total?$total:($j*$per_page));

						$str = '';
						if ($j != $page) { $str .= '<a href="javascript:loadpage('.$j.')" style="text-decoration:underline" title="Wishlists '.$resnum.'">'; } else { $str .= "<b>"; }
						$str .= $j;
						if ($j != $page) { $str .= '</a>'; } else { $str .= "</b>"; }
						$links[] = $str;
					}
					echo implode(" ",$links);

					if ($pe != $pages)
					{
						$lastresnum = ((($pages-1)*$per_page)+1).'-'.$total;
						if (($page+$padby) < ($pages-1)) { echo " ..."; }
						?> <a href="javascript:loadpage(<?php echo $pages;?>)" style="text-decoration:underline" title="Wishlists <?php echo $lastresnum;?>"><?php echo $pages;?></a><?php
					}

					?>
				</td>
				<td align="left">
					<a href="javascript:gonext()" title="Next Page"><img src="/images/search_next.gif" width="48" height="18" border="0" /></a>
					<a href="javascript:loadpage(<?php echo $pages;?>)" title="Last Page"><img src="/images/search_last.gif" width="60" height="18" border="0" /></a>
				</td>
				<td><img src="/images/blank.gif" width="10" height="1" /></td>
				<td>
					Jump to Page: <select name="page" size="1" onchange="loadpage(this.value)"<?php echo ($pages==1?' disabled="disabled"':'');?>>
						<?php
						for ($i=1; $i<=$pages; $i++)
						{
							$snum = (($i-1)*$per_page);
							$enum = (($i*$per_page)>$total?$total:($i*$per_page))-1;
							$slet = ucwords(strtolower(substr($this->wishlists[$wishlistIDs[$snum]]['cus_lname'],0,2)));
							$elet = ucwords(strtolower(substr($this->wishlists[$wishlistIDs[$enum]]['cus_lname'],0,2)));
							if (is_numeric(substr($slet,0,1))) { $slet = '#'; }
							if (is_numeric(substr($elet,0,1))) { $elet = '#'; }
							if ($slet != $elet) { $lets = "(Last Name: $slet-$elet)"; }
							else { $lets = "(Last Name: $slet)"; }

							if ($i == $page) { $s = ' selected="selected"'; } else { $s = ''; }
							?><option value="<?php echo $i;?>"<?php echo $s;?>><?php echo $i;?> <?php echo $lets;?></option><?php
						}
						?>
					</select>
				</td>
			</tr>
		</table>
		<form method="get" action="/admin/utilities/wishlist.php" name="lp">
		<input type="hidden" name="page" value="">
		</form>
		<p />

		<?php
		$wishlists = array();
		$wishlistIDs = array_keys($this->wishlists);
		for ($i=$start; $i<=$end; $i++) { $wishlists[] = $this->wishlists[$wishlistIDs[$i]]; }

		$this->print_table($wishlists,'remove',$page);
	}

	/**
	* Output the wishlist/item list
	* @param array $wishlists
	* @param string $function
	* @param integer $page
	*/
	function print_table($wishlists,$function,$page)
	{
		global $act;

		$colspans = array(
			'remove'  => 3,
			'email'   => 3,
			'contact' => 2
		);
		$titles = array(
			'remove' => 'Remove',
			'email'  => 'Email'
		);

		?>
		<script type="text/javascript">
			function send_email(wishlistID)
			{
				var frm = document.getElementById('emlfrm');
				frm.wishlistID.value = wishlistID;
				frm.submit();
			}
			function ncverify(frm)
			{
				// change any blank/negative values to 0
				for (var i=0; i<frm.elements.length; i++)
				{
					if (frm.elements[i].type == 'text')
					{
						var val = parseInt(frm.elements[i].value)+'';
						if (!val.length || (val*1) < 0) { val = 0; }
						frm.elements[i].value = val;
					}
				}
				return true;
			}

			/**
			* Confirm and remove all items from a customer
			* @param	integer	wishlistID
			*/
			function removeAll(wishlistID)
			{
				if (confirm('Are you sure you would like to remove ALL items for this customer?\nThis is NOT undoable!'))
				{
					go('/admin/utilities/wishlist.php?act=delete_all&wishlistID=' + wishlistID);
				}
			}

			function remove_item(wishlistID,itemID)
			{
				if (confirm('Are you sure you want to delete the selected wishlist item?'))
				{
					var frm = document.wishlists;
					frm.wishlistID.value = wishlistID;
					frm.itemID.value = itemID;
					frm.submit();
				}
			}
		</script>

		<form method="post" action="/admin/utilities/wishlist.php" name="emlfrm" id="emlfrm">
			<input type="hidden" name="act" value="email">
			<input type="hidden" name="wishlistID" value="">
		</form>

		<form method="post" action="/admin/utilities/wishlist.php" name="wishlists">
		<input type="hidden" name="act" value="delete">
		<input type="hidden" name="wishlistID" value="">
		<input type="hidden" name="itemID" value="">
		</form>

		<form method="post" action="/admin/utilities/wishlist.php" onsubmit="return ncverify(this)">
		<input type="hidden" name="act" value="<?php echo $act;?>" />
		<input type="hidden" name="page" value="<?php echo $page;?>" />
		<?php
		$this->pg->outlineTableHead('95%');
		?>
		<tr>
			<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Customer</b></td>
			<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Items</b></td>
			<?php if ($function!='contact') { ?><td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Functions</b></td><?php } ?>
		</tr>
		<?php
		while (list($a,$arr) = each($wishlists))
		{
			$bg = (($a%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

			?>
			<tr>
				<td valign="top" bgcolor="<?php echo $bg;?>">
					<b><?php echo "{$arr['cus_lname']}, {$arr['cus_fname']}";?></b><br />
					Home: <?php echo $this->pg->format('phone',$arr['cus_phone']);?><br />
					Cell: <?php echo $this->pg->format('phone',$arr['cus_cellphone']);?><br />
					<?php echo (strlen($arr['cus_email'])?'<a href="mailto:'.$arr['cus_email'].'">':'');?><?php echo $arr['cus_email'];?><?php echo (strlen($arr['cus_email'])?'</a>':'');?>
				</td>
				<td valign="top" bgcolor="#F0F0F0" style="padding:0px">
					<table border="0" cellspacing="1" cellpadding="2" width="100%">
						<tr bgcolor="#DEDEDE">
							<td width="20%" align="center"><b>Platform</b></td>
							<td width="100%" align="center"><b>Title</b></td>
							<td align="center"><b>Added</b></td>
							<td align="center"><b>New/Used</b></td>
							<td align="center"><b>#Calls</b></td>
							<?php
							if ($function == 'email') { ?><td width="50" align="center"><b>EmailSent</b></td><?php }

							if ($function != 'contact')
							{
								?><td width="75" align="center"><b><?php echo @$titles[$function];?></b></td><?php
							}
							?>
						</tr>
						<?php
						while (list($a,$iarr) = each($arr['items']))
						{
							?>
							<tr bgcolor="#E5E5E5">
								<td><?php echo $iarr['pla_name'];?></td>
								<td style="white-space:normal"><?php echo $iarr['itm_title'];?></td>
								<td><?php echo date('m/d/Y h:ia',$iarr['wli_timeadded']);?></td>
								<td><?php echo ($iarr['wli_newused']==ITEM_NEW?'New':($iarr['wli_newused']==ITEM_USED?'Used':'Either'));?></td>
								<td align="center">
									<?php
									if ($function == 'contact') { echo $iarr['wli_num_calls']; }
									else
									{
										?>
										<input type="hidden" name="orignumcalls[<?php echo $arr['wsh_wishlistID'];?>][<?php echo $iarr['wli_itemID'];?>]" value="<?php echo $iarr['wli_num_calls'];?>" />
										<input type="text" name="numcalls[<?php echo $arr['wsh_wishlistID'];?>][<?php echo $iarr['wli_itemID'];?>]" size="2" value="<?php echo $iarr['wli_num_calls'];?>" onkeypress="return onlynumbers(this.value,event,true,true)" />
										<?php
									}
									?>
								</td>
								<?php
								if ($function == 'email') { ?><td><?php echo ($arr['wli_emailsent']==YES?'YES':'No');?></td><?php }

								if ($function != 'contact')
								{
									?><td align="left"><?php
										if ($function == 'remove')
										{
											?><a href="javascript:remove_item(<?php echo $arr['wsh_wishlistID'];?>,<?php echo $iarr['itm_itemID'];?>)">Remove Item &gt;</a><?php
										}
										elseif ($function == 'email')
										{
											if (strlen(@$arr['cus_email'])) { ?><a href="javascript:send_email(<?php echo $arr['wsh_wishlistID'];?>)">Send Email &gt;</a><?php }
											else { ?>Call customer<?php }
										}
									?></td>
									<?php
								}
								?>
							</tr>
							<?php
						}
						?>
					</table>
				</td>
				<?php
				if ($function != 'contact')
				{
					?>
					<td valign="top" bgcolor="<?php echo $bg;?>">
						<a href="/admin/utilities/wishlist.php?act=select&customerID=<?php echo $arr['wsh_customerID'];?>&use_itemID=<?php echo $this->use_itemID;?>">Add Another Item &gt;</a><br />
						<a href="javascript:void(0)" onclick="removeAll(<?php echo $arr['wsh_wishlistID'];?>);return false">Remove All Items &gt;</a>
					</td>
					<?php
				}
				?>
			</tr>
			</label>
			<?php
		}
		if (!count($wishlists)) { ?><tr><td colspan="<?php echo $colspans[$function];?>" align="center" bgcolor="<?php echo $this->pg->color('table-cell');?>">No wishlists found</td></tr><?php }

		$this->pg->outlineTableFoot();
		?>
		<p />
		<?php if ($function != 'contact') { ?><input type="submit" value="Update # Calls &gt;" class="btn" /><?php } ?>
		</form>
		<?php
		if ($function != 'contact')
		{
			?>
			<p />
			<form method="post" action="/admin/utilities/wishlist.php" onsubmit="if (this.numcalledmax.value > 0) { return true; } else { alert('Please enter a number greater than 0'); return false; }">
			<input type="hidden" name="act" value="<?php echo $act;?>" />
			<input type="hidden" name="page" value="<?php echo $page;?>" />
			<input type="hidden" name="remove_num_called" value="<?php echo YES;?>">
			Remove all wishlist items that have been called on
			<input type="text" name="numcalledmax" size="2" onkeypress="return onlynumbers(this.value,event,true,true)" />
			or more time(s)
			<input type="submit" value="Remove Items &gt;" class="btn" />
			</form>
			<br />
			<font size="1"><b>Note:</b> this will remove <b>ANY</b> items that have been called on, not just in-stock items!</font>
			<?php
		}
	}

	/**
	* Update the call numbers
	* @param array $callnums '# Calls' text field value (POSTed data from the form)
	* @param array $origcallnums Original # calls hidden field value (POSTed data from the form)
	*/
	function update_num_calls($callnums,$origcallnums)
	{
		global $db;

		$sqls = array();
		while (list($wishlistID,$items) = each($callnums))
		{
			while (list($itemID,$num) = each($items))
			{
				if ($num != $origcallnums[$wishlistID][$itemID])
				{
					$sqls[] = "UPDATE wishlist_items SET wli_num_calls=$num WHERE wli_wishlistID=$wishlistID AND wli_itemID=$itemID";
				}
			}
		}

		if (count($sqls))
		{
			while (list($a,$sql) = each($sqls))
			{
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}
		}

		$this->pg->status("Changed the number of calls for ".count($sqls)." wishlist item".(count($sqls)!=1?'s':''));
	}

	/**
	* Remove wishlist items that have been called $numcalledmax time(s) or more
	* @param array $numcalledmax
	*/
	function remove_num_called($numcalledmax)
	{
		global $db;

		$wishlistIDs = array();
		$sql = "SELECT wsh_wishlistID FROM wishlists WHERE wsh_storeID={$_SESSION['storeID']}";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result)) { $wishlistIDs[] = $row['wsh_wishlistID']; }

		if (count($wishlistIDs))
		{
			$sql = "DELETE FROM wishlist_items WHERE wli_wishlistID IN (".implode(',',$wishlistIDs).") AND wli_num_calls>=$numcalledmax";
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			$removed = mysql_affected_rows($db);
		}
		else { $removed = 0; }

		$this->pg->status("Removed wishlist items that were called $numcalledmax or more time".($numcalledmax!=1?'s':'')." ($removed removed)");
	}

	/**
	* Perform the customer search
	*/
	function customer_search()
	{
		$this->cust->criteria = $this->criteria;
		$this->cust->search();
	}

	/**
	* Output the customer search results
	*/
	function customer_results()
	{
		$this->cust->search_results('wishlist.php','wishlist',urlencode("{$_SESSION['root_admin']}utilities/wishlist.php?customerID=%customerID%"),array('use_itemID'=>$this->use_itemID));
	}

	/**
	* Set the customerID for the current wishlist and look up their info
	* @param integer $customerID
	*/
	function set_customerID($customerID)
	{
		$this->cust->set_customerID($customerID,NO);

		$this->create_wishlist($_SESSION['storeID'],$customerID);
		$_SESSION['wishlist_customerID'] = $customerID;
		$_SESSION['wishlist_customer_name'] = "{$this->cust->info['fname']} {$this->cust->info['lname']}";
	}

	/**
	* Create a wishlist and return the wishlistID or return the existing wishlistID for the current customer
	* @param integer $storeID
	* @param integer $customerID
	*/
	function create_wishlist($storeID,$customerID)
	{
		global $db;

		$sql = "SELECT wsh_wishlistID FROM wishlists WHERE wsh_storeID={$_SESSION['storeID']} AND wsh_customerID=$customerID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		if (mysql_num_rows($result))
		{
			// a wishlistID already exists
			$row = mysql_fetch_assoc($result);
			$wishlistID = $row['wsh_wishlistID'];
		}
		else
		{
			// create a wishlist
			$sql = "INSERT INTO wishlists VALUES (NULL,$storeID,$customerID)";
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$wishlistID = mysql_insert_id();
		}

		$_SESSION['wishlist_wishlistID'] = $wishlistID;

		return $wishlistID;
	}

	/**
	* Output the customer's name
	*/
	function show_customer()
	{
		?><b>Customer:</b> <?php echo @$_SESSION['wishlist_customer_name'];?> (<a href="/admin/utilities/wishlist.php?use_itemID=<?php echo $this->use_itemID;?>">Change</a>)<p /><?php
	}

	/**
	* Show the item search form
	* @param	boolean	$limit	show the "limit item" message? [optional, default false]
	*/
	function item_search_form($limit=false)
	{
		if ($this->use_itemID)
		{
			$itm = new items($this->pg);
			$itm->set_itemID($this->use_itemID);
			$this->its->criteria = array(
				'upc'        => $itm->info['upc'],
				'title'      => $itm->info['title'],
				'platformID' => $itm->info['platformID']
			);
		}

		if ($limit)
		{
			echo 'Search below for the items that you would like to limit the list to:';
			$this->its->act = 'limit_search';
		}
		else
		{
			echo "Search below for the items that you would like the add to the customer's wishlist:";
		}
		?>
		<p />
		<?php
		$this->its->action = "{$_SESSION['root_admin']}utilities/wishlist.php";
		$this->its->form_head(
			array(
				'use_itemID' => $this->use_itemID
			)
		);
		$this->its->newused(ITEM_USED);
		$this->its->upc();
		$this->its->title();
		$this->its->platform();
		$this->its->form_foot('upc');
	}

	/**
	* Perform the item search
	*/
	function item_search()
	{
		$this->its->criteria = $this->criteria;
		$this->its->search();
	}

	/**
	* Output the item search results
	*/
	function item_results()
	{
		if (!count($this->its->results))
		{
			echo "Your search did not match any items. Please try again.<p />";
			$this->item_search_form();
		}
		else
		{
			$only1 = ''; $only2 = '';
			$count = count($this->its->results);
			if ($count > 100)
			{
				$only1 = ' Only the first 100 are shown.';
				$only2 = 'Please narrow your search criteria. ';

				$this->its->results = array_slice($this->its->results,0,100);
			}

			$again_act = ($this->its->criteria['act']=='limit_search' ? 'limit' : 'search');

			?>
			<?php echo $count;?> item<?php echo ($count==1?'':'s');?> matched your criteria.<?php echo $only1;?>
			<p />
			<?php echo $only2;?>Click <a href="/admin/utilities/wishlist.php?act=<?php echo $again_act;?>&customerID=<?php echo @$_SESSION['wishlist_customerID'];?>&use_itemID=<?php echo $this->use_itemID;?>">here</a> to search again.
			<?php
			if ($this->its->criteria['act'] == 'limit_search')
			{
				$this->its->results_act = 'limit_select';
			}
			$this->its->showSmallResults(true);
		}
	}

	/**
	* Add an item to the current customer's wishlist
	* @param	integer	$itemID
	* @return	string	status text
	* @access	public
	*/
	function add($itemID)
	{
		global $db;

		if (isset($_SESSION['wishlist_wishlistID']))
		{
			$sql = "INSERT IGNORE INTO wishlist_items VALUES ({$_SESSION['wishlist_wishlistID']},$itemID,{$_SESSION['search_criteria']['newused']},".NO.",'".time()."',0)";
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$sql = "SELECT itm_title,pla_name FROM items,platforms WHERE itm_itemID=$itemID AND itm_platformID=pla_platformID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			if (mysql_num_rows($result))
			{
				$row = mysql_fetch_assoc($result);
				$title = $row['itm_title'];
				$platform = $row['pla_name'];
			}
			else
			{
				$title = 'Unknown Title';
				$platform = 'Unknown Platform';
			}

			$nu = ($_SESSION['search_criteria']['newused']==ITEM_NEW?'New':($_SESSION['search_criteria']['newused']==ITEM_USED?'Used':'Either'));
			return "Added item <b>$title/$platform/$nu</b> to the wishlist for <b>{$_SESSION['wishlist_customer_name']}</b>";
		}
		else
		{
			$this->pg->error('Wishlist variable not set! Please do not reload the page!');
		}
	}

	/**
	* Delete a wishlist item
	* @param integer $wishlistID
	*/
	function delete($wishlistID,$itemID)
	{
		global $db;

		$sql = "DELETE FROM wishlist_items WHERE wli_wishlistID=$wishlistID AND wli_itemID=$itemID";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		$this->pg->status('Wishlist item deleted');
	}

	/**
	* Delete ALL wishlist items from a customer's wishlist
	* @param	integer	$wishlistID
	*/
	function deleteAllItems($wishlistID)
	{
		global $db;

		$sql = "DELETE FROM wishlist_items WHERE wli_wishlistID=$wishlistID";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		$this->pg->status('All wishlist items deleted for the selected customer');
	}

	/**
	* Unset all of the session variables
	*/
	function reset_vars()
	{
		unset($_SESSION['wishlist_wishlistID']);
	}

	/**
	* Removes wishlist items from the array that don't have any in-stock items or don't match the itemIDs in $onlyitemIDs
	* @param array $wishlists
	* @param array $onlyitemIDs [optional, default array()]
	* @return array of $wishlists that have in-stock items/items in the $onlyitemIDs array
	*/
	function wishlist_filter($wishlists,$onlyitemIDs=array())
	{
		global $db;

		// build a list of itemIDs
		$itemIDs = array();
		while (list($a,$arr) = each($wishlists))
		{
			while (list($b,$iarr) = each($arr['items']))
			{
				if (!in_array($iarr['itm_itemID'],$itemIDs)) { $itemIDs[] = $iarr['itm_itemID']; }
			}
		}
		reset($wishlists);

		if (count($itemIDs))
		{
			if (!count($onlyitemIDs))
			{
				$qtys = array(); // format: $qtys[itemID] = quantity_array
				$sql = "SELECT * FROM quantity WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID IN (".implode(',',$itemIDs).")";
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);

				while ($row = mysql_fetch_assoc($result)) { $qtys[$row['qty_itemID']] = $row; }
			}

			// parse wishlists and remove any that don't have in-stock quantities/aren't in the $onlyitemIDs array
			$new_wishlists = array();
			while (list($a,$arr) = each($wishlists))
			{
				$new_items = array();
				while (list($b,$iarr) = each($arr['items']))
				{
					if (!count($onlyitemIDs))
					{
						//echo "{$iarr['itm_itemID']}/{$iarr['itm_title']}: ".($iarr['wli_newused']==ITEM_NEW?'new':'used').' (new:'.@$qtys[$iarr['itm_itemID']]['qty_new'].' / used:'.@$qtys[$iarr['itm_itemID']]['qty_used'].')<br />';
						if ($iarr['wli_newused'] == ITEM_NEW && @$qtys[$iarr['itm_itemID']]['qty_new']) { $new_items[] = $iarr; }
						elseif ($iarr['wli_newused'] == ITEM_USED && @$qtys[$iarr['itm_itemID']]['qty_used']) { $new_items[] = $iarr; }
						elseif ($iarr['wli_newused'] == EITHER && (@$qtys[$iarr['itm_itemID']]['qty_new'] || @$qtys[$iarr['itm_itemID']]['qty_used'])) { $new_items[] = $iarr; }
					}
					else
					{
						if (in_array($iarr['wli_itemID'],$onlyitemIDs)) { $new_items[$iarr['wli_itemID']] = $iarr; }
					}
				}
				$wishlists[$a]['items'] = $new_items;

				if (count($new_items)) { $new_wishlists[] = $wishlists[$a]; }
			}

			// build updated list of itemIDs
			$itemIDs = array();
			while (list($a,$arr) = each($new_wishlists))
			{
				while (list($b,$iarr) = each($arr['items']))
				{
					if (!in_array($iarr['itm_itemID'],$itemIDs)) { $itemIDs[] = $iarr['itm_itemID']; }
				}
			}
			reset($new_wishlists);
			$_SESSION['wishlist_itemIDs'] = $itemIDs;

			return $new_wishlists;
		}
		else { return array(); }
	}

	/**
	* Output a list of in-stock wishlist items
	*/
	function show_in_stock()
	{
		global $db;

		$this->get_wishlists();
		$this->wishlists = $this->wishlist_filter($this->wishlists);

		?>
		The following wishlists contain items that are in stock.<br />
		Click <b>Send Email</b> to send the in-stock notice to the customer.
		<p />
		<font size="1"><b>Note:</b> The email will contain a list of all in-stock items for that customer; not only the one you click on!</font>
		<p />
		<input type="button" value="&lt; Return to Current Wishlists" onclick="document.location='/admin/utilities/wishlist.php'" class="btn" />
		<p />
		<?php
		$this->print_table($this->wishlists,'email',1);
	}

	/**
	* Output a list of wishlists that contain the given items
	* @param	array	$itemIDs
	*/
	function showWithItem($itemIDs)
	{
		global $db;

		$this->get_wishlists();
		$this->wishlists = $this->wishlist_filter($this->wishlists, $itemIDs);

		?>
		The following wishlists contain the selected item<?php echo getS(count($itemIDs));?>.
		<p />
		<input type="button" value="&lt; Return to Current Wishlists" onclick="document.location='/admin/utilities/wishlist.php'" class="btn" />
		<p />
		<?php
		$this->print_table($this->wishlists, 'remove', 1);
	}

	/**
	* Output the email body contents for the user to edit
	*/
	function email_form()
	{
		// parse out the items that don't aren't in stock
		list($this->info) = $this->wishlist_filter(array($this->info));

		if (count($this->info['items']))
		{
			$body = $this->email_body($this->info['items']);

			?>
			If you would like, you can change the body of the email below before it is sent.<br />
			Click <b>Send Email</b> when you are done.
			<p />
			<input type="button" value="&lt; Cancel / Return to In Stock List" onclick="document.location='/admin/utilities/wishlist.php?act=viewinstock'" class="btn" />
			<p />
			<?php echo $this->pg->outlineTableHead();?>
			<form method="post" action="/admin/utilities/wishlist.php">
			<input type="hidden" name="act" value="sendemail">
			<input type="hidden" name="wishlistID" value="<?php echo $this->wishlistID;?>">
			<tr>
				<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Name::</b></td>
				<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><?php echo "{$this->info['cus_fname']} {$this->info['cus_lname']}";?></td>
			</tr>
			<tr>
				<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Email:</b></td>
				<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><?php echo $this->info['cus_email'];?></td>
			</tr>
			<tr>
				<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Email Subject:</b></td>
				<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="email[subject]" size="50" value="Wishlist Items in Stock" /></td>
			</tr>
			<tr>
				<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Email Body:</b></td>
				<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><textarea name="email[body]" rows="15" cols="65" scrolling="virtual" style="font-family:Courier New;font-size:12"><?php echo $body;?></textarea></td>
			</tr>
			<?php echo $this->pg->outlineTableFoot();?>
			<p />
			<input type="submit" value="Send Email &gt;" class="btn" /> <input type="reset" value="Reset Form &gt;" class="btn" />
			</form>
			<?php
		}
		else
		{
			echo "The selected wishlist does not contain any in-stock items.";
		}
	}

	/**
	* Return the email body for the given itemIDs
	* @param array $items array of items (array must have 'itm_title' and 'pla_name' keys!)
	* @param integer $locale INSTORE/ONLINE (to determine which action to show the customer) [optional, default INSTORE]
	*/
	function email_body($items,$locale=INSTORE)
	{
		$words = (count($items)!=1?'items from your wishlist are':'item from your wishlist is');
		$words2 = (count($items)!=1?'these items':'this item');
		$body = "
This email has been sent to inform you that as of ".date('F jS, Y \a\t g:ia').", the following $words in stock:

------------------------------------------------------------

{items}

------------------------------------------------------------

{instructions}

------------------------------------------------------------
This email does not guarantee that you will receive this item, nor does it guarantee that the item will remain in our store for any specific amount of time. All items are offered on a first-come, first-serve basis.
";

		// generate the items
		$maxlength = 0;
		while (list($a,$arr) = each($items))
		{
			if (strlen($arr['pla_name']) > $maxlength) { $maxlength = strlen($arr['pla_name']); }
		}
		reset($items);

		$body_items = array();
		$either_note = '';
		$items = array_merge(array(array('pla_name'=>'PLATFORM','wli_newused'=>'NEW/USED','itm_title'=>'TITLE')),$items);
		while (list($a,$arr) = each($items))
		{
			$nu = ($arr['wli_newused']==ITEM_NEW?'New':($arr['wli_newused']==ITEM_USED?'Used':($arr['wli_newused']==EITHER?'Either*':'NEW/USED')));
			$body_items[] = ' '.str_pad($arr['pla_name'],$maxlength).'   '.str_pad($nu,8).'    '.$arr['itm_title'];

			if ($arr['wli_newused'] == EITHER && !strlen($either_note)) { $either_note = "\n\n * Please call to find out if this item is new/used."; }
		}
		$body = str_replace('{items}',implode("\n",$body_items).$either_note,$body);

		// generate the instructions
		if ($locale == INSTORE)
		{
			$instructions = "If you would like to reserve a copy of $words2, please call us. Otherwise, come in to the store as soon as possible!";
			$instructions .= "\n\n{$_SESSION['store_info']['sto_name']}";
			$instructions .= "\n{$_SESSION['store_info']['sto_address']}";
			$instructions .= "\n{$_SESSION['store_info']['sto_city']}, {$_SESSION['store_info']['sto_state']} {$_SESSION['store_info']['sto_zip']}";
			$instructions .= "\n".$this->pg->format('phone',$_SESSION['store_info']['sto_phone']);
		}
		else
		{
			$instructions = "Please visit http://www.fununlimitedonline.com to purchase $words!";
		}
		$body = str_replace('{instructions}',$instructions,$body);

		// wrap the email body at 60 characters
		$body = wordwrap($body,60,"\n");

		return $body;
	}

	/**
	* Send in-stock emails (called after item traded in/returned)
	* @param array $wishlists filtered array of wishlists
	*/
	function send_emails($wishlists)
	{
		while (list($a,$arr) = each($wishlists))
		{
			if (strlen($arr['cus_email']))
			{
				$this->info = $arr;
				$this->email = array(
					'subject'=>'Wishlist Items Now in Stock',
					'body'=>$this->email_body($arr['items'],INSTORE)
				);
				$this->send_email();
			}
		}
	}

	/**
	* Send the email stored in $this->email to the customer stored in $this->info
	*/
	function send_email()
	{
		global $db;

		if (!count($this->info) || !count($this->email))
		{
			$this->pg->error('Unable to send email - values missing');
		}
		else
		{
			$to = $this->info['cus_email'];
			$from = "{$_SESSION['store_info']['sto_name']} <{$_SESSION['store_info']['sto_email']}>";
			$subject = $this->email['subject'];
			$body = $this->email['body'];

			$sent = NO; //@mail($to,$subject,$body,"From: $from");

			if (!$sent) { $this->pg->error("Unable to send email to {$this->info['cus_email']}"); }
			else
			{
				$this->pg->status("Email successfully sent to {$this->info['cus_email']}");
				$sql = "UPDATE wishlist_items SET wli_emailsent=" . YES . " WHERE wli_wishlistID=$this->wishlistID AND wli_itemID IN (".implode(',',$_SESSION['wishlist_itemIDs']).")";
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}
		}
	}

	/**
	* Check the invoice trade/return items to see if any of them are in a wishlist
	* If so, set some session variables so the point-of-sale page opens the 'contact customer' window
	*/
	function checkInvoiceItems()
	{
		global $db;

		$itemIDs = array();
		$newused = array();
		$items = @$_SESSION['cust_items'];
		if (!is_array($items)) { $items = array(); }

		while (list($a,$arr) = each($items))
		{
			if ($arr['ini_type'] == TRADE || $arr['ini_type'] == RETURNS)
			{
				$itemIDs[] = $arr['ini_itemID'];
				$newused[] = $arr['ini_newused'];
			}
		}

		$this->check($itemIDs,$newused);
	}

	/**
	* Delete any invoice sale items from the customer's wishlist
	*/
	function deleteSaleItems()
	{
		global $db;

		$itemIDs = array();
		$newused = array();
		$items = @$_SESSION['cust_items'];
		if (!is_array($items)) { $items = array(); }

		while (list($a,$arr) = each($items))
		{
			if ($arr['ini_type'] == SALE)
			{
				$itemIDs[] = $arr['ini_itemID'];
				$newused[] = $arr['ini_newused'];
			}
		}

		if (count($itemIDs))
		{
			// get the customer's wishlist
			$wishlistID = 0;
			$this->get_wishlists(-1, -1, $_SESSION['customerID']);
			if (!count($this->wishlists)) { return; }

			// delete the items
			$wishlistID = array_shift(array_keys($this->wishlists));
			$sqls = array();
			while (list($a,$itemID) = each($itemIDs))
			{
				$sqls[] = "DELETE FROM wishlist_items WHERE wli_wishlistID=$wishlistID AND wli_itemID=$itemID AND wli_newused IN ({$newused[$a]},".EITHER.")";
			}

			while (list($a,$sql) = each($sqls))
			{
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}
		} // if itemIDs
	} // end function deleteSaleItems

	/**
	* Check if the given itemIDs/newused are in any wishlists - if so, set the needed variables
	* @param array $itemIDs
	* @param array $newused
	*/
	function check($itemIDs,$newused)
	{
		$in_wishlist = NO;

		if (count($itemIDs))
		{
			$this->get_wishlists();

			while (list($a,$arr) = each($this->wishlists))
			{
				while (list($b,$iarr) = each($arr['items']))
				{
					if (in_array($iarr['wli_itemID'],$itemIDs) && ($iarr['wli_newused'] == EITHER || $iarr['wli_newused'] == $newused[array_search($iarr['wli_itemID'],$itemIDs)]))
					{
						$in_wishlist = YES;
						break;
					}
					//if (in_array($iarr['wli_itemID'],$itemIDs)) { $in_wishlist = YES; break; }
				}

				if ($in_wishlist == YES) { break; }
			}
		}

		if ($in_wishlist == YES)
		{
			// if the point-of-sale class sees this variable set,
			// it will open a popup window to alert the user to
			// contact the customer(s)
			// note: the popup window sends the emails to the customers!
			$_SESSION['invoice_wishlist_itemIDs'] = $itemIDs;
		}
	}
}

?>