<?php
/**
* Rankings class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class rankings
{
	/**
	* page class reference
	* @var reference $pg
	*/
	var $pg;

	/**
	* Array of rankings
	* @var array $rankings
	*/
	var $rankings;

	/**
	* Ranking types
	* @var array $types
	*/
	var $types;

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
	function rankings(&$pg)
	{
		$this->pg =& $pg;
		$this->rankings = array();
		$this->types = array('sales','cashtrades','credittrades','returns');

		$this->error = new error('Rankings');
	}

	/**
	* Update rankings table
	* @param	integer	$use_year	year to use for YTD [optional, defaults to current year]
	* @access	public
	*/
	function updateRankings($use_year=0, $storeID=0)
	{
		global $db;

		// determine which year to use for YTD
		// if today is before March 6 of this year, use last year. otherwise, this year
		if (!$use_year)
		{
			$use_year = (time()<mktime(0,0,0,3,6,date('Y')) ? (date('Y') - 1) : date('Y'));
		}

		set_time_limit(0);
		$prefixes = array(
			''     => array(
				0,
				time()
			),
			'ytd_' => array(
				mktime(0,0,0,3,6, $use_year), // the anniversary of using this software (started 3/6/2004)
				mktime(0,0,0,3,6,($use_year + 1))
			)
		);
		$ranks = array();

		$storeID = ($storeID ? $storeID : $_SESSION['storeID']);

		// delete rankings
		$sql = "DELETE FROM rankings WHERE rnk_storeID=$storeID AND rnk_archived=" . NO;
		mysql_query($sql, $db);
		$this->error->mysql(__FILE__,__LINE__);

		// loop through the prefixes/date ranges and build the rankings
		while (list($prefix,list($from, $to)) = each($prefixes))
		{
			$customerIDs = array();
			$sql = "SELECT DISTINCT inv_customerID FROM invoices WHERE inv_storeID=$storeID AND inv_completedtime BETWEEN $from AND $to AND inv_customerID>0 ORDER BY inv_customerID";
			$result = mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				$customerIDs[] = $row['inv_customerID'];
				if (!isset($ranks[$row['inv_customerID']]))
				{
					$ranks[$row['inv_customerID']] = array(
						'sa'      => 0,
						'sav'     => 0,
						'ca'      => 0,
						'cav'     => 0,
						'cr'      => 0,
						'crv'     => 0,
						'rr'      => 0,
						'rv'      => 0,

						'ytd_sa'  => 0,
						'ytd_sav' => 0,
						'ytd_ca'  => 0,
						'ytd_cav' => 0,
						'ytd_cr'  => 0,
						'ytd_crv' => 0,
						'ytd_rr'  => 0,
						'ytd_rv'  => 0
					);
				} // if customer not yet seen
			} // each row

			// sales
			$sale_customerIDs = $customerIDs;
			$sql = "SELECT inv_customerID,(SUM(inv_cash)+SUM(inv_check)+SUM(inv_charge)-SUM(inv_tax)) AS total FROM invoices WHERE inv_storeID=$storeID AND inv_completedtime BETWEEN $from AND $to AND inv_customerID>0 AND inv_completed= " . YES . " GROUP BY inv_customerID ORDER BY total DESC";
			$result = mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				$ranks[$row['inv_customerID']][$prefix . 'sav'] = $row['total'];
			}

			// cash trades
			$cash_customerIDs = $customerIDs;
			$sql = "SELECT inv_customerID,SUM(ini_price) AS total FROM invoices,invoice_items WHERE inv_storeID=$storeID AND inv_completedtime BETWEEN $from AND $to AND inv_customerID>0 AND inv_completed=" . YES . " AND inv_invoiceID=ini_invoiceID AND ini_type=".TRADE." AND ini_trade_type=".CASH." GROUP BY inv_customerID ORDER BY total DESC";
			$result = mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				$ranks[$row['inv_customerID']][$prefix . 'cav'] = $row['total'];
			}

			// credit trades
			$credit_customerIDs = $customerIDs;
			$sql = "SELECT inv_customerID,SUM(ini_price) AS total FROM invoices,invoice_items WHERE inv_storeID=$storeID AND inv_completedtime BETWEEN $from AND $to AND inv_customerID>0 AND inv_completed=" . YES . " AND inv_invoiceID=ini_invoiceID AND ini_type=".TRADE." AND ini_trade_type=".CREDIT." GROUP BY inv_customerID ORDER BY total DESC";
			$result = mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				$ranks[$row['inv_customerID']][$prefix . 'crv'] = $row['total'];
			}

			// returns
			$return_customerIDs = $customerIDs;
			$sql = "SELECT inv_customerID,SUM(ini_price) AS total FROM invoices,invoice_items WHERE inv_storeID=$storeID AND inv_completedtime BETWEEN $from AND $to AND inv_customerID>0 AND inv_completed=" . YES . " AND inv_invoiceID=ini_invoiceID AND ini_type=".RETURNS." GROUP BY inv_customerID ORDER BY total DESC";
			$result = mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				$ranks[$row['inv_customerID']][$prefix . 'rv'] = $row['total'];

				// subtract the returns from their sales totals
				//$ranks[$row['inv_customerID']][$prefix . 'sav'] -= $row['total'];
			}
		} // each prefix/date range
		reset($prefixes);

		// loop through each prefix/date range again and determine the rankings
		// suffixes to the value field => rank field
		$suffixes = array(
			'sav' => 'sa',
			'cav' => 'ca',
			'crv' => 'cr',
			'rv'  => 'rr'
		);
		while (list($prefix,list($from, $to)) = each($prefixes))
		{
			while (list($key_value, $key_rank) = each($suffixes))
			{
				// extract the values for this prefix->value
				$totals = array();
				while (list($customerID, $arr) = each($ranks))
				{
					$totals[$customerID] = $arr[$prefix . $key_value];
				} // each customer/rank
				reset($ranks);

				// reverse sort (high to low)
				arsort($totals);

				// determine the ranks
				$current_rank = 0;
				$last_total = 0;
				$current_row = 0;

				while (list($customerID, $total) = each($totals))
				{
					$current_row++;

					if ($total != $last_total)
					{
						$current_rank = $current_row;
						$last_total = $total;
					}
					$ranks[$customerID][$prefix . $key_rank] = $current_rank;
				} // each customer->total
			} // each suffix
			reset($suffixes);
		} // each prefix/date range

		// insert the rankings into the database
		$vals = array();
		while (list($customerID, $arr) = each($ranks))
		{
			while (list($key, $val) = each($arr))
			{
				if (!strlen($val)) { $arr[$key] = 0; }
			}

			$vals[] = '(' . implode(',',array(
				'rnk_customerID'             => $customerID,
				'rnk_storeID'                => $storeID,

				'rnk_sales'                  => $arr['sa'],
				'rnk_sales_value'            => $arr['sav'],
				'rnk_cashtrades'             => $arr['ca'],
				'rnk_cashtrades_value'       => $arr['cav'],
				'rnk_credittrades'           => $arr['cr'],
				'rnk_credittrades_value'     => $arr['crv'],
				'rnk_returns'                => $arr['rr'],
				'rnk_returns_value'          => $arr['rv'],

				'rnk_ytd_sales'              => $arr['ytd_sa'],
				'rnk_ytd_sales_value'        => $arr['ytd_sav'],
				'rnk_ytd_cashtrades'         => $arr['ytd_ca'],
				'rnk_ytd_cashtrades_value'   => $arr['ytd_cav'],
				'rnk_ytd_credittrades'       => $arr['ytd_cr'],
				'rnk_ytd_credittrades_value' => $arr['ytd_crv'],
				'rnk_ytd_returns'            => $arr['ytd_rr'],
				'rnk_ytd_returns_value'      => $arr['ytd_rv'],
				'rnk_archived'               => NO
			)) . ')';
		} // each customer->rank

		$chunks = array_chunk($vals,10000);
		while (list($a, $chunk) = each($chunks))
		{
			$sql = 'INSERT INTO rankings VALUES ' . implode(',', $chunk);
			mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);
		}
	} // end function updateRankings

	/**
	* Obtain all rankings for the given type
	* @param	string	$type
	* @param	boolean	$ytd		get the YTD rankings? [optional, default NO]
	* @param	integer	$archived	return the archived rankings of this year [optional, default 0]
	* @param	integer	$customerID	only this customerID [optional, default 0]
	*/
	function setRankings($type, $ytd=NO, $archived=0, $customerID=0)
	{
		global $db;

		$this->rankings = array();
		if (!$archived) { $archived = 0; }

		if (!in_array($type, $this->types))
		{
			$this->pg->error("Invalid rank type: $type");
		}
		else
		{
			if ($ytd)
			{
				$fields = array("rnk_ytd_{$type}", "rnk_ytd_{$type}_value");
			}
			else
			{
				$fields = array("rnk_{$type}", "rnk_{$type}_value");
			}
			$sql = "
				SELECT
					*
				FROM
					rankings,customers
				WHERE
					rnk_storeID={$_SESSION['storeID']}
					AND {$fields[0]}>0
					AND {$fields[1]}!=0
					AND rnk_customerID=cus_customerID
					" . ($customerID ? " AND cus_customerID=$customerID" : '') . "
					AND rnk_archived=$archived
				ORDER BY
					" . implode(',', $fields) . ' DESC'
			;
			$result = mysql_query($sql, $db);
			$this->error->mysql(__FILE__,__LINE__);

			while ($row = mysql_fetch_assoc($result))
			{
				$this->rankings[$row['cus_customerID']] = $row;
			}
		}
	}

	/**
	* Return all rankings
	*/
	function getRankings($customerID=0)
	{
		return ($customerID ? @$this->rankings[$customerID] : $this->rankings);
	}

	/**
	* Gets all rankings for the given customer
	* @param integer $customerID
	* @return array
	*/
	function getCustomerRankings($customerID)
	{
		global $db;

		$defaults = array();
		$rank_select = array();
		$max_select = array();
		while (list($a, $type) = each($this->types))
		{
			$defaults[] = 0;
			$rank_select[] = "rnk_$type";
			$max_select[] = "MAX(rnk_$type) AS rnk_$type";

			$defaults[] = 0;
			$rank_select[] = "rnk_ytd_$type";
			$max_select[] = "MAX(rnk_ytd_$type) AS rnk_ytd_$type";
		}

		// get the customer rankings
		$sql = "SELECT ".implode(',', $rank_select)." FROM rankings WHERE rnk_storeID={$_SESSION['storeID']} AND rnk_customerID=$customerID";
		$result = mysql_query($sql, $db);
		$this->error->mysql(__FILE__,__LINE__);
		if (!mysql_num_rows($result)) { $ranks = array_combine($rank_select, $defaults); }
		else { $ranks = mysql_fetch_assoc($result); }

		// get the maximum rankings
		$sql = "SELECT ".implode(',', $max_select)." FROM rankings WHERE rnk_storeID={$_SESSION['storeID']}";
		$result = mysql_query($sql, $db);
		$this->error->mysql(__FILE__,__LINE__);
		$row = mysql_fetch_assoc($result);

		// combine into 'is/of'
		$return = array();
		while (list($key, $val) = each($ranks))
		{
			if ($row[$key]) { $of = $row[$key]; }
			else { $of = 0; }
			$return[$key] = "$val/$of";
		}

		return $return;
	}

	/**
	* Archive the current non-archived rankings with the given year
	* @param	integer	$year
	* @access	public
	*/
	function archiveRankings($year)
	{
		global $db;

		if (!$year) { return; }

		// update the rankings, using the year as the reference
		$this->updateRankings($year);

		// set all non-archived rankings to the current year
		$sql = "UPDATE rankings SET rnk_archived=$year WHERE rnk_storeID={$_SESSION['storeID']} AND rnk_archived=" . NO;
		mysql_query($sql, $db);
		$this->error->mysql(__FILE__,__LINE__);

		// re-update the rankings
		$this->updateRankings();
	}

	/**
	* Returns YES/NO if the customer is in the top 500 for the current year
	* @param	integer	$customerID
	* @return	integer	NO if not in the top 500, the rank if they are
	* @access	public
	*/
	function inTop500($customerID)
	{
		if (@$_SESSION['in500_last_customerID'] == $customerID)
		{
			return $_SESSION['in500_last_rank'];
		}
		else
		{
			$this->setRankings('sales', NO, 0, $customerID);
			$rankings = $this->getRankings($customerID);
			$rank = @$rankings['rnk_sales'];

			if ($rank <= 500)
			{
				$return = $rank;
			}
			else
			{
				$return = NO;
			}

			$_SESSION['in500_last_customerID'] = $customerID;
			$_SESSION['in500_last_rank'] = $return;

			return $return;
		}
	} // end function inTop500
} // end class rankings

?>