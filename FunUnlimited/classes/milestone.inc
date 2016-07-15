<?php
/**
* Sales milestone discounts class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class milestone
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
	* Current milestoneID
	* @var integer $milestoneID
	*/
	var $milestoneID;

	/**
	* Milestone information
	* @var array $info
	*/
	var $info;

	/**
	* All milestones
	* @var array $milestones
	*/
	var $milestones;

	/**
	* Discount-qualifying customers
	* @var array $customers
	*/
	var $customers;

	/**
	* Sort order for the customer list
	* @var string $orderby
	*/
	var $orderby;

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
	function milestone(&$pg)
	{
		$this->pg =& $pg;
		$this->milestoneID = 0;
		$this->info = array();
		$this->status = array();

		$this->error = new error('Sales Milestone Discount class');
	}

	/**
	* Pull in all milestones for this store
	*/
	function get_milestones()
	{
		global $db;

		$this->milestones = array();
		$sql = "SELECT * FROM milestones WHERE mil_storeID=".$_SESSION['storeID']." ORDER BY mil_saleamount DESC";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result)) { $this->milestones[] = $row; }
	}

	/**
	* Pulls the information from $_POST
	* @param integer $loadinfo Load the information for the milestone? [optional, default YES]
	*/
	function pull_post($loadinfo=YES)
	{
		$this->milestoneID = @$_POST['milestoneID'];
		if ($this->milestoneID && $loadinfo) { $this->set_milestoneID($this->milestoneID); }

		if (isset($_POST['info']))
		{
			$this->info = $_POST['info'];
			while (list($key,$val) = each($this->info)) { $this->info[$key] = stripslashes($val); }
			reset($this->info);
		}

		$this->orderby = @$_POST['orderby'];
		if (!strlen($this->orderby)) { $this->orderby = 'name'; }
	}

	/**
	* Output the milestone add/edit form
	*/
	function add_form()
	{
		?>
		<script type="text/javascript">
			function verify(frm)
			{
				if (frm.elements['info[saleamount]'].value == '') { alert('You must enter a sale amount.'); frm.elements['info[saleamount]'].focus(); return false; }
				else if (frm.elements['info[range]'].selectedIndex == 0) { alert('You must select a date range.'); frm.elements['info[range]'].focus(); return false; }
				else if (frm.elements['info[salediscamount]'].value == '' && frm.elements['info[tradeupamount]'].value == '') { alert('You must enter a sale or trade discount amount.'); frm.elements['info[salediscamount]'].focus(); return false; }
				else { return true; }
			}

			function check_keypress(value,e,which)
			{
				var frm = document.msfrm;
				if (frm.elements['info['+which+']'][0].checked) { return onlynumbers(value,e,true); }
				else { return onlynumbers(value,e); }
			}

			function change_field(name)
			{
				document.msfrm.elements['info['+name+']'].value = format_price(document.msfrm.elements['info['+name+']'].value);
			}
		</script>

		<?=$this->pg->outlineTableHead();?>
		<form method="post" action="/admin/setup_invoice/milestones.php" onsubmit="return verify(this)" name="msfrm">
		<input type="hidden" name="act" value="<?=($this->milestoneID?'doedit':'add');?>">
		<input type="hidden" name="milestoneID" value="<?=$this->milestoneID;?>">
		<tr><td align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b><?=($this->milestoneID?'Edit':'Add New');?> Milestone</b></td></tr>
		<tr>
			<td align="center" bgcolor="<?=$this->pg->color('table-cell');?>">
				When a customer reaches $<input type="text" name="info[saleamount]" size="9" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value)" style="text-align:right" value="<?=@$this->info['saleamount'];?>"> in sales in
				<select name="info[range]" size="1"><?php
					$options = array(''=>'',DISC_RANGE_ALL=>'All Time',DISC_RANGE_YEAR=>'The Current Year',DISC_RANGE_MONTH=>'The Current Month');
					while (list($val,$show) = each($options))
					{
						if ($val == @$this->info['range'] && isset($this->info['range'])) { $s = ' selected="selected"'; } else { $s = ''; }
						?><option value="<?=$val;?>"<?=$s;?>><?=$show;?></option><?php
					}
				?></select><br />
				<table border="0" cellspacing="0" cellpadding="3">
					<tr>
						<td align="right">discount sales:</td>
						<td>
							<input type="text" name="info[salediscamount]" size="7" onkeypress="return check_keypress(this.value,event,'saledisctype')" style="text-align:right" value="<?=@$this->info['salediscamount'];?>">
							<input type="radio" name="info[saledisctype]" id="sp" value="<?=DISC_PERCENT;?>" class="nb"<?=(@$this->info['saledisctype']==DISC_PERCENT||!isset($this->info['saledisctype'])?' checked="checked"':'');?>> <label for="sp">percent</label>
							<input type="radio" name="info[saledisctype]" id="sd" value="<?=DISC_DOLLAR;?>" class="nb" onclick="change_field('salediscamount')"<?=(@$this->info['saledisctype']==DISC_DOLLAR?' checked="checked"':'');?>> <label for="sd">amount</label>
						</td>
					</tr>
					<tr>
						<td align="right">markup trades:</td>
						<td>
							<input type="text" name="info[tradeupamount]" size="7" onkeypress="return check_keypress(this.value,event,'tradeuptype')" style="text-align:right" value="<?=@$this->info['tradeupamount'];?>">
							<input type="radio" name="info[tradeuptype]" id="tp" value="<?=DISC_PERCENT;?>" class="nb"<?=(@$this->info['tradeuptype']==DISC_PERCENT||!isset($this->info['tradeuptype'])?' checked="checked"':'');?>> <label for="tp">percent</label>
							<input type="radio" name="info[tradeuptype]" id="td" value="<?=DISC_DOLLAR;?>" class="nb" onclick="change_field('tradeupamount')"<?=(@$this->info['tradeuptype']==DISC_DOLLAR?' checked="checked"':'');?>> <label for="td">amount</label>
						</td>
					</tr>
				</table>
			</td>
		</tr>
		<?php
		$this->pg->outlineTableFoot();
		?>
		<p />
		<input type="submit" value="<?=($this->milestoneID?'Edit':'Add');?> Milestone &gt;" class="btn">
		<input type="reset" value="Reset Form &gt;" class="btn">
		</form>
		<?php

		if ($this->milestoneID) { $this->pg->cancel("{$_SESSION['root_admin']}setup_invoice/milestones.php"); }
		$this->pg->addOnload("document.msfrm.elements['info[saleamount]'].focus()");
	}

	/**
	* Output the milestone list
	* @param string $status
	* @param integer $reset Reset the variables? [optional, default NO]
	*/
	function show_list($status="",$reset=NO)
	{
		global $db;

		if ($reset)
		{
			$this->milestoneID = 0;
			$this->info = array();
		}

		$this->get_milestones();

		echo (strlen($status)?$this->pg->status($status):'');

		$this->add_form();

		?><p /><hr width="75%" size="-1" color="#CCCCCC" /><p /><?php

		if (!count($this->milestones))
		{
			?>
			There are no sales milestones.
			<?php
		}
		else
		{
			// output the milestone list

			?>
			<script type="text/javascript">
				function go(where,mileID)
				{
					if (where != 'delete' || (where == 'delete' && confirm('Are you sure you want to delete this milestone?')))
					{
						var frm = document.miles;
						frm.act.value = where;
						frm.milestoneID.value = mileID;
						frm.submit();
					}
				}
			</script>

			<form method="post" action="/admin/setup_invoice/milestones.php" name="miles">
			<input type="hidden" name="act" value="">
			<input type="hidden" name="milestoneID" value="">
			<?=$this->pg->outlineTableHead();?>
			<tr bgcolor="<?=$this->pg->color('table-head');?>">
				<td><b>Sale Amount</b></td>
				<td><b>Date Range</b></td>
				<td><b>Discount/Markup</b></td>
				<td><b>Functions</b></td>
			</tr>
			<?php

			while (list($a,$arr) = each($this->milestones))
			{
				$bg = (($a%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

				?>
				<tr bgcolor="<?=$bg;?>">
					<td valign="middle">$<?=number_format($arr['mil_saleamount'],2);?></td>
					<td valign="middle"><?=($arr['mil_range']==DISC_RANGE_ALL?'All time':($arr['mil_range']==DISC_RANGE_YEAR?'Current year':'Current month'));?></td>
					<td valign="middle">
						Sales: <?php
						if ($arr['mil_saledisctype'] == DISC_PERCENT) { $p = ''; $s = '%'; }
						elseif ($arr['mil_saledisctype'] == DISC_DOLLAR) { $p = '$'; $s = ''; }
						echo $p.number_format($arr['mil_salediscamount'],2).$s;
						?><br />
						Trades: <?php
						if ($arr['mil_tradeuptype'] == DISC_PERCENT) { $p = ''; $s = '%'; }
						elseif ($arr['mil_tradeuptype'] == DISC_DOLLAR) { $p = '$'; $s = ''; }
						echo $p.number_format($arr['mil_tradeupamount'],2).$s;
						?>
					</td>
					<td>
						<input type="button" value="Edit Milestone &gt;" onclick="go('edit',<?=$arr['mil_milestoneID'];?>)" class="btn"><br />
						<img src="/images/blank.gif" width="1" height="2"><br />
						<input type="button" value="Delete Milestone &gt;" onclick="go('delete',<?=$arr['mil_milestoneID'];?>)" class="btn"><br />
						<img src="/images/blank.gif" width="1" height="2"><br />
						<input type="button" value="View Qualifying Customers &gt;" onclick="go('showcusts',<?=$arr['mil_milestoneID'];?>)" class="btn">
					</td>
				</tr>
				<?php
			}
			?>

			<?=$this->pg->outlineTableFoot();?>
			</form>
			<?php
		}
	}

	/**
	* Add/edit a new milestone
	*/
	function add()
	{
		global $db;

		if ($this->milestoneID)
		{
			$this->delete();
			$setID = $this->milestoneID;
		}
		else { $setID = 'NULL'; }

		$sql = "INSERT INTO milestones VALUES ($setID,{$_SESSION['storeID']},{$this->info['saleamount']},{$this->info['range']},{$this->info['salediscamount']},{$this->info['saledisctype']},{$this->info['tradeupamount']},{$this->info['tradeuptype']})";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
	}

	/**
	* Delete a milestone
	*/
	function delete()
	{
		global $db;

		$sql = "DELETE FROM milestones WHERE mil_milestoneID=$this->milestoneID";

		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
	}

	/**
	* Set the active milestone ID and pull the information
	* @param integer $milestoneID
	*/
	function set_milestoneID($milestoneID)
	{
		global $db;

		$this->milestoneID = $milestoneID;

		$sql = "SELECT * FROM milestones WHERE mil_milestoneID=$milestoneID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		$row = mysql_fetch_assoc($result);
		// remove 'mil_'
		if (is_array($row))
		{
			while (list($key,$val) = each($row)) { $this->info[substr($key,4)] = $val; }
		}
	}

	/**
	* Pulls the milestone-qualifying customers into $this->customers
	*/
	function pull_customers()
	{
		global $db;

		$this->customers = array();

		$orderby = ($this->orderby=='name'?'cus_lname,cus_fname':'total DESC');

		$sql = "SELECT inv_customerID,SUM(ini_price) AS total,cus_fname,cus_lname,cus_address,cus_city,cus_state,cus_zip,cus_phone,cus_cellphone,cus_email FROM invoices,invoice_items,customers WHERE inv_storeID=".$_SESSION['storeID']." AND inv_completed=" . YES . " AND inv_invoiceID=ini_invoiceID AND ini_type=".SALE." AND inv_customerID=cus_customerID GROUP BY inv_customerID ORDER BY $orderby";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			if ($row['total'] >= $this->info['saleamount']) { $this->customers[] = $row; }
		}
	}

	/**
	* Output the list of qualifying customers
	*/
	function show_customers()
	{
		?>
		<input type="button" value="&lt; Return to Milestones" onclick="document.location='/admin/setup_invoice/milestones.php'" class="btn">
		<p />
		<?php

		$range = ($this->info['range']==DISC_RANGE_ALL?'All time':($this->info['range']==DISC_RANGE_YEAR?'Current year':'Current month'));

		if ($this->info['saledisctype'] == DISC_PERCENT) { $sp = ''; $ss = '%'; }
		elseif ($this->info['saledisctype'] == DISC_DOLLAR) { $sp = '$'; $ss = ''; }

		if ($this->info['tradeuptype'] == DISC_PERCENT) { $tp = ''; $ts = '%'; }
		elseif ($this->info['tradeuptype'] == DISC_DOLLAR) { $tp = '$'; $ts = ''; }

		$this->pg->outlineTableHead();
		?>
		<tr><td colspan="2" align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b>Milestone Information</b></td></tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><b>Sale Amount:</b></td>
			<td align="right" bgcolor="<?=$this->pg->color('table-cell2');?>">$<?=number_format($this->info['saleamount'],2);?></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><b>Date Range:</b></td>
			<td align="right" bgcolor="<?=$this->pg->color('table-cell2');?>"><?=$range;?></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><b>Sale Discount:</b></td>
			<td align="right" bgcolor="<?=$this->pg->color('table-cell2');?>"><?=$sp.number_format($this->info['salediscamount'],2).$ss;?></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><b>Trade Markup:</b></td>
			<td align="right" bgcolor="<?=$this->pg->color('table-cell2');?>"><?=$tp.number_format($this->info['tradeupamount'],2).$ts;?></td>
		</tr>
		<?=$this->pg->outlineTableFoot();?>
		<p />

		<script type="text/javascript">
			function orderby(ob)
			{
				var frm = document.msob;
				frm.orderby.value = ob;
				frm.submit();
			}
		</script>
		<form method="post" action="/admin/setup_invoice/milestones.php" name="msob">
			<input type="hidden" name="act" value="showcusts">
			<input type="hidden" name="milestoneID" value="<?=$this->milestoneID;?>">
			<input type="hidden" name="orderby" value="">
		</form>

		<?php
		if (count($this->customers))
		{
			?>
			<font size="1">
				<b>Order List By:</b>
				<?=($this->orderby=='name'?'':'<a href="javascript:orderby(\'name\')">');?>Customer Name<?=($this->orderby=='name'?'':'</a>');?>
				|
				<?=($this->orderby=='totsale'?'':'<a href="javascript:orderby(\'totsale\')">');?>Total Sales<?=($this->orderby=='totsale'?'':'</a>');?>
			</font>
			<p />
			<?php
		}

		$this->pg->outlineTableHead();
		?>
		<tr>
			<td colspan="2" align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b>Customer</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b>Total Sales</b></td>
		</tr>
		<?php
		while (list($a,$arr) = each($this->customers))
		{
			?>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-cell');?>">
					<b><?=$arr['cus_fname'].' '.$arr['cus_lname'];?></b><br />
					<?=$arr['cus_address'];?><br />
					<?=$arr['cus_city'].' '.$arr['cus_state'].', '.$arr['cus_zip'];?>
				</td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>">
					<?=$this->pg->format('phone',$arr['cus_phone']);?><?=(strlen($arr['cus_cellphone'])?' (Home)<br />'.$this->pg->format('phone',$arr['cus_cellphone']).' (Cell)':'');?><br />
					<?=(strlen($arr['cus_email'])?'<a href="mailto:'.$arr['cus_email'].'">'.$arr['cus_email'].'</a>':'');?>
				</td>
				<td align="right" bgcolor="<?=$this->pg->color('table-cell2');?>">$<?=number_format($arr['total'],2);?></td>
			</tr>
			<?php
		}
		if (!count($this->customers)) { ?><tr><td colspan="3" bgcolor="<?=$this->pg->color('table-cell2');?>">No qualifying customers</td></tr><?php }

		$this->pg->outlineTableFoot();

		?>
		<p />
		<input type="button" value="&lt; Return to Milestones" onclick="document.location='/admin/setup_invoice/milestones.php'" class="btn">
		<?php
	}

	/**
	* Return the discount for a given customerID
	* @param integer $customerID
	*/
	function get_customer_discount($customerID)
	{
		global $db;

		$return = array(0,0,0,0,0,0,0); // array(milestoneID,range,sale_discount_type,sale_discount_amount,trade_discount_type,trade_discount_amount,customer_sales_total)

		$this->get_milestones();

		while (list($a,$arr) = each($this->milestones))
		{
			// find the date according to the range
			if ($arr['mil_range'] == DISC_RANGE_ALL) { $time = ''; }
			elseif ($arr['mil_range'] == DISC_RANGE_YEAR) { $time = ' AND ini_timeadded>='.strtotime('January 1 '.date('Y').' 12:00am'); }
			elseif ($arr['mil_range'] == DISC_RANGE_MONTH) { $time = ' AND ini_timeadded>='.strtotime(date('m').'/1/'.date('Y').' 12:00am'); }

			// get trade total
			$sql = "SELECT SUM(ini_price) AS total FROM invoices,invoice_items WHERE inv_storeID=".$_SESSION['storeID']." AND inv_completed=" . YES . " AND inv_invoiceID=ini_invoiceID AND ini_type=".TRADE." AND inv_customerID=$customerID".$time;
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			if (mysql_num_rows($result))
			{
				$row = mysql_fetch_assoc($result);
				$tradetotal = $row['total'];
			} else { $tradetotal = 0; }

			// get sale total
			$sql = "SELECT SUM(ini_price) AS total FROM invoices,invoice_items WHERE inv_storeID=".$_SESSION['storeID']." AND inv_completed=" . YES . " AND inv_invoiceID=ini_invoiceID AND ini_type=".SALE." AND inv_customerID=$customerID".$time;
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			if (mysql_num_rows($result))
			{
				$row = mysql_fetch_assoc($result);
				$saletotal = $row['total'];
			} else { $saletotal = 0; }

			// sale total is all sales minus all trades
			$saletotal -= $tradetotal;
			if ($saletotal < 0) { $saletotal = 0; }

			if ($saletotal >= $arr['mil_saleamount'])
			{
				$return = array($arr['mil_milestoneID'],$arr['mil_range'],$arr['mil_saledisctype'],$arr['mil_salediscamount'],$arr['mil_tradeuptype'],$arr['mil_tradeupamount'],sprintf('%0.2f',$saletotal));
				break;
			}
		}

		return $return;
	}
}

?>