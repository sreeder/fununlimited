<?php
/**
* Box discounts class (adding/editing/removing)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class box_discounts
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
	* Discounts
	* @var array $discounts
	*/
	var $discounts;

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
	function box_discounts(&$pg)
	{
		$this->pg =& $pg;
		$this->storeID = $_SESSION['storeID'];
		$this->discounts = array();
		$this->status = array();

		$this->error = new error('Box Discounts');

		$this->get_discounts();
	}

	/**
	* Pull in the current discounts
	*/
	function get_discounts()
	{
		global $db;

		$this->discounts = array();

		$sql = "SELECT * FROM box_discounts WHERE bod_storeID=$this->storeID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			$this->discounts[$row['bod_platformID']] = array(
				$row['bod_sale_nobox'],
				$row['bod_sale_storebox'],
				$row['bod_trade_nobox'],
				$row['bod_dohalf'],
				$row['bod_percent']
			);
		}

		if (!isset($this->discounts[0]))
		{
			$this->discounts[0] = array('','','',NO,NO);
		}
	}

	/**
	* Outputs the listing of discounts with textboxes to change for each platform
	*/
	function discounts_form()
	{
		?>
		Please enter the discount for each box type.
		<p />
		Any values may be left blank; the <b>Default</b> values will be used.
		<p />

		<script type="text/javascript">
			function verify(frm)
			{
				if (frm.snb0.value.length == 0) { alert('You must enter a default sale no box discount.'); frm.snb0.focus(); return false; }
				else if (frm.sspb0.value.length == 0) { alert('You must enter a default sale store printed box discount.'); frm.sspb0.focus(); return false; }
				else if (frm.tnb0.value.length == 0) { alert('You must enter a default trade no box discount.'); frm.tnb0.focus(); return false; }
				else { return true; }
			}
		</script>
		<?php

		$this->pg->outlineTableHead();
		?>

		<form method="post" action="/admin/setup_invoice/box_discounts.php" name="bdfrm" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="set">
		<tr>
			<td rowspan="2" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Platform</b></td>
			<td align="center" rowspan="2" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>$/% Off</b></td>
			<td align="center" colspan="2" bgcolor="<?php echo $this->pg->color('table-head-darker');?>"><b>Sale</b></td>
			<td align="center" colspan="2" bgcolor="<?php echo $this->pg->color('table-head-darker');?>"><b>Trade</b></td>
		</tr>
		<tr>
			<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>No Box</b></td>
			<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Store Printed Box</b></td>
			<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>No Box</b></td>
			<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>1/2 Amount?</b></td>
		</tr>
		<?php
		$pla = new platforms($this->pg,0);
		$pla->set_item('platforms');
		$pla->values = array_merge(array(0=>array(0,'<b>Default</b>')),$pla->values);
		while (list($a,$arr) = each($pla->values))
		{
			$id   = $arr[0];
			$name = $arr[1];

			$snb  = @$this->discounts[$id][0];
			$sspb = @$this->discounts[$id][1];
			$tnb  = @$this->discounts[$id][2];
			$dh   = @$this->discounts[$id][3];
			$perc = @$this->discounts[$id][4];

			?>
			<tr>
				<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><?php echo $name;?></td>
				<td bgcolor="<?php echo $this->pg->color('table-cell');?>">
					<input type="radio" name="perc[<?php echo $id;?>]" id="pn<?php echo $id;?>" value="<?php echo NO;?>" class="nb"<?php echo (!$perc ? ' checked="checked"' : '');?> />
					<label for="pn<?php echo $id;?>">$</label>
					<input type="radio" name="perc[<?php echo $id;?>]" id="py<?php echo $id;?>" value="<?php echo YES;?>" class="nb"<?php echo ($perc ? ' checked="checked"' : '');?> />
					<label for="py<?php echo $id;?>">%</label>
				</td>
				<td align="center" bgcolor="<?php echo $this->pg->color('table-cell2');?>">
					<input type="text" name="sale_nobox[<?php echo $id;?>]" size="5" value="<?php echo $snb;?>" id="snb<?php echo $id;?>" style="vertical-align:middle" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value,false)" style="text-align:right">
				</td>
				<td align="center" bgcolor="<?php echo $this->pg->color('table-cell2');?>">
					<input type="text" name="sale_storebox[<?php echo $id;?>]" size="5" value="<?php echo $sspb;?>" id="sspb<?php echo $id;?>" style="vertical-align:middle" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value,false)" style="text-align:right">
				</td>
				<td align="center" bgcolor="<?php echo $this->pg->color('table-cell2');?>">
					<input type="text" name="trade_nobox[<?php echo $id;?>]" size="5" value="<?php echo $tnb;?>" id="tnb<?php echo $id;?>" style="vertical-align:middle" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value,false)" style="text-align:right">
				</td>
				<label for="dh<?php echo $id;?>">
				<td style="cursor:hand" align="center" bgcolor="<?php echo $this->pg->color('table-cell2');?>">
					<input type="checkbox" name="dohalf[<?php echo $id;?>]" value="<?php echo YES;?>" id="dh<?php echo $id;?>" style="vertical-align:middle;border-width:0px"<?php echo ($dh==YES?' checked="checked"':'');?>>
				</td>
				</label>
			</tr>
			<?php
		}
		$this->pg->outlineTableFoot();

		?>
		<p />
		<input type="submit" value="Update Discounts &gt;" class="btn">
		</form>

		<p />
		<font size="1">
			<b>Note:</b> When an item is added to a trade whose platform has the <b>1/2 Amount</b><br />
			box checked and the original price minus the box discount is less than<br />
			1/2 the original, the price will be set to 1/2 the original.
			<p />
			<b>For Example:</b> Original price is $6.50; no box discount is $5.00;<br />
			$6.50 - $5.00 = $1.50, which is less than 1/2 the original price ($3.25);<br />
			the price will be set to $3.25.
		</font>
		<?php

		//$this->pg->addOnload('document.bdfrm.nb0.focus()');
	}

	/**
	* Pulls the information from $_POST
	*/
	function pull_post()
	{
		$perc   = $_POST['perc'];
		$snb    = $_POST['sale_nobox'];
		$sspb   = $_POST['sale_storebox'];
		$tnb    = $_POST['trade_nobox'];
		$dohalf = @$_POST['dohalf'];
		$platformIDs = array_keys($snb);

		$this->discounts = array();

		while (list($a,$platformID) = each($platformIDs))
		{
			$this_snb  = @$snb[$platformID];
			$this_sspb = @$sspb[$platformID];
			$this_tnb  = @$tnb[$platformID];
			$this_dh   = @$dohalf[$platformID];
			$this_perc = @$perc[$platformID];

			if (strlen($this_snb) && !is_numeric($this_snb))
			{
				$this_snb = '';
			}
			elseif (is_numeric($this_snb))
			{
				$this_snb = sprintf('%0.2f',$this_snb);
			}
			if (strlen($this_sspb) && !is_numeric($this_sspb))
			{
				$this_sspb = '';
			}
			elseif (is_numeric($this_sspb))
			{
				$this_sspb = sprintf('%0.2f',$this_sspb);
			}
			if (strlen($this_tnb) && !is_numeric($this_tnb))
			{
				$this_tnb = '';
			}
			elseif (is_numeric($this_tnb))
			{
				$this_tnb = sprintf('%0.2f',$this_tnb);
			}

			$this->discounts[$platformID] = array(
				$this_snb,
				$this_sspb,
				$this_tnb,
				$this_dh,
				$this_perc
			);
		}

		ksort($this->discounts);
	}

	/**
	* Set the discounts for the platforms in $this->discounts
	*/
	function set()
	{
		global $db;

		$sqls = array("DELETE FROM box_discounts WHERE bod_storeID={$_SESSION['storeID']}");

		$vals = array();

		while (list($platformID,list($snb,$sspb,$tnb,$dh,$perc)) = each($this->discounts))
		{
			if (!$platformID && !strlen($dh)) { $dh = NO; }
			$vals[] = "($platformID,$this->storeID,'$snb','$sspb','$tnb','$dh',$perc)";
		}
		$sqls[] = "INSERT INTO box_discounts VALUES ".implode(',',$vals);

		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}

		$this->status[0] = SET;
	}

  /**
  * Check the status of the box discount set
  * @return integer YES/NO
  */
  function was_set()
  {
    if ($this->status[0] == SET) { return YES; }
    else { return NO; }
  }
}

?>