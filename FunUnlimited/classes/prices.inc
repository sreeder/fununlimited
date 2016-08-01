<?php
/**
* Prices
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class prices
{
	/**
	* page class reference
	* @var reference $pg
	*/
	var $pg;

	/**
	* Array of prices
	* @var array $prices
	*/
	var $prices;

	/**
	* Array of price changes
	* @var array $changes
	*/
	var $changes;

	/**
	* Error reporting class
	* @var identifier $error
	*/
	var $error;

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	*/
	function prices(&$pg)
	{
		$this->pg =& $pg;

		$this->error = new error('Prices');
	}

	/**
	* Pulls in the the price(s) for the given item(s)
	* @param integer/array $itemIDs array of itemIDs (can be array or single itemID) [optional, default array()]
	*/
	function setPrices($itemIDs=array())
	{
		global $db;

		$this->prices = array();
		if (!is_array($itemIDs)) { $itemIDs = array($itemIDs); }
		while (list($a,$itemID) = each($itemIDs)) { $this->prices[$itemID] = 0; }

		$sql = "SELECT prc_itemID,prc_price,prc_fet FROM prices WHERE prc_storeID={$_SESSION['storeID']}".(count($itemIDs)?" AND prc_itemID IN (".implode(',',$itemIDs).")":"");
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result)) { $this->prices[$row['prc_itemID']] = $row; }
	}

	/**
	* Retrieves the price(s) for the given itemID(s)
	* @param integer/array $itemIDs array of itemIDs (can be array or single itemID) [optional, default array()]
	* @return array of prices/a single price; format: array(itemID=>array(price_info)[,itemID=>...])
	*/
	function getPrices($itemIDs=array())
	{
		if (is_array($itemIDs) && !count($itemIDs)) { return $this->prices; }
		if (!is_array($itemIDs)) { return @$this->prices[$itemIDs]; }

		$return = array();
		while (list($a,$itemID) = each($itemIDs))
		{
			if (isset($this->prices[$itemID])) { $return[$itemID] = $this->prices[$itemID]; }
		}

		return $return;
	}

	/**
	* Set the pricing for the given itemID
	* @param integer $itemID
	* @param float $new new price; if -1, it is not set
	* @param float $used used price; if -1, it is not set
	* @param integer $return_changed if YES, returnes the number of changed rows (mysql_affected_rows())
	* @return integer (only if $return_changed=YES)
	*/
	function setPrice($itemID,$new,$used,$return_changed=NO)
	{
		global $db;

		// subtract a penny from even dollar amounts (IE: $15.00 becomes $14.99)
		$new = $this->fix_price(sprintf('%0.2f',$new));
		$used = $this->fix_price(sprintf('%0.2f',$used));
		//echo "used: " . $used;

		// see if the item exists in the 'prices' table; if so, pull the item's pricing
		$exists = NO;
		$old_prices = array();
		$sql = "SELECT * FROM prices WHERE prc_itemID=$itemID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		if (mysql_num_rows($result))
		{
			$exists = YES;
			$old_prices = mysql_fetch_assoc($result);
		}
		else { $old_prices = array('prc_new'=>-1,'prc_used'=>-1); }

		// change/insert the prices
		$vals = array();
		if (!$exists) { $vals[] = "prc_itemID=$itemID"; }
		if ($new != -1) { $vals[] = "prc_new=$new"; }
		if ($used != -1) { $vals[] = "prc_used=$used"; }

		$sql = ($exists?'UPDATE':'INSERT INTO').' prices SET '.implode(',',$vals).($exists?" WHERE prc_itemID=$itemID":'');
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		if ($return_changed) { return mysql_affected_rows(); }


		/*TODO stephanie R 8/1/16	always setting used to -1 on creation
		if ($new != $old_prices['prc_new']) { $this->store_price_change($itemID,ITEM_NEW,$old_prices['prc_new'],$new); }
		if ($used != $old_prices['prc_used']) { $this->store_price_change($itemID,ITEM_USED,$old_prices['prc_used'],$used); }
		*/
	}

	/**
	* Store a price change notice
	* @param integer $itemID
	* @param integer $newused
	* @param float $from_price
	* @param float $to_price
	* @param integer $storeID [optional, defaults to current storeID]
	*/
	function store_price_change($itemID,$newused,$from_price,$to_price,$storeID=-1)
	{
		global $db;

		if ($storeID == -1) { $storeID = $_SESSION['storeID']; }

		$sql = "INSERT INTO price_changes VALUES ($itemID,$storeID,$newused,$from_price,$to_price,'".time()."')";
		echo $sql;
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
	}

	/**
	* Return the date range of all price changes
	* @return string
	*/
	function getPriceChangeRange()
	{
		global $db;

		$sql = "SELECT MIN(pch_time) AS min,MAX(pch_time) AS max FROM price_changes";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		$row = mysql_fetch_assoc($result);

		return array(date('m/d/Y',$row['min']),date('m/d/Y',$row['max']));
	}

	/**
	* Pull in price changes
	* @param	integer	$newused	can be ITEM_NEW/ITEM_USED/BOTH
	* @param	integer	$platformID	platformID (0 for all)
	* @param	integer	$fromdate	from date (timestamp)
	* @param	integer	$todate		to date (timestamp) [optional, defaults to current time()]
	*/
	function setPriceChanges($newused,$platformID=0,$fromdate,$todate='')
	{
		global $db;

		if ($todate == '') { $todate = time(); }
		$nu_where = ($newused==ITEM_NEW || $newused==ITEM_USED ? "pch_newused=$newused AND " : '');
		$platform_where = ($platformID ? " AND pla_platformID=$platformID" : '');

		$this->changes = array();
		$sql = "SELECT * FROM price_changes,items,platforms WHERE {$nu_where}pch_time BETWEEN $fromdate AND $todate AND pch_itemID=itm_itemID AND itm_platformID=pla_platformID{$platform_where} ORDER BY pla_name,itm_title";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result))
		{
			$this->changes[] = $row;
		}
	}

	/**
	* Return all price changes
	* @return array
	*/
	function getPriceChanges()
	{
		return $this->changes;
	}

	/**
	* Given a price, if it's a round dollar, a penny is subtracted (IE: $4.00 = $3.99, $14.50 = $14.50)
	* @param integer $price
	* @return integer fixed price
	*/
	function fix_price($price)
	{
		// !!! THIS NEEDS TO BE REMOVED FROM ITEMS.INC !!!
		if ($price > 0 && sprintf('%0.0f',$price) == $price) { $price = ($price-0.01); }

		return $price;
	}

	/**
	* Apply a dollar/percent discount
	* @param	float	$price
	* @param	boolean	$do_percent
	* @param	float	$amount
	* @return	float
	* @access	public
	*/
	function applyDiscount($price,$do_percent,$amount)
	{
		if ($amount == 0)
		{
			return $price;
		}
		if ($do_percent)
		{
			return ($price * (1 - ($amount / 100)));
		}
		else
		{
			return ($price - $amount);
		}
	} // end function applyDiscount
} // end classes prices

?>