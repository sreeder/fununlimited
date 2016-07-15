<?php
/**
* Item searching (forms and searching)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class item_search extends item_search_results
{
	/**
	* page class reference
	* @var reference $pg
	*/
	var $pg;

	/**
	* Page action
	* @var string $act
	*/
	var $act;

	/**
	* Form action
	* @var string $action
	*/
	var $action;
	/**
	* Array of search results
	* @var array $results
	*/
	var $results;

	/**
	* Search criteria
	* @var array $criteria
	*/
	var $criteria;

	/**
	* Current page
	* @var integer $page
	*/
	var $page;

	/**
	* Number of items to display on each page
	* @var integer $per_page
	*/
	var $per_page;

	/**
	* Maximum results (defaults to MAX_ITEM_SEARCH_RESULTS)
	* @var integer $max_results
	*/
	var $max_results;

	/**
	* UPC in question for an item-match search
	* @var string $findingmatch
	*/
	var $findingmatch;

	/**
	* Is it an online store search?
	* @var integer $online
	*/
	var $online;

	/**
	* Search type (online store)
	* @var integer $search_type
	*/
	var $search_type;

	/**
	* 'Narrow by' values (online store)
	* @var array $narrow
	*/
	var $narrow;

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
	function item_search(&$pg)
	{
		$this->timer =& $GLOBALS['timer_class'];

		$this->pg =& $pg;

		$this->setAction('items.php');
		$this->setResults(array());
		$this->act = 'search';
		$this->per_page = (ONADMIN ? 25 : 10);
		$this->max_results = (ONADMIN ? MAX_ITEM_SEARCH_RESULTS : MAX_ITEM_SEARCH_RESULTS_STORE);

		$this->results_act = 'select';

		$this->setOnline(NO);

		$this->error = new error('Item Search');
	}

	/**
	* Output the search form header
	* @param array $hidden hidden form values (format: 'key'=>'value') [optional, default array()]
	* @param string $method form method
	*/
	function form_head($hidden=array(),$method='POST')
	{
		$this->pg->outlineTableHead();
		?>
		<form method="<?=$method;?>" action="<?=$this->getAction();?>" name="itmsearch">
		<input type="hidden" name="act" value="<?php echo $this->act;?>">
		<?php
		while (list($key,$val) = each($hidden)) { ?><input type="hidden" name="<?=$key;?>" value="<?=$val;?>"><?php }
	}

	/**
	* Output the search form footer (buttons)
	* @param string $focus Form field to focus on [optional, default '']
	*/
	function form_foot($focus='')
	{
		$this->pg->outlineTableFoot();
		?>
		<p />
		<input type="submit" value="Search &gt;" class="btn">
		</form>
		<?php

		if (strlen($focus))
		{
			$this->pg->addOnload("document.itmsearch.{$focus}.focus()");
		}
	}

	/**
	* Form item: UPC box
	*/
	function upc()
	{
		?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>UPC:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell2');?>">
				<input type="text" name="upc" size="25" value="<?=@$this->criteria['upc'];?>">
			</td>
		</tr>
		<?php
	}

	/**
	* Form item: Title box
	*/
	function title()
	{
		?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Title:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell2');?>"><input type="text" name="title" size="35" value="<?=@$this->criteria['title'];?>"></td>
		</tr>
		<?php
	}

	/**
	* Form item: Description box
	*/
	function description()
	{
		?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Description:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell2');?>"><input type="text" name="description" size="20" value="<?=@$this->criteria['description'];?>"></td>
		</tr>
		<?php
	}

	/**
	* Form item: Year selection
	*/
	function year()
	{
		?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Year:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell2');?>">
				<select name="yearID" size="1" style="vertical-align:middle"><option value=""></option><?php
					$yr = new years();
					while (list($a,$arr) = each($yr->years))
					{
						?><option value="<?=$arr['yer_yearID'];?>"><?=$arr['yer_year'];?></option><?php
					}
				?></select>
			</td>
		</tr>
		<?php
	}

	/**
	* Form item: Platform selection
	*/
	function platform()
	{
		?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Platform:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell2');?>">
				<select name="platformID" size="1" style="vertical-align:middle"><option value=""></option><?php
					$pla = new platforms($this->pg,0);
					$pla->set_item('platforms');
					while (list($a,$arr) = each($pla->values))
					{
						if ($arr[0] == @$this->criteria['platformID']) { $s = ' selected="selected"'; } else { $s = ''; }
						?><option value="<?=$arr[0];?>"<?=$s;?>><?=$arr[1];?></option><?php
					}
				?></select>
			</td>
		</tr>
		<?php
	}

	/**
	* Form item: Active radio
	*/
	function active()
	{
		?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Active:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell2');?>">
				<input type="radio" name="active" value="<?=YES;?>" checked="checked" id="activey" style="vertical-align:middle;border-width:0px"> <label style="cursor:hand" for="activey">YES</label>
				<input type="radio" name="active" value="<?=NO;?>" id="activen" style="vertical-align:middle;border-width:0px"> <label style="cursor:hand" for="activen">No</label>
			</td>
		</tr>
		<?php
	}

	/**
	* Form item: Items per-page selection
	*/
	function perpage()
	{
		?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Misc:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell2');?>">
				Items per page: <select name="per_page" size="1" style="vertical-align:middle"><?php
					$options = array(5,10,25,50);
					$sel = (ONADMIN ? 25 : 10);
					while (list($a,$val) = each($options))
					{
						if ($val == $sel) { $s = ' selected="selected"'; } else { $s = ''; }
						?><option value="<?=$val;?>"<?=$s;?>><?=$val;?></option><?php
					}
				?></select>
			</td>
		</tr>
		<?php
	}

	/**
	* Form item: New/Used
	* @param integer $default
	*/
	function newused($default=ITEM_NEW)
	{
		if ((!isset($this->criteria['newused']) && $default == ITEM_NEW) || @$this->criteria['newused'] == ITEM_NEW) { $c1 = ' checked="checked"'; $c2 = ''; $c3 = ''; }
		elseif ((!isset($this->criteria['newused']) && $default == ITEM_USED) || $this->criteria['newused'] == ITEM_USED) { $c1 = ''; $c2 = ' checked="checked"'; $c3 = ''; }
		else { $c1 = ''; $c2 = ' checked="checked"'; $c3 = ''; }
		?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>New/Used:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell2');?>">
				<input type="radio" name="newused" value="<?=ITEM_NEW;?>" class="nb" id="nu_new"<?=$c1;?>> <label for="nu_new">New</label>
				<input type="radio" name="newused" value="<?=ITEM_USED;?>" class="nb" id="nu_used"<?=$c2;?>> <label for="nu_used">Used</label>
				<input type="radio" name="newused" value="<?=EITHER;?>" class="nb" id="nu_either"<?=$c3;?>> <label for="nu_either">Either</label>
			</td>
		</tr>
		<?php
	}

	/**
	* Displays the normal search form
	* @param integer $showupc Show the UPC criteria box? [optional, default YES]
	* @param array $hidden Array of hidden values [optional, default array()]
	*/
	function form($upc=YES,$hidden=array(),$activeandpp=YES)
	{
		// output the form/table
		$hidden = array_merge(array('findingmatch'=>$this->findingmatch),$hidden);
		$this->form_head($hidden);

		if ($upc) { $this->upc(); }
		$this->title();
		$this->description();
		$this->year();
		$this->platform();
		if ($activeandpp)
		{
			$this->active();
			$this->perpage();
		}

		$this->form_foot(($upc==YES?'upc':'title'));
	}

	/**
	* Displays the online store's advanced search form
	*/
	function advanced_form()
	{
		$this->form_head(array('search_type'=>ITEM_SEARCH_ADVANCED),'GET');
		$this->title();
		//$this->description();
		$this->year();
		$this->platform();
		$this->form_foot();
	}

	/**
	* Parses $_POST/$_GET
	*/
	function pull_post()
	{
		$this->act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

		$this->criteria = array_merge($_GET,$_POST);
		if (!is_array($this->criteria)) { $this->criteria = array(); }
		if (isset($this->criteria['criteria']))
		{
			$this->criteria = array_merge($this->criteria,$this->criteria['criteria']);
		}

		$this->search_type = @$_GET['search_type'];

		$narrow = array(
			'platform'   => '',
			'startswith' => '',
			'type'       => '',
			'company'    => '',
			'title'      => ''
		);
		while (list($key,$v) = each($narrow))
		{
			$narrow[$key] = @$_GET["narrow_{$key}"];
		}
		$this->setNarrow($narrow);

		if (isset($_POST['per_page']) || isset($_GET['per_page']))
		{
			$this->per_page = getGP('per_page',@$_SESSION['search_per_page']);
			if (!ONADMIN) { $this->per_page = 10; }
			$_SESSION['search_per_page'] = $this->per_page;
		}

		while (list($k,$v) = each($this->criteria))
		{
			if (!is_array($v) && !strlen(trim($v))) { unset($this->criteria[$k]); }
		}
	}

	/**
	* Performs the item search according to the criteria in $this->criteria
	* @param string $orderby SQL 'order by' value [optional, default 'itm_title']
	* @param string $sqlfunc user function to call before executing SQL [optional, default '']
	* @param integer $page Page number of results [optional, default '']
	* @return array search results
	*/
	function search($orderby='itm_title',$sqlfunc='',$page='')
	{
		global $db;

		$this->timer->startTimer(__CLASS__.'::'.__FUNCTION__);
		$online = $this->getOnline();

		$results = array();
		$companies = array(0=>'&nbsp;'); // format: $companies[$companyID] = name
		$exact_itemIDs = array();
		$all_itemIDs = array();

		//if (!$online) { $qtysel = "qty_new,qty_used,"; }
		//else { $qtysel = "SUM(qty_new) AS qty_new,SUM(qty_used) AS qty_used,"; }

		$search_types = array('exact','fuzzy');
		while (list($a,$type) = each($search_types))
		{
			$this->timer->startTimer("search_$type");
			$sql = 'SELECT * FROM items,years,platforms,types WHERE ';

			$this->timer->startTimer("search_$type-build_query");
			$where = array(
				'itm_yearID=yer_yearID',
				'itm_platformID=pla_platformID',
				'itm_typeID=typ_typeID',
			);

			//if (!$online) { $where[] = "qty_storeID={$_SESSION['storeID']}"; }

			if (isset($this->criteria['itemIDs']))
			{
				$where[] = 'itm_itemID IN ' . getIn($this->criteria['itemIDs']);
			}
			if (strlen(@$this->criteria['upctitle']))
			{
				if ($type == 'exact')
				{
					// pull the exact matches
					$where[] = "(itm_upc='" . mysql_real_escape_string($this->format_title_search($this->criteria['upctitle'])) . "' OR (itm_title_search='" . mysql_real_escape_string($this->format_title_search($this->criteria['upctitle'])) . "'))";
				}
				else
				{
					// pull the fuzzy matches
					$exp = explode(' ',$this->criteria['upctitle']);
					while (list($a,$word) = each($exp))
					{
						$where[] = "(itm_upc LIKE '" . mysql_real_escape_string($this->format_title_search($word)) . "%' OR (itm_title_search LIKE '%" . mysql_real_escape_string($this->format_title_search($word)) . "%'))";
					}
				}
			}
			if (strlen(@$this->criteria['upc']))
			{
				$where[] = "itm_upc LIKE '" . mysql_real_escape_string($this->criteria['upc']) . "%'";
			}
			if (strlen(@$this->criteria['title']))
			{
				$exp = explode(' ',$this->criteria['title']);
				while (list($a,$word) = each($exp))
				{
					$where[] = "itm_title_search LIKE '%" . mysql_real_escape_string($this->format_title_search($word)) . "%'";
				}
			}
			if (strlen(@$this->criteria['description'])) { $where[] = "itm_description LIKE '" . mysql_real_escape_string($this->criteria['description']) . "%'"; }
			if (strlen(@$this->criteria['yearID'])) { $where[] = "itm_yearID={$this->criteria['yearID']}"; }
			if (strlen(@$this->criteria['platformID'])) { $where[] = "itm_platformID={$this->criteria['platformID']}"; }

			if (!strlen(@$this->criteria['active'])) { $this->criteria['active'] = YES; }
			$where[] = "itm_active=" . $this->criteria['active'];

			if ($online)
			{
				// online store criteria searches

				if (strlen(@$this->criteria['platform'])) { $where[] = "pla_name LIKE '%" . mysql_real_escape_string($this->criteria['platform']) . "%'"; }
				if (strlen(@$this->criteria['company']))
				{
					// find company IDs that match the criteria
					$companyIDs = array(-1);
					$csql = "SELECT com_companyID FROM companies WHERE com_name LIKE '%" . mysql_real_escape_string($this->criteria['company']) . "%'";
					$cresult = mysql_query($csql,$db);
					$this->error->mysql(__FILE__,__LINE__);
					while ($crow = mysql_fetch_assoc($cresult)) { $companyIDs[] = $crow['com_companyID']; }

					$where[] = "(itm_company1ID IN (" . implode(',',$companyIDs) . ") OR itm_company2ID IN (" . implode(',',$companyIDs) . "))";
				}
				if (strlen(@$this->criteria['type'])) { $where[] = "typ_type LIKE '%" . mysql_real_escape_string($this->criteria['type']) . "%'"; }
				if (strlen(@$this->criteria['age']))
				{
					$age = $this->criteria['age'];

					if (is_numeric($age)) { $where[] = "itm_age='$age'"; }
					elseif (substr($age,-1) == '+' && is_numeric(substr($age,0,-1))) { $where[] = "itm_age>=" . substr($age,0,-1); }
					else
					{
						$exp = explode('-',$age);
						if (count($exp) == 2 && is_numeric($exp[0]) && is_numeric($exp[1]))
						{
							$l = $exp[0]; $h = $exp[1];
							$where[] = "(itm_age>=$l && itm_age<=$h)";
						}
						else
						{
							// invalid range format; include a condition that will never be true (this will always return 0 results)
							$where[] = "itm_age<-999999";
						}
					}
				}
				if (strlen(@$this->criteria['year'])) { $where[] = "yer_year='{$this->criteria['year']}'"; }
			}

			if ($type == 'fuzzy' && count($exact_itemIDs)) { $where[] = 'itm_itemID NOT IN (' . implode(',',$exact_itemIDs) . ')'; }

			$sql .= implode(' AND ',$where);
			//if ($online) { $sql .= ' GROUP BY qty_itemID'; } // totals the quantities for all stores
			$sql .= " ORDER BY $orderby";

			if (false && is_integer($page)) { $sql .= ' LIMIT ' . (($page-1)*$this->max_results) . ",$this->max_results"; }
			else { $sql .= " LIMIT $this->max_results"; }

			// call the user-defined function $sqlfunc if needed
			if (strlen($sqlfunc) && function_exists($sqlfunc)) { $sql = call_user_func($sqlfunc,$sql); }

			$this->timer->stopTimer("search_$type-build_query");

			//echo "<p />$sql<p />";
			$this->timer->startTimer("search_$type-execute_query");
			$itemIDs = array();
			$companyIDs = array();
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				$itemIDs[] = $row['itm_itemID'];
				$all_itemIDs[] = $row['itm_itemID'];
				$companyIDs[] = $row['itm_company1ID'];
				$companyIDs[] = $row['itm_company2ID'];

				$row['com1_name'] = '&nbsp;'; // updated below
				$row['com2_name'] = '&nbsp;'; // updated below
				$row['qty_new'] = 0; // updated below
				$row['qty_used'] = 0; // updated below
				$row['prc_new'] = 0; // updated below
				$row['prc_used'] = 0; // updated below

				$results[] = $row;
			}
			$this->timer->stopTimer("search_$type-execute_query");

			// pull the companies
			$this->timer->startTimer("search_$type-pull_companies");
			$companyIDs = array_unique($companyIDs);
			$sql = 'SELECT * FROM companies WHERE com_companyID IN ' . getIn($companyIDs);
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				$companies[$row['com_companyID']] = $row['com_name'];
			}

			// set the companies in the items array
			foreach ($results as $a => $arr)
			{
				if ($arr['itm_company1ID']) { $results[$a]['com1_name'] = $companies[$arr['itm_company1ID']]; }
				if ($arr['itm_company2ID']) { $results[$a]['com2_name'] = $companies[$arr['itm_company2ID']]; }
			}
			$this->timer->stopTimer("search_$type-pull_companies");

			if ($type == 'exact') { $exact_itemIDs = $itemIDs; }

			if (count($itemIDs))
			{
				$this->timer->startTimer("search_$type-get_qty_pricing");
				// get quantities/prices
				$pqwhere = array();
				if ($online) { $pqwhere[] = "qty_storeID={$_SESSION['storeID']}"; }
				else { $pqwhere[] = 'qty_storeID=1'; }
				$pqwhere[] = 'qty_itemID IN (' . implode(',',$itemIDs) . ')';
				$pqwhere[] = 'qty_itemID=prc_itemID';

				if ($online)
				{
					$pqwhere[] = "((prc_new>0 AND qty_new>0) OR (prc_used>0 AND qty_used>0))";

					if (strlen(@$this->criteria['lessprice'])) { $pqwhere[] = "((prc_new>0 && prc_new<={$this->criteria['lessprice']}) OR (prc_used>0 && prc_used<={$this->criteria['lessprice']}))"; }
					if (strlen(@$this->criteria['moreprice'])) { $pqwhere[] = "((prc_new>0 && prc_new>={$this->criteria['moreprice']}) OR (prc_used>0 && prc_used>={$this->criteria['moreprice']}))"; }
					if (strlen(@$this->criteria['pricerange']))
					{
						$exp = explode('-',$this->criteria['pricerange']);
						if (count($exp) == 2 && is_numeric($exp[0]) && is_numeric($exp[1]))
						{
							$l = $exp[0]; // low
							$h = $exp[1]; // high
							$pqwhere[] = "((prc_new>0 && prc_new>=$l && prc_new<=$h) OR (prc_used>0 && prc_used>=$l && prc_used<=$h))";
						}
						else
						{
							// invalid range format; include a condition that will never be true (this will always return 0 results)
							$pqwhere[] = "itm_itemID IS NULL";
						}
					}
				}

				$sql = 'SELECT qty_itemID,qty_new,qty_used,prc_new,prc_used FROM quantity,prices WHERE ' . implode(' AND ',$pqwhere);
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);

				$pq_itemIDs = array();
				while ($row = mysql_fetch_assoc($result))
				{
					$pq_itemIDs[] = $row['qty_itemID'];
					$idx = array_search($row['qty_itemID'],$all_itemIDs);
					$results[$idx]['qty_new'] = $row['qty_new'];
					$results[$idx]['qty_used'] = $row['qty_used'];
					$results[$idx]['prc_new'] = $row['prc_new'];
					$results[$idx]['prc_used'] = $row['prc_used'];
				}

				if ($online)
				{
					// remove items that weren't pulled from the qty/price tables
					$remove_itemIDs = array_diff($all_itemIDs,$pq_itemIDs);
					foreach ($remove_itemIDs as $itemID)
					{
						$idx = array_search($itemID,$all_itemIDs);
						unset($results[$idx]);
					}
					array_merge($results);
				}
				$this->timer->stopTimer("search_$type-get_qty_pricing");
			}
			$this->timer->stopTimer("search_$type");
		}

		$this->setResults($results);

		$_SESSION['search_criteria'] = $this->criteria;
		$_SESSION['search_results'] = $results;
		$_SESSION['search_count'] = count($results);

		$this->timer->stopTimer(__CLASS__.'::'.__FUNCTION__);

		return $results;
	}

	/**
	* Take a searched title value and remove everything but letters/numbers
	* @param string $title
	* @return string formatted title
	*/
	function format_title_search($title)
	{
		return preg_replace('/[^a-zA-Z0-9]/','',$title);
	}

	/**
	* Set the online store search status
	* @param boolean $online
	*/
	function setOnline($online) { $this->online = $online; }

	/**
	* Return the online store search status
	* @return boolean
	*/
	function getOnline() { return $this->online; }

	/**
	* Set the form action
	* @param string $action
	*/
	function setAction($action) { $this->action = $action; }

	/**
	* Return the form action
	* @return string
	*/
	function getAction() { return $this->action; }

	/**
	* Set the narrow criteria
	* @param array $narrow
	*/
	function setNarrow($narrow) { $this->narrow = $narrow; }

	/**
	* Return the narrow criteria
	* @return array
	*/
	function getNarrow() { return $this->narrow; }

	/**
	* Set the search criteria
	* @param array $criteria
	*/
	function setCriteria($criteria) { $this->criteria = $criteria; }

	/**
	* Return the search criteria
	* @return array
	*/
	function getCriteria() { return $this->criteria; }

	/**
	* Set the search results
	* @param array $results
	*/
	function setResults($results) { $this->results = $results; }

	/**
	* Return the search results
	* @return array
	*/
	function getResults() { return $this->results; }
}

?>