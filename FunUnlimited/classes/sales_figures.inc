<?php
/**
* Sales figures class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class sales_figures
{
	/**
	* page class reference
	* @var reference $pg
	*/
	var $pg;

	/**
	* Reference time (rather than using time(), use this)
	* @var integer $reference_time
	*/
	var $reference_time;

	/**
	* SQL command to get sales sum
	* @var string $getsum
	*/
	var $getsum;

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
	function sales_figures(&$pg)
	{
		$this->pg =& $pg;
		$this->reference_time = time();
		$this->getsum = '(SUM(inv_cash)+SUM(inv_check)+SUM(inv_charge)-SUM(inv_tax)) AS total';

		$this->error = new error('Sales Figures Class');
	}

	/**
	* Set the reference time
	* @param integer $time
	*/
	function setReferenceTime($time)
	{
		$this->reference_time = ($time==-1 ? time() : $time);
	}

	/**
	* Get the reference time
	* @return integer
	*/
	function getReferenceTime()
	{
		return $this->reference_time;
	}

	/**
	* Return sales figures for one day grouped by hour
	* @param integer $which 0 for today, -1 for average
	* @param string $type
	*/
	function getHourly($which,$type)
	{
		global $db;

		$typestring = $this->getTypeString($type);
		$total_string = $this->getTotalString($type);

		$time = strtotime(date('m/d/Y', $this->getReferenceTime()));

		$hourly = array();
		if ($which == 0)
		{
			// current day
			$sql = "
				SELECT
					(HOUR(FROM_UNIXTIME(inv_completedtime))+1) AS t_hour,
					$total_string AS total
				FROM invoices,invoice_items
				WHERE
					inv_completed=" . YES . " AND
					inv_invoiceID=ini_invoiceID AND
					$typestring AND
					DATE_FORMAT(FROM_UNIXTIME($time),'%c/%e/%Y')=DATE_FORMAT(FROM_UNIXTIME(inv_completedtime),'%c/%e/%Y') AND
					inv_customerID!=3176
				GROUP BY ini_invoiceID
				ORDER BY t_hour
			";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				@$hourly[$row['t_hour']] += $row['total'];
			}
		}
		else
		{
			// average
			$vals = array();
			$days = array();
			$sql = "
				SELECT
					DATE_FORMAT(FROM_UNIXTIME(inv_completedtime),'%c/%e/%Y') AS t_date,
					(HOUR(FROM_UNIXTIME(inv_completedtime))+1) AS t_hour,
					$total_string AS total
				FROM invoices,invoice_items
				WHERE
					inv_completed=" . YES . " AND
					inv_invoiceID=ini_invoiceID AND
					$typestring AND
					DAYOFWEEK(FROM_UNIXTIME(inv_completedtime))=DAYOFWEEK(FROM_UNIXTIME($time)) AND
					DATE_FORMAT(FROM_UNIXTIME($time),'%c/%e/%Y')!=DATE_FORMAT(FROM_UNIXTIME(inv_completedtime),'%c/%e/%Y') AND
					inv_customerID!=3176
				GROUP BY ini_invoiceID
				ORDER BY t_hour
			";
			$daytotals = array();
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				@$vals[$row['t_hour']] += $row['total'];
				$days[$row['t_date']] = YES;
				@$daytotals[$row['t_date']] += $row['total'];
			}

			// compute the averages
			while (list($hour,$total) = each($vals))
			{
				$hourly[$hour] = ($total / count($days));
			}
		}

		// fill in all incomplete hours with 0
		$this->putZeros($hourly,1,24);

		return $hourly;
	}

	/**
	* Return sales figures for one month grouped by day
	* @param integer $which 0 for this month, -1 for average
	* @param string $type
	*/
	function getDaily($which,$type)
	{
		global $db;

		$typestring = $this->getTypeString($type);
		$total_string = $this->getTotalString($type);

		$time = strtotime(date('m/d/Y', $this->getReferenceTime()));

		$daily = array();
		if ($which == 0)
		{
			$month = date('n',$time);
			$year = date('Y',$time);

			$sql = "
				SELECT
					DAYOFMONTH(FROM_UNIXTIME(inv_completedtime)) AS t_day,
					$total_string AS total
				FROM invoices,invoice_items
				WHERE
					inv_completed=" . YES . " AND
					inv_invoiceID=ini_invoiceID AND
					$typestring AND
					MONTH(FROM_UNIXTIME(inv_completedtime))=$month AND
					YEAR(FROM_UNIXTIME(inv_completedtime))=$year AND
					inv_customerID!=3176
				GROUP BY ini_invoiceID
				ORDER BY t_day
			";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				@$daily[$row['t_day']] += $row['total'];
			}
		}
		else
		{
			$vals = array();
			$days = array();
			$years = array();
			$totals = array();
			$sql = "
				SELECT
					YEAR(FROM_UNIXTIME(inv_completedtime)) AS t_year,
					DATE_FORMAT(FROM_UNIXTIME(inv_completedtime),'%c/%e/%Y') AS t_date,
					DAYOFMONTH(FROM_UNIXTIME(inv_completedtime)) AS t_day,
					$total_string AS total
				FROM invoices,invoice_items
				WHERE
					inv_completed=" . YES . " AND
					inv_invoiceID=ini_invoiceID AND
					$typestring AND
					MONTH(FROM_UNIXTIME(inv_completedtime))=MONTH(FROM_UNIXTIME($time)) AND
					YEAR(FROM_UNIXTIME(inv_completedtime))!=YEAR(FROM_UNIXTIME($time)) AND
					inv_customerID!=3176
				GROUP BY ini_invoiceID
				ORDER BY t_day
			";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				@$vals[$row['t_day']] += $row['total'];
				@$totals[$row['t_year']][$row['t_day']] += $row['total'];
				$days[$row['t_date']] = YES;
				$years[$row['t_year']] = YES;
			}

			// compute the averages
			while (list($day,$total) = each($vals))
			{
				// figure out how many times this day has been seen
				$div = 0;
				while (list($year,$a) = each($years))
				{
					if (isset($totals[$year][$day]))
					{
						$div++;
					}
				}
				reset($years);

				$daily[$day] = ($total / $div);
			}
		}

		// fill in all incomplete days with 0
		$this->putZeros($daily,1,31);

		return $daily;
	}

	/**
	* Return sales figures for one year grouped by month
	* @param integer $which 0 for this year, -1 for average
	* @param string $type
	*/
	function getMonthly($which,$type)
	{
		global $db;

		$typestring = $this->getTypeString($type);
		$total_string = $this->getTotalString($type);

		$time = strtotime(date('m/d/Y', $this->getReferenceTime()));

		$monthly = array();
		if ($which == 0)
		{
			$year = date('Y', $time);

			$sql = "
				SELECT
					MONTH(FROM_UNIXTIME(inv_completedtime)) AS t_month,
					$total_string AS total
				FROM invoices,invoice_items
				WHERE
					inv_completed=" . YES . " AND
					inv_invoiceID=ini_invoiceID AND
					$typestring AND
					YEAR(FROM_UNIXTIME(inv_completedtime))=$year AND
					inv_customerID!=3176
				GROUP BY ini_invoiceID
				ORDER BY t_month
			";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				@$monthly[$row['t_month']] += $row['total'];
			}
		}
		else
		{
			// get the average
			$vals = array();
			$years = array();
			$totals = array();
			$sql = "
				SELECT
					YEAR(FROM_UNIXTIME(inv_completedtime)) AS t_year,
					MONTH(FROM_UNIXTIME(inv_completedtime)) AS t_month,
					$total_string AS total
				FROM invoices,invoice_items
				WHERE
					inv_completed=" . YES . " AND
					inv_invoiceID=ini_invoiceID AND
					$typestring AND
					YEAR(FROM_UNIXTIME($time))!=YEAR(FROM_UNIXTIME(inv_completedtime)) AND
					inv_customerID!=3176
				GROUP BY ini_invoiceID
				ORDER BY t_month
			";

            #echo $sql;
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				@$vals[$row['t_month']] += $row['total'];
				@$totals[$row['t_year']][$row['t_month']] += $row['total'];
				$years[$row['t_year']] = YES;
			}

			// compute the averages
			while (list($month,$total) = each($vals))
			{
				// figure out how many times this month has been seen
				$div = 0;
				while (list($year,$a) = each($years))
				{
					if (isset($totals[$year][$month]))
					{
						$div++;
					}
				}
				reset($years);

				$monthly[$month] = ($total / $div);
			}
		}

		// fill in all incomplete months with 0
		$this->putZeros($monthly,1,12);

		return $monthly;
	}

	/**
	* Calculate the projected values for the incomplete periods at the end of the given data array
	* @param array $arr data array
	* @param array $avg_arr average data array
	* @return array(projected_values,crossover_point)
	*/
	function getProjected($arr,$avg_arr)
	{
		$data_start = 0;
		for ($zero_from=0; $zero_from<count($arr); $zero_from++)
		{
			if (!array_sum(array_slice($arr,0,$zero_from))) { $data_start = $zero_from; }
			if (!array_sum(array_slice($arr,$zero_from))) { break; }
		}
		$keys = array_keys($arr);

		if (!array_sum($arr)) { $projected = array_fill($keys[0],count($arr),0); } // fill projected values array with zeros
		else
		{
			$data = array_slice($arr,$data_start,(count($arr)-$zero_from));

			$count = count($data);
			if (!$count) { $count = 1; }

			$average = sprintf('%0.2f',(array_sum($data)/$count));
			$projected = $arr;

			$projected[$keys[($zero_from-1)]] = $arr[$keys[($zero_from-1)]];
			for ($i=$zero_from; $i<count($arr); $i++)
			{
				if ($avg_arr[$keys[$i]] > 0) { $projected[$keys[$i]] = $average; }
			}
		}

		return array($projected,$zero_from);
	}

	/**
	* Return the ini_type= string words
	* @return	array
	*/
	function getTypeWords()
	{
		return array(
			'sales'         => 'Sales',
			'trades_cash'   => 'Trades - Cash',
			'trades_credit' => 'Trades - Credit',
			'returns'       => 'Returns'
		);
	}

	/**
	* Return the ini_type= string
	* @param	string	$type
	* @return	string
	*/
	function getTypeString($type)
	{
		$sale = SALE;
		$trade = TRADE;
		$return = RETURNS;

		$cash = CASH;
		$credit = CREDIT;

		$type_strings = array(
			'sales'         => "ini_type=$sale",
			'trades_cash'   => "ini_type=$trade AND ini_trade_type=$cash",
			'trades_credit' => "ini_type=$trade AND ini_trade_type=$credit",
			'returns'       => "ini_type=$return"
		);

		return @$type_strings[$type];
	}

	function getTotalString($type)
	{
		// if the invoice has any cash applied, don't subtract applied credit
		$total_strings = array(
			//'sales'         => '(SUM(ini_price) - inv_tax - inv_credit - inv_cash_out)',
			'sales'         => '(SUM(ini_price) - inv_tax - IF(inv_cash>0, 0, inv_credit) - inv_cash_out)',
			'trades_cash'   => 'SUM(ini_price)',
			'trades_credit' => 'SUM(ini_price)',
			'returns'       => 'SUM(ini_price)'
		);

		return @$total_strings[$type];
	}

	/**
	* Populate unset array values with 0s and reorder array (reorder is pointless, but easier to read with print_r)
	* @param array &$array
	* @param integer $min
	* @param integer $max
	*/
	function putZeros(&$array,$min,$max)
	{
		$new_array = array();
		for ($i=1; $i<=$max; $i++)
		{
			if (!isset($array[$i])) { $array[$i] = 0; }
			$new_array[$i] = sprintf('%0.2f',$array[$i]);
		}
		$array = $new_array;
	}

	/**
	* Remove irrelevant entries from the beginning/end of each array (entries where both are 0)
	* The arrays must have the EXACT SAME keys in the beginning!!!
	* @param array &$arr1
	* @param array &$arr2
	* @param array &$arr3
	*/
	function removeIrrelevant(&$arr1,&$arr2,&$arr3)
	{
		$keys = array_keys($arr1);
		sort($keys);

		// beginning
		for ($i=0; $i<count($keys); $i++)
		{
			$key = $keys[$i];

			if (
				(!isset($arr1[$key]) || $arr1[$key] == 0)
				&&
				(!isset($arr2[$key]) || $arr2[$key] == 0)
				&&
				(!isset($arr3[$key]) || $arr3[$key] == 0)
			)
			{
				unset($arr1[$key]);
				unset($arr2[$key]);
				unset($arr3[$key]);
			}
			else { break; }
		}

		// end
		for ($i=(count($keys)-1); $i>=0; $i--)
		{
			$key = $keys[$i];

			if (
				(!isset($arr1[$key]) || $arr1[$key] == 0)
				&&
				(!isset($arr2[$key]) || $arr2[$key] == 0)
				&&
				(!isset($arr3[$key]) || $arr3[$key] == 0)
			)
			{
				unset($arr1[$key]);
				unset($arr2[$key]);
				unset($arr3[$key]);
			}
			else { break; }
		}
	}

	/**
	* Returns a formatted label (IE: hourly/18 returns '6PM')
	* @param string $which which data set?
	* @param integer $num
	* @return string
	*/
	function formatLabel($which,$num)
	{
		if (!$num) { return; }

		if ($which == 'hourly')
		{
			if ($num == 12) { return '12PM'; }
			elseif ($num == 24) { return '12AM'; }
			else { return ($num % 12) . ($num>12 ? 'PM' : 'AM'); }
		}
		elseif ($which == 'daily')
		{
			return $num;
		}
		elseif ($which == 'monthly')
		{
			$months = array('Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec');
			return $months[($num - 1)];
		}
	}
}

?>