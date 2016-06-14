<?php
/**
* Store to store inventory movement class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class preorder
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
	* Current preorderID
	* @var integer $preorderID
	*/
	var $preorderID;

	/**
	* Preorder itemID
	* @var integer $itemID;
	*/
	var $itemID;

	/**
	* Preorder item information
	* @var array $item_info
	*/
	var $item_info;

	/**
	* Preorder information
	* @var array $info
	*/
	var $info;

	/**
	* Customer information
	* @var array $customerinfo
	*/
	var $customerinfo;

	/**
	* Customer list
	* @var array $customers
	*/
	var $customers;

	/**
	* Customer status'
	* @var array $customer_status
	*/
	var $customer_status;

	/**
	* Current customerID
	* @var integer $customerID
	*/
	var $customerID;

	/**
	* Customer close options
	* @var integer $close_options
	*/
	var $close_options;

	/**
	* Viewing active/completed preorders?
	* @var integer $active
	*/
	var $active;

	/**
	* All preorders
	* @var array $preorders
	*/
	var $preorders;

  /**
  * Status variable used for error reporting (bad email, bad zip code, success, etc)
  * @var array $status
  */
  var $status;

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	*/
	function preorder(&$pg)
	{
		$this->pg =& $pg;
		$this->preorderID = 0;
		$this->active = PRE_ACTIVE;
		$this->info = array();
		$this->status = array();
		$this->customers = array();
		$this->customerinfo = array();
		$this->customer_status = array();
		$this->close_options = array(
			PRE_UNRELEASED=>'',
			PRE_RECEIVED=>'Item Purchased/Received',
			PRE_REFUNDED=>'Money Refunded',
			PRE_FORFEITED=>'Money Forfeited'
		);

		$this->error = new error('Preorders class');
	}

	/**
	* Pull in all preorders for this store
	*/
	function get_preorders()
	{
		global $db;

		$itm = new items($this->pg);

		$this->preorders = array();
		$sql = "SELECT pre_preorderID,pre_itemID,pre_status,pre_notes,pre_releasedate FROM preorders WHERE pre_storeID=".$_SESSION['storeID']." ORDER BY pre_releasedate";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result))
		{
			$customers = array();
			$preorderID = $row['pre_preorderID'];
			$csql = "SELECT * FROM preorder_customers WHERE prc_preorderID=$preorderID";
			$cresult = mysql_query($csql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($crow = mysql_fetch_assoc($cresult)) { $customers[] = $crow; }
			$row['customers'] = $customers;

			$this->set_itemID($row['pre_itemID']);
			$row['item_info'] = $this->item_info;

			$this->preorders[] = $row;
		}
	}

	/**
	* Pulls the information from $_POST
	*/
	function pull_post()
	{
		$this->preorderID = @$_POST['preorderID'];
		if ($this->preorderID) { $this->set_preorderID($this->preorderID); }

		if (@$_POST['itemID']) { $this->set_itemID(@$_POST['itemID']); }

		$active = (isset($_GET['active'])?$_GET['active']:@$_POST['active']);
		if (!strlen($active)) { $active = PRE_ACTIVE; }
		$this->active = $active;

		if (isset($_POST['info']))
		{
			$this->info = $_POST['info'];
			while (list($key,$val) = each($this->info)) { $this->info[$key] = stripslashes($val); }
			reset($this->info);
		}

		if (isset($_POST['customerinfo']))
		{
			$this->customerinfo = $_POST['customerinfo'];
			while (list($key,$val) = each($this->customerinfo)) { $this->customerinfo[$key] = stripslashes($val); }
			reset($this->customerinfo);
		}

		if (isset($_POST['customer_status']))
		{
			$this->customer_status = $_POST['customer_status'];
			reset($this->customer_status);
		}

		$this->customerID = @$_POST['customerID'];
	}

	/**
	* Output the navigation buttons
	*/
	function nav_buttons()
	{
		?>
		<input type="button" value="Create New Preorder &gt;" onclick="document.location='/admin/utilities/preorder.php?act=new'" class="btn">
		<p />
		<?php
	}

	/**
	* Returns the number of active/completed preorders
	*/
	function preorder_count()
	{
		$active = 0;
		$completed = 0;

		while (list($a,$arr) = each($this->preorders))
		{
			if ($arr['pre_status'] == PRE_ACTIVE) { $active++; }
			elseif ($arr['pre_status'] == PRE_COMPLETED) { $completed++; }
		}
		reset($this->preorders);

		return array($active,$completed);
	}

	/**
	* Output the preorder list with navigation buttons
	* @param string $status
	*/
	function show_list($status="")
	{
		global $db;

		$this->nav_buttons();

		$this->get_preorders();
		list($totactive,$totcompleted) = $this->preorder_count();

		if ($this->active == PRE_ACTIVE) { $word = 'active'; $count = $totactive; }
		elseif ($this->active == PRE_COMPLETED) { $word = 'completed'; $count = $totcompleted; }

		if (!$count)
		{
			?>
			There are no <?=$word;?> preorders.
			<?php
		}
		else
		{
			// output the preorder list

			?>
			<script type="text/javascript">
				function go(where,preID)
				{
					if (where != 'delete' || (where == 'delete' && confirm('Are you sure you want to delete this preorder?\n\nNote: this is not reversible!')))
					{
						var frm = document.pres;
						frm.act.value = where;
						frm.preorderID.value = preID;
						frm.submit();
					}
				}
			</script>

			<?php
			if (strlen($status)) { $this->pg->status($status); }
			?>

			<form method="post" action="/admin/utilities/preorder.php" name="pres">
			<input type="hidden" name="act" value="">
			<input type="hidden" name="preorderID" value="">
			<input type="hidden" name="active" value="<?=$this->active;?>">
			<?=$this->pg->outlineTableHead();?>
			<tr bgcolor="<?=$this->pg->color('table-head');?>">
				<td><b>Title/Platform</b></td>
				<td><b>Release Date</b></td>
				<td><b>Details</b></td>
				<td><b>Functions</b></td>
			</tr>

			<?php

			$shown = -1;
			while (list($a,$arr) = each($this->preorders))
			{
				if ($arr['pre_status'] == $this->active)
				{
					$shown++;
					$bg = (!($shown%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

					$totcusts = count($arr['customers']);
					$totdown = 0;
					$totstatus = array(PRE_UNRELEASED=>0,PRE_RECEIVED=>0,PRE_REFUNDED=>0,PRE_FORFEITED=>0);
					while (list($b,$custarr) = each($arr['customers']))
					{
						$totdown += $custarr['prc_amountdown'];
						$totstatus[$custarr['prc_status']]++;
					}
					reset($arr['customers']);

					if ($arr['pre_releasedate'] <= time()) { $past = YES; } else { $past = NO; }

					?>
					<tr bgcolor="<?=$bg;?>">
						<td><?=($past==YES&&$arr['pre_status']==PRE_ACTIVE?'<font color="red"><b>':'');?><?=$arr['item_info']['title'];?><br /><?=$arr['item_info']['platformname'];?><?=($past==YES&&$arr['pre_status']==PRE_ACTIVE?'</b></font>':'');?></td>
						<td><?=($past==YES&&$arr['pre_status']==PRE_ACTIVE?'<font color="red"><b>':'');?><?=date('m/d/y',$arr['pre_releasedate']);?><?=($past==YES&&$arr['pre_status']==PRE_ACTIVE?'</b></font>':'');?></td>
						<td><?php
							echo ($past==YES&&$arr['pre_status']==PRE_ACTIVE?'<font color="red"><b>':'');
							?>
							Total Customers: <?=$totcusts;?><br />
							Total Down: $<?=number_format($totdown,2);?>
							<?php
							if ($arr['pre_status'] == PRE_COMPLETED)
							{
								echo "<br />Status: <b>COMPLETED</b><br />";
								while (list($status,$tot) = each($totstatus))
								{
									if ($status != PRE_UNRELEASED)
									{
										echo $this->close_options[$status].": $tot<br />";
									}
								}
							}

							echo ($past==YES&&$arr['pre_status']==PRE_ACTIVE?'</b></font>':'');
						?></td>
						<td>
							<?php
							if ($arr['pre_status'] == PRE_ACTIVE)
							{
								if ($past == YES)
								{
									// released items
									?>
									<input type="button" value="Complete Preorder &gt;" onclick="go('complete',<?=$arr['pre_preorderID'];?>)" class="btn"><br />
									<img src="/images/blank.gif" width="1" height="2"><br />
									<input type="button" value="Delete Preorder &gt;" onclick="go('delete',<?=$arr['pre_preorderID'];?>)" class="btn">
									<?php
								}
								else
								{
									// future releases
									?>
									<input type="button" value="View/Add Customers &gt;" onclick="go('view',<?=$arr['pre_preorderID'];?>)" class="btn"><br />
									<img src="/images/blank.gif" width="1" height="2"><br />
									<input type="button" value="Delete Preorder &gt;" onclick="go('delete',<?=$arr['pre_preorderID'];?>)" class="btn">
									<?php
								}
							}
							else
							{
								// completed preorders
								?>
								<input type="button" value="View Customers &gt;" onclick="go('view',<?=$arr['pre_preorderID'];?>)" class="btn"><br />
								<img src="/images/blank.gif" width="1" height="2"><br />
								<input type="button" value="Delete Preorder &gt;" onclick="go('delete',<?=$arr['pre_preorderID'];?>)" class="btn">
								<?php
							}
							?>
						</td>
					</tr>
					<?php
				}
			}
			?>

			<?=$this->pg->outlineTableFoot();?>
			</form>
			<?php
		}

		$viewactive = -1;
		if ($this->active == PRE_ACTIVE && $totcompleted) { $viewactive = PRE_COMPLETED; $opp = 'Completed'; $num = $totcompleted; }
		elseif ($this->active == PRE_COMPLETED && $totactive) { $viewactive = PRE_ACTIVE; $opp = 'Active'; $num = $totactive; }

		if ($viewactive > -1)
		{
			?>
			<p />
			<b>Viewing:</b> <?=($this->active==PRE_ACTIVE?'Active':'Completed');?> Preorders
			(<a href="/admin/utilities/preorder.php?active=<?=$viewactive;?>" title="View <?=$opp;?>"><?=$num;?> <?=$opp;?></a>)
			<?php
		}
	}

	/**
	* Generate a new preorder item (create the database entry)
	* @param integer $edit Editing the item? If NO, the item is being added
	*/
	function add_item($edit=NO)
	{
		global $db;

		if ($edit == YES)
		{
			// delete the item first
			$sql = "DELETE FROM preorders WHERE pre_preorderID=$this->preorderID";
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}

		// insert the new row and obtain the preorder ID
		$vals = array(
			($edit==NO?'NULL':$this->preorderID),
			$_SESSION['storeID'],
			$this->itemID,
			$this->info['price'],
			PRE_ACTIVE,
			mysql_escape_string($this->info['notes']),
			strtotime($this->info['releasedate'])
		);

		$sql = "INSERT INTO preorders VALUES ('".implode("','",$vals)."')";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		$preorderID = mysql_insert_id();

		$this->set_preorderID($preorderID);
	}

	/**
	* Output the complete preorder form (shows customers and their status)
	*/
	function complete_form()
	{
		if (!count($this->customer_status))
		{
			while (list($a,$arr) = each($this->customers))
			{
				$this->customer_status[$arr['prc_customerID']] = $arr['prc_status'];
			}
			reset($this->customers);
		}

		?>
		<script type="text/javascript">
			function verify(frm,doalert)
			{
				for (var i=0; i<frm.elements.length; i++)
				{
					if (frm.elements[i].type.substring(0,6) == 'select' && frm.elements[i].selectedIndex == 0)
					{
						if (doalert == true)
						{
							alert('You must select a status for each customer');
							frm.elements[i].focus();
						}
						return false;
						break;
					}
				}
				return true;
			}

			function saveinfo(frm)
			{
				var alldone = verify(frm,false);
				var docomplete = false;

				if (alldone == true && confirm('You have selected a status for every customer.\n\nWould you like to complete this preorder?')) { docomplete = true; }
				if (docomplete == false) { frm.complete.value = <?=NO;?> }

				frm.submit();
			}
		</script>

		<b>Title:</b> <?=$this->item_info['title'];?><br />
		<b>Platform:</b> <?=$this->item_info['platformname'];?><br />
		<b>Price:</b> $<?=number_format($this->info['price'],2);?><br />
		<?=(strlen($this->info['notes'])?'<b>Notes:</b> <a href="javascript:void(0)" title="'.$this->info['notes'].'">Mouse over to view notes</a>':'');?>
		<p />
		Select the status of each customer below:
		<p />
		<font size="1">
			<b>Note:</b> You can change the status of as many customers as you would like,<br />
			and then come back to this form later by pressing <b>Return to Preorders</b>.
		</font>
		<p />
		<?php

		$this->pg->outlineTableHead();
		?>
		<form method="post" action="/admin/utilities/preorder.php" name="precust" onsubmit="return verify(this,true)">
		<input type="hidden" name="act" value="docomplete">
		<input type="hidden" name="preorderID" value="<?=$this->preorderID;?>">
		<input type="hidden" name="complete" value="<?=YES;?>">
		<tr>
			<td colspan="4" align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b>Customers</b></td>
		</tr>
		<tr>
			<td align="center" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b>Name</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b>$ Down</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b>Balance</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b>Status</b></td>
		</tr>
		<?php
		if (count($this->customers))
		{
			$total_down = 0;
			$total_balance = 0;
			$total_price = 0;

			while (list($a,$arr) = each($this->customers))
			{
				$bg = (($a%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

				$total_down += $arr['prc_amountdown'];
				$total_balance += ($this->info['price']-$arr['prc_amountdown']);
				$total_price += $this->info['price'];

				?>
				<tr>
					<td bgcolor="<?=$bg;?>"><?=$arr['prc_fname'];?> <?=$arr['prc_lname'];?></td>
					<td align="right" bgcolor="<?=$bg;?>"><?=number_format($arr['prc_amountdown'],2);?></td>
					<td align="right" bgcolor="<?=$bg;?>"><?=number_format(($this->info['price']-$arr['prc_amountdown']),2);?></td>
					<td bgcolor="<?=$bg;?>"><select name="customer_status[<?=$arr['prc_customerID'];?>]" size="1"><?php
						while (list($v,$status) = each($this->close_options))
						{
							if ($v == @$this->customer_status[$arr['prc_customerID']]) { $s = ' selected="selected"'; } else { $s = ''; }
							?><option value="<?=$v;?>"<?=$s;?>><?=$status;?></option><?php
						}
						reset($this->close_options);
					?></select></td>
				</tr>
				<?php
			}

			$perc = sprintf('%0.1f',(($total_down/$total_price)*100));

			?>
			<tr>
				<td align="right" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b>Totals:</b></td>
				<td align="right" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b>$<?=number_format($total_down,2);?></b></td>
				<td align="right" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b>$<?=number_format($total_balance,2);?></b></td>
				<td align="center" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><font color="#999999"><?=$perc;?>% down</font></td>
			</tr>
			<?php
		} else { ?><tr><td colspan="4" bgcolor="<?=$this->pg->color('table-cell');?>">No customers on preorder</td></tr><?php }

		$this->pg->outlineTableFoot();
		?>
		<p />
		<input type="button" value="&lt; Return to Preorders" onclick="saveinfo(this.form)" class="btn">
		<input type="submit" value="Complete Preorder &gt;" class="btn">
		</form>
		<?php
	}

	/**
	* Set the current request as completed
	*/
	function complete()
	{
		global $db;

		$sqls = array();
		$complete = @$_POST['complete'];
		if (!strlen($complete)) { $complete = NO; }

		// set the customer status'
		while (list($customerID,$status) = each($this->customer_status))
		{
			$sqls[] = "UPDATE preorder_customers SET prc_status=$status WHERE prc_preorderID=$this->preorderID AND prc_customerID=$customerID";
		}

		if ($complete)
		{
			// change the status to complete
			$sqls[] = "UPDATE preorders SET pre_status=".PRE_COMPLETED." WHERE pre_preorderID=$this->preorderID";
		}

		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}

		return ($complete?'Preorder completed':'Saved preorder information');
	}

	/**
	* Set the active preorder ID and pull the information
	* @param integer $preorderID
	*/
	function set_preorderID($preorderID)
	{
		global $db;

		$this->preorderID = $preorderID;

		// pull in the preorder information
		$this->info = array();
		$sql = "SELECT * FROM preorders WHERE pre_preorderID=$preorderID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		$row = mysql_fetch_assoc($result);
		// remove 'pre_'
		if (is_array($row))
		{
			while (list($key,$val) = each($row)) { $this->info[substr($key,4)] = $val; }
		}

		// pull in the preorder customers
		$this->customers = array();
		$sql = "SELECT * FROM preorder_customers WHERE prc_preorderID=$preorderID ORDER BY prc_lname,prc_fname";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result)) { $this->customers[] = $row; }

		// pull in the item information
		$this->set_itemID($this->info['itemID']);
	}

	/**
	* Set the itemID
	* @param integer $itemID
	*/
	function set_itemID($itemID)
	{
		$this->itemID = $itemID;

		$itm = new items($this->pg);
		$itm->set_itemID($itemID);
		$this->item_info = $itm->info;

		$pla = new platforms($this->pg,$this->item_info['platformID']);
		$this->item_info['platformname'] = $pla->platform_name();
	}

	/**
	* Output the preorder item search form
	*/
	function item_form()
	{
		$its = new item_search($this->pg);
		$its->action = $_SESSION['root_admin'].'utilities/preorder.php';
		$its->form_head();
		$its->upc();
		$its->title();
		$its->platform();
		$its->form_foot('upc');
	}

	/**
	* Output the preorder item info form
	* @param integer $edit Editing the item? If NO, the item is being added
	*/
	function item_info_form($edit=NO)
	{
		?>
		<script type="text/javascript">
			function verify(frm)
			{
				if (frm.elements['info[price]'].value == '') { alert('You must enter a price'); frm.elements['info[price]'].focus(); return false; }
				else if (!validDate(frm.elements['info[releasedate]'].value)) { alert('Invalid date format (must be mm/dd/yyyy)'); frm.elements['info[releasedate]'].focus(); return false; }
				else { return true; }
			}
		</script>

		<?php
		if ($edit == NO) { ?>Enter the price and release date below to create a new preorder:<?php }
		else
		{
			?>You may change the price, release date, and notes below:<?php
			$this->info['releasedate'] = date('m/d/Y',$this->info['releasedate']);
		}
		?>
		<p />

		<?=$this->pg->outlineTableHead();?>
		<form method="post" action="/admin/utilities/preorder.php" name="pre" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="<?=($edit==NO?'createnew':'edititem');?>">
		<input type="hidden" name="preorderID" value="<?=$this->preorderID;?>">
		<input type="hidden" name="itemID" value="<?=$this->itemID;?>">
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Title:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><?=@$this->item_info['title'];?></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Platform:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><?=@$this->item_info['platformname'];?></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Price:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">$<input type="text" name="info[price]" size="6" value="<?=@$this->info['price'];?>" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value)" style="text-align:right"></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Release Date:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[releasedate]" size="12" value="<?=@$this->info['releasedate'];?>"> (mm/dd/yyyy)</td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Notes:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><textarea name="info[notes]" rows="4" cols="40" scrolling="auto"><?=@$this->info['notes'];?></textarea></td>
		</tr>
		<?=$this->pg->outlineTableFoot();?>

		<p />
		<input type="submit" value="<?=($edit==NO?'Create New':'Edit');?> Preorder &gt;" class="btn"> <input type="reset" value="Reset Form &gt;" class="btn">
		</form>
		<?php

		$this->pg->addOnload("document.pre.elements['info[price]'].focus()");
	}

	/**
	* Show the customer entry form (active)/customer info form (completed)
	* @param string $status
	*/
	function customer_form($status="")
	{
		?>
		<script type="text/javascript">
			function dodelete(cusID)
			{
				if (confirm('Are you sure you want to remove this customer from the preorder?'))
				{
					var frm = document.custfrm;
					frm.act.value = 'remcustomer';
					frm.customerID.value = cusID;
					frm.submit();
				}
			}

			function doedit()
			{
				var frm = document.custfrm;
				frm.act.value = 'edit';
				frm.submit();
			}
		</script>

		<form method="post" action="/admin/utilities/preorder.php" name="custfrm">
			<input type="hidden" name="preorderID" value="<?=$this->preorderID;?>">
			<input type="hidden" name="act" value="remcustomer">
			<input type="hidden" name="customerID" value="">
		</form>

		<b>Title:</b> <?=$this->item_info['title'];?><br />
		<b>Platform:</b> <?=$this->item_info['platformname'];?><br />
		<b>Price:</b> $<?=number_format($this->info['price'],2);?><br />
		<?=(strlen($this->info['notes'])?'<b>Notes:</b> <a href="javascript:void(0)" title="'.$this->info['notes'].'">Mouse over to view notes</a>':'');?>
		<p />
		<input type="button" value="&lt; Return to Preorders" onclick="document.location='/admin/utilities/preorder.php?act=close&active=<?=$this->active;?>'" class="btn">
		<?=($this->info['status']==PRE_ACTIVE?'<input type="button" value="Edit Preorder Information &gt;" onclick="javascript:doedit()" class="btn">':'');?>
		<p />
		<?php

		if (strlen($status)) { $this->pg->status($status); }

		if ($this->info['status'] == PRE_ACTIVE)
		{
			?>
			<table border="0" cellspacing="5" cellpadding="0" width="95%">
				<tr>
					<td align="center" valign="top" width="50%" class="normal">
						<?php
		}

		if ($this->info['status'] == PRE_ACTIVE)
		{
			?>
			<b>Add Customer</b>
			<p />
			To add a customer, enter their name<br />
			and the amount they are putting down.
			<p />

			<script type="text/javascript">
				function verify(frm)
				{
					if (frm.elements['customerinfo[fname]'].value == '' || frm.elements['customerinfo[lname]'].value == '') { alert('You must enter a customer name'); frm.elements['customerinfo[fname]'].focus(); return false; }
					else if (frm.elements['customerinfo[amountdown]'].value == '') { alert('You must enter an amount down'); frm.elements['customerinfo[amountdown]'].focus(); return false; }
					else { return true; }
				}
			</script>

			<?=$this->pg->outlineTableHead();?>
			<form method="post" action="/admin/utilities/preorder.php" name="precust" onsubmit="return verify(this)">
			<input type="hidden" name="act" value="addcustomer">
			<input type="hidden" name="preorderID" value="<?=$this->preorderID;?>">
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b>First Name:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="customerinfo[fname]" size="30"></td>
			</tr>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Last Name:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="customerinfo[lname]" size="30"></td>
			</tr>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Amount Down:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>">$<input type="text" name="customerinfo[amountdown]" size="6" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value)" style="text-align:right"></td>
			</tr>
			<?=$this->pg->outlineTableFoot();?>

			<p />
			<input type="submit" value="Add Customer &gt;" class="btn"> <input type="reset" value="Clear Form &gt;" class="btn">
			</form>
			<p />
			<?php
			$this->pg->addOnload("document.precust.elements['customerinfo[fname]'].focus()");
		}

		if ($this->info['status'] == PRE_ACTIVE)
		{
			?>
			</td>
			<td align="center" valign="top" width="50%" class="normal">
				<b>Current Customers</b>
				<p />
				<?php
		}

		$this->pg->outlineTableHead();
		?>
		<tr>
			<td colspan="4" align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b>Customers</b></td>
		</tr>
		<tr>
			<td align="center" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b>Name</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b>$ Down</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b>Balance</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b><?=($this->info['status']==PRE_ACTIVE?'Functions':'Status');?></b></td>
		</tr>
		<?php
		if (count($this->customers))
		{
			$total_down = 0;
			$total_balance = 0;
			$total_price = 0;

			while (list($a,$arr) = each($this->customers))
			{
				$bg = (($a%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

				$total_down += $arr['prc_amountdown'];
				$total_balance += ($this->info['price']-$arr['prc_amountdown']);
				$total_price += $this->info['price'];

				?>
				<tr>
					<td bgcolor="<?=$bg;?>"><?=$arr['prc_fname'];?> <?=$arr['prc_lname'];?></td>
					<td align="right" bgcolor="<?=$bg;?>"><?=number_format($arr['prc_amountdown'],2);?></td>
					<td align="right" bgcolor="<?=$bg;?>"><?=number_format(($this->info['price']-$arr['prc_amountdown']),2);?></td>
					<td bgcolor="<?=$bg;?>"><?php
						if ($this->info['status'] == PRE_ACTIVE) { ?><input type="button" value="Remove &gt;" onclick="dodelete(<?=$arr['prc_customerID'];?>)" class="btn"><?php }
						else { echo $this->close_options[$arr['prc_status']]; }
					?></td>
				</tr>
				<?php
			}

			$perc = sprintf('%0.1f',(($total_down/$total_price)*100));

			?>
			<tr>
				<td align="right" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b>Totals:</b></td>
				<td align="right" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b>$<?=number_format($total_down,2);?></b></td>
				<td align="right" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><b>$<?=number_format($total_balance,2);?></b></td>
				<td align="center" bgcolor="<?=$this->pg->color('table-head-lighter');?>"><font color="#999999"><?=$perc;?>% down</font></td>
			</tr>
			<?php
		} else { ?><tr><td colspan="4" bgcolor="<?=$this->pg->color('table-cell');?>">No customers on preorder</td></tr><?php }

		$this->pg->outlineTableFoot();

		if ($this->info['status'] == PRE_ACTIVE)
		{
						?>
					</td>
				</tr>
			</table>
			<?php
		}

		?>
		<p />
		<input type="button" value="&lt; Return to Preorders" onclick="document.location='/admin/utilities/preorder.php?act=close&active=<?=$this->active;?>'" class="btn">
		<?=($this->info['status']==PRE_ACTIVE?'<input type="button" value="Edit Preorder Information &gt;" onclick="javascript:doedit()" class="btn">':'');?>
		<?php
	}

	/**
	* Add a customer to the current preorder
	*/
	function add_customer()
	{
		global $db;

		$vals = array(
			'NULL',
			$this->preorderID,
			mysql_escape_string($this->customerinfo['fname']),
			mysql_escape_string($this->customerinfo['lname']),
			$this->customerinfo['amountdown'],
			NO
		);

		$sql = "INSERT INTO preorder_customers VALUES ('".implode("','",$vals)."')";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
	}

	/**
	* Remove a customer from the current preorder
	*/
	function remove_customer()
	{
		global $db;

		$sql = "DELETE FROM preorder_customers WHERE prc_preorderID=$this->preorderID AND prc_customerID=$this->customerID";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
	}

	/**
	* Delete the current preorder
	*/
	function dodelete()
	{
		global $db;

		$sqls = array(
			"DELETE FROM preorders WHERE pre_preorderID=$this->preorderID",
			"DELETE FROM preorder_customers WHERE prc_preorderID=$this->preorderID"
		);

		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}
	}
}

?>