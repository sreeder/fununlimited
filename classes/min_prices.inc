<?php
/**
* Minimum prices class (adding/editing/removing)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class min_prices
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
	* Prices
	* @var array $prices
	*/
	var $prices;

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
	function min_prices(&$pg)
	{
		$this->pg =& $pg;
		$this->storeID = $_SESSION['storeID'];
		$this->prices = array();
		$this->status = array();

		$this->error = new error('Minimum Prices');

		$this->get_prices();
	}

	/**
	* Pull in the current minimum prices
	*/
	function get_prices()
	{
		global $db;

		$this->percs = array();

		$sql = "SELECT * FROM min_prices WHERE mnp_storeID=$this->storeID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			$this->prices[$row['mnp_platformID']] = $row['mnp_price'];
		}

		if (!isset($this->prices[0])) { $this->prices[0] = '0.00'; }
	}

	/**
	* Outputs the listing of prices with textboxes to change for each platform
	*/
	function prices_form()
	{
		?>
		Please enter the minimum price for each platform.
		<p />
		Any values may be left blank; the <b>Default</b> values will be used.
		<p />

		<script type="text/javascript">
			function verify(frm)
			{
				if (frm.mp0.value.length == 0) { alert('You must enter a default minimum price.'); frm.mp0.focus(); return false; }
				else { return true; }
			}
		</script>
		<?php

		$this->pg->outlineTableHead();
		?>

		<form method="post" action="/admin/setup_invoice/min_prices.php" name="mpfrm" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="set">
		<tr>
			<td bgcolor="<?=$this->pg->color('table-head');?>"><b>Platform</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b>Price</b></td>
		</tr>
		<?php
		$pla = new platforms($this->pg,0);
		$pla->set_item('platforms');
		$pla->values = array_merge(array(0=>array(0,'<b>Default</b>')),$pla->values);
		while (list($a,$arr) = each($pla->values))
		{
			$id = $arr[0];
			$name = $arr[1];

			$mp = @$this->prices[$id];

			?>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-cell');?>"><?=$name;?></td>
				<td align="center" bgcolor="<?=$this->pg->color('table-cell2');?>">
					$<input type="text" name="price[<?=$id;?>]" size="5" value="<?=$mp;?>" id="mp<?=$id;?>" style="vertical-align:middle" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value,false)" style="text-align:right">
				</td>
			</tr>
			<?php
		}
		$this->pg->outlineTableFoot();

		?>
		<p />
		<input type="submit" value="Update Prices &gt;" class="btn">
		</form>

		<p />
		<font size="1">
			<b>Note:</b> This feature may be used to define a flat rate for an entire platform.<br />
			Set the box discounts to a large number (IE: $1000.00), uncheck <b>1/2 Amount</b>, and set the<br />
			minimum price to the flat rate. When the item is added to an invoice, select <b>No Box</b> or<br />
			<b>Store Printed Box</b> and the price will be set to the minimum price/flat rate.
		</font>
		<?php

		//$this->pg->addOnload('document.mpfrm.mp0.focus()');
	}

	/**
	* Pulls the information from $_POST
	*/
	function pull_post()
	{
		$price = $_POST['price'];
		$platformIDs = array_keys($price);

		$this->prices = array();

		while (list($a,$platformID) = each($platformIDs))
		{
			if (strlen($price[$platformID])) { $price[$platformID] = sprintf('%0.2f',$price[$platformID]); }
			$this->prices[$platformID] = @$price[$platformID];
		}

		ksort($this->prices);
	}

	/**
	* Set the minimum prices for the platforms in $this->prices
	*/
	function set()
	{
		global $db;

		$sqls = array("DELETE FROM min_prices WHERE mnp_storeID={$_SESSION['storeID']}");

		$vals = array();

		while (list($platformID,$mp) = each($this->prices))
		{
			$vals[] = "($platformID,$this->storeID,'$mp')";
		}
		$sqls[] = "INSERT INTO min_prices VALUES ".implode(',',$vals);

		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}

		$this->status[0] = SET;
	}

  /**
  * Check the status of the minimum price set
  * @return integer YES/NO
  */
  function was_set()
  {
    if ($this->status[0] == SET) { return YES; }
    else { return NO; }
  }
}

?>