<?php
/**
* Per-copy discounts class (adding/editing/removing)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class percopy_discounts
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
	function percopy_discounts(&$pg)
	{
		$this->pg =& $pg;
		$this->storeID = $_SESSION['storeID'];
		$this->discounts = array();
		$this->status = array();

		$this->error = new error('Per-Copy Discounts');

		$this->get_discounts();
	}

	/**
	* Pull in the current discounts
	*/
	function get_discounts()
	{
		global $db;

		$this->discounts = array();

		$sql = "SELECT * FROM percopy_discounts WHERE pcd_storeID=$this->storeID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			if (strlen($row['pcd_discount']))
			{
				$row['pcd_discount'] = sprintf('%0.2f',$row['pcd_discount']);
			}
			$this->discounts[$row['pcd_platformID']] = array(
				$row['pcd_discount'],
				$row['pcd_percent']
			);
		}

		if (!isset($this->discounts[0]))
		{
			$this->discounts[0] = array('',NO);
		}
	}

	/**
	* Outputs the listing of discounts with textboxes to change for each platform
	*/
	function discounts_form()
	{
		?>
		Please enter the amount to subtract (on a trade) for each additional copy in stock.
		<p />
		Any values may be left blank; the <b>Default</b> value will be used.
		<p />

		<script type="text/javascript">
			function verify(frm)
			{
				if (frm.d0.value.length == 0) { alert('You must enter a default discount.'); frm.d0.focus(); return false; }
				else { return true; }
			}
		</script>
		<?php

		$this->pg->outlineTableHead();
		?>

		<form method="post" action="/admin/setup_invoice/percopy_discounts.php" name="pcfrm" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="set">
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Platform</b></td>
			<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>$/% Off</b></td>
			<td align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Discount</b></td>
		</tr>
		<?php
		$pla = new platforms($this->pg,0);
		$pla->set_item('platforms');
		$pla->values = array_merge(array(0=>array(0,'<b>Default</b>')),$pla->values);
		while (list($a,$arr) = each($pla->values))
		{
			$id   = $arr[0];
			$name = $arr[1];

			$disc = @$this->discounts[$id][0];
			$perc = @$this->discounts[$id][1];

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
					$<input type="text" name="disc[<?php echo $id;?>]" size="5" value="<?php echo $disc;?>" id="d<?php echo $id;?>" style="vertical-align:middle" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value,false)" style="text-align:right">
				</td>
			</tr>
			<?php
		}
		$this->pg->outlineTableFoot();

		?>
		<p />
		<input type="submit" value="Update Discounts &gt;" class="btn">
		</form>
		<?php

		$this->pg->addOnload('document.pcfrm.d0.focus()');
	}

	/**
	* Pulls the information from $_POST
	*/
	function pull_post()
	{
		$disc = $_POST['disc'];
		$perc = $_POST['perc'];
		$platformIDs = array_keys($disc);

		$this->discounts = array();

		while (list($a,$platformID) = each($platformIDs))
		{
			$this_disc = @$disc[$platformID];
			$this_perc = @$perc[$platformID];

			if (strlen($this_disc) && !is_numeric($this_disc))
			{
				$this_disc = '';
			}
			elseif (is_numeric($this_disc))
			{
				$this_disc = sprintf('%0.2f',$this_disc);
			}

			$this->discounts[$platformID] = array($this_disc,$this_perc);
		}

		ksort($this->discounts);
	}

	/**
	* Set the discounts for the platforms in $this->discounts
	*/
	function set()
	{
		global $db;

		$sqls = array("DELETE FROM percopy_discounts WHERE pcd_storeID={$_SESSION['storeID']}");

		$vals = array();

		while (list($platformID,list($disc,$perc)) = each($this->discounts))
		{
			$vals[] = "($platformID,$this->storeID,'$disc',$perc)";
		}
		$sqls[] = "INSERT INTO percopy_discounts VALUES " . implode(',',$vals);

		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}

		$this->status[0] = SET;
	}

  /**
  * Check the status of the per-copy discount set
  * @return integer YES/NO
  */
  function was_set()
  {
    if ($this->status[0] == SET) { return YES; }
    else { return NO; }
  }
}

?>