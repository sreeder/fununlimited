<?php
/**
* Store to store inventory movement class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class inventory_movement
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
	* Current requestID
	* @var integer $requestID
	*/
	var $requestID;

	/**
	* Request stores (format: array(fromstoreID,tostoreID)
	* @var array $stores
	*/
	var $stores;

	/**
	* Request information
	* @var array $info
	*/
	var $info;

	/**
	* Request quantity options
	* @var array $qty_options
	*/
	var $qty_options;

	/**
	* Item list
	* @var array $items
	*/
	var $items;

	/**
	* Selected items
	* @var array $selitems
	*/
	var $selitems;

	/**
	* All requests
	* @var array $requests
	*/
	var $requests;

  /**
  * Request status
  * @var integer $status
  */
  var $status;

	/**
	* Request status words
	* @var array $status_words
	*/
	var $status_words;

	/**
	* Prices for selected items
	* @var array $prices
	*/
	var $prices;

	/**
	* Discounts
	* @var array $discounts
	*/
	var $discounts;

	/**
	* Selected discounts
	* @var array $seldiscounts
	*/
	var $seldiscounts;

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	*/
	function inventory_movement(&$pg)
	{
		$this->pg =& $pg;
		$this->requestID = @$_SESSION['requestID'];
		$this->status = -1;

		$this->status_words = array(
			MOV_INCOMPLETE=>'Request incomplete',
			MOV_COMPLETE=>'Request complete',
			MOV_REQUEST_SENT.MOV_FROM_STORE=>'Inventory list sent<br />Awaiting reply',
			MOV_REQUEST_SENT.MOV_TO_STORE=>'Inventory list received<br />Awaiting response<br /><font color="red"><b>Please reply!</b></font>',
			MOV_ITEMS_SELECTED.MOV_FROM_STORE=>'Requested inventory list received<br />Awaiting response<br /><font color="red"><b>Please reply!</b></font>',
			MOV_ITEMS_SELECTED.MOV_TO_STORE=>'Requested inventory list sent<br />Awaiting reply',
			MOV_IN_TRANSIT=>'Items in transit',
			MOV_ITEMS_RECEIVED.MOV_FROM_STORE=>'Items received by <b>To</b> store<br />Please confirm request completion',
			MOV_ITEMS_RECEIVED.MOV_TO_STORE=>'Items received<br />Awaiting confirmation from <b>From</b> store',
			MOV_NOT_ALL_RECEIVED=>'Items received<p /><font color="red"><b>Not all items were received!<br />Please contact the store<br />to resolve this issue.</b></font>',
			MOV_DENIED=>'Request denied'
		);

		$this->error = new error('Inventory Movement Class');
	}

	/**
	* Return the status with MOV_FROM_STORE/MOV_TO_STORE concatenated if necessary
	* @param integer $status
	* @param integer $tostoreID
	* @return string
	*/
	function fixStatus($status,$tostoreID)
	{
		if ($status == MOV_REQUEST_SENT)
		{
			$status .= ($_SESSION['storeID']==$tostoreID?MOV_TO_STORE:MOV_FROM_STORE);
		}
		elseif ($status == MOV_ITEMS_SELECTED)
		{
			$status .= ($_SESSION['storeID']==$tostoreID?MOV_TO_STORE:MOV_FROM_STORE);
		}
		elseif ($status == MOV_ITEMS_RECEIVED)
		{
			$status .= ($_SESSION['storeID']==$tostoreID?MOV_TO_STORE:MOV_FROM_STORE);
		}

		return $status;
	}

	/**
	* Return the status words associated with the current status
	* @param integer $status
	* @param integer $tostoreID
	* @return string
	*/
	function getStatusWords($status,$tostoreID)
	{
		return @$this->status_words[$this->fixStatus($status,$tostoreID)];
	}

	/**
	* Set the requestID and pull it's information
	* @param integer $requestID
	* @return
	*/
	function setRequestID($requestID)
	{
		global $db;

		$_SESSION['requestID'] = 0;
		$_SESSION['request_info'] = array();

		if ($requestID)
		{
			unset($_SESSION['request_info']);
			$sql = "SELECT * FROM requests WHERE req_requestID=$requestID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			if (mysql_num_rows($result))
			{
				$row = mysql_fetch_assoc($result);
				$info = unserialize($row['req_config']);
				unset($row['req_config']);
				$info = array_merge($info,$row);

				$_SESSION['requestID'] = $requestID;

				if (is_array($info))
				{
					$_SESSION['request_info'] = $info;

					// get the request items
					$items = array();
					$sql = "SELECT * FROM request_items WHERE rqi_requestID=$requestID ORDER BY rqi_platformname,rqi_title";
					$result = mysql_query($sql,$db);
					$this->error->mysql(__FILE__,__LINE__);
					while ($row = mysql_fetch_assoc($result)) { $items[] = $row; }
					$_SESSION['request_info']['items'] = $items;
				}
				else { $_SESSION['request_info'] = array(); }
			}
		}
	}

	/**
	* Return the information for the current request
	* @return array
	*/
	function getRequestInfo() { return @$_SESSION['request_info']; }

	/**
	* Create a new request (the stores and discounts must be set)
	* @return integer requestID
	*/
	function createNew()
	{
		global $db;

		$info = @$_SESSION['request_info'];
		if (!@$_SESSION['requestID'] && count($info))
		{
			// insert the request into the database and get the requestID
			unset($info['items']);
			$config = serialize($info);

			$vals = array();
			$vals[] = "req_fromstoreID={$info['stores']['from']}";
			$vals[] = "req_tostoreID={$info['stores']['to']}";
			$vals[] = "req_config='".mysql_escape_string($config)."'";
			$vals[] = "req_status=".MOV_INCOMPLETE;
			$vals[] = "req_timeadded=".time();
			$vals[] = "req_active=".YES;

			$sql = "INSERT INTO requests SET ".implode(',',$vals);
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$requestID = mysql_insert_id();

			return $requestID;
		}
	}

	/**
	* Pull in all requests for this store
	* @param boolean $previous pull in complete invoices where this is the FROM store? [optional, default NO]
	*/
	function setRequests($previous=NO)
	{
		global $db;

		$this->requests = array();
		$sql = "SELECT req_requestID,req_fromstoreID,req_tostoreID,req_status,req_notes,req_timeadded,sto_name AS sto_fromname FROM requests,stores WHERE req_active=" . YES . " AND (req_tostoreID={$_SESSION['storeID']} OR req_fromstoreID={$_SESSION['storeID']}) AND req_fromstoreID=sto_storeID ORDER BY req_timeadded";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result))
		{
			$add = YES;

			// if they are the FROM store, and the status is complete, do not show
			if ($row['req_fromstoreID'] == $_SESSION['storeID'] && in_array($row['req_status'],array(MOV_COMPLETE)) && !$previous) { $add = NO; }

			// if they are the TO store, and the status is denied/received/not all received/incomplete, do not show (it should be impossible to have a tostoreID and a status of MOV_INCOMPLETE, but this is here as a safety measure)
			if ($row['req_tostoreID'] == $_SESSION['storeID'] && in_array($row['req_status'],array(MOV_DENIED,MOV_NOT_ALL_RECEIVED,MOV_INCOMPLETE,MOV_COMPLETE)) && !$previous) { $add = NO; }

			// if they are viewing previous, only show complete
			if ($previous && $row['req_status'] != MOV_COMPLETE) { $add = NO; }

			if ($add)
			{
				if ($row['req_tostoreID'])
				{
					$toname = $this->getStoreName($row['req_tostoreID']);

					if (!strlen($toname)) { $toname = '- Unknown -'; }
				}
				else { $toname = '- Unknown -'; }
				$row['sto_toname'] = $toname;

				$items = array();
				$requestID = $row['req_requestID'];
				$isql = "SELECT * FROM request_items WHERE rqi_requestID=$requestID";
				$iresult = mysql_query($isql,$db);
				$this->error->mysql(__FILE__,__LINE__);
				while ($irow = mysql_fetch_assoc($iresult)) { $items[] = $irow; }
				$row['items'] = $items;

				$this->requests[] = $row;
			}
		}
	}

	/**
	* Return all requests for this store
	* @return array
	*/
	function getRequests()
	{
		return $this->requests;
	}

	/**
	* Return the to/from stores
	* @return array
	*/
	function getStores() { return @$_SESSION['request_info']['stores']; }

	/**
	* Return the from store min/to store max quantities
	* @return array
	*/
	function getQuantities() { return @$_SESSION['request_info']['qtys']; }

	/**
	* Get the box/manual/condition discounts
	* @param string $type which discount type? [optional, default '']
	* @return array
	*/
	function getDiscounts($type='')
	{
		$discounts = @$_SESSION['request_info']['discounts'];
		if ($type == '') { return $discounts; }
		else { return (is_array(@$discounts[$type])?$discounts[$type]:array()); }
	}

	/**
	* Get the selected platforms
	* @return array/integer
	*/
	function getSelectedPlatforms()
	{
		return (is_array(@$_SESSION['request_info']['selplatforms'])?$_SESSION['request_info']['selplatforms']:-1);
	}

	/**
	* Return the items from the current request
	* @return array
	*/
	function getRequestItems()
	{
		$info = $this->getRequestInfo();
		return @$info['items'];
	}

	/**
	* Pull in the items according to the to/from stores and the from min/to max quantities
	*/
	function setCriteriaItems()
	{
		global $db;

		set_time_limit(300);
		$this->items = array();

		$fromqty = $_SESSION['request_info']['qtys']['from'];
		$toqty = $_SESSION['request_info']['qtys']['to'];

		$vals = array();
		$sql = "SELECT itm_itemID,itm_title,pla_platformID,pla_name,q1.qty_new AS fromqty_new,q1.qty_used AS fromqty_used,q2.qty_new AS toqty_new,q2.qty_used AS toqty_used,prc_new,prc_used FROM items,platforms,quantity AS q1,quantity AS q2,prices WHERE ";
		$vals[] = "q1.qty_storeID={$_SESSION['request_info']['stores']['from']}";
		$vals[] = "q2.qty_storeID={$_SESSION['request_info']['stores']['to']}";
		$vals[] = "((q1.qty_new>=$fromqty AND q2.qty_new<=$toqty) OR (q1.qty_used>=$fromqty AND q2.qty_used<=$toqty))";
		$vals[] = "q1.qty_itemID=q2.qty_itemID";
		$vals[] = "q1.qty_itemID=prc_itemID";
		$vals[] = "prc_itemID=itm_itemID";
		$vals[] = "itm_platformID=pla_platformID";
		$sql .= implode(' AND ',$vals);
		$sql .= " ORDER BY pla_name,itm_title";

		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result)) { $this->items[] = $row; }
	}

	/**
	* Return the criteria-matched items
	* @return array
	*/
	function getCriteriaItems() { return $this->items; }

	/**
	* Return the platforms of the given items
	* @param array $items
	* @return array
	*/
	function getItemPlatforms($items)
	{
		$items = $this->getCriteriaItems();

		$platforms = array();
		while (list($a,$arr) = each($items))
		{
			if (!in_array($arr['pla_platformID'],array_keys($platforms))) { $platforms[$arr['pla_platformID']] = $arr['pla_name']; }
		}
		reset($items);

		return $platforms;
	}

	/**
	* Remove all items for the non-selected platforms
	*/
	function removeNonSelectedPlatformItems()
	{
		$info = $this->getRequestInfo();
		$selplatforms = $this->getSelectedPlatforms();
		$items = $this->getRequestItems();

		// loop through the items
		$newitems = array();
		while (list($a,$arr) = each($items))
		{
			if (in_array($arr['pla_platformID'],$selplatforms)) { $newitems[] = $arr; }
		}

		$_SESSION['request_info']['items'] = $newitems;
	}

	/**
	* Take a list of itemID.newused values and return only the itemIDs
	* @param array $itemIDnus
	* @return array
	*/
	function onlyItemIDs($itemIDnus)
	{
		$itemIDs = array();
		while (list($a,$itemIDnu) = each($itemIDnus)) { $itemIDs[] = substr($itemIDnu,0,(strlen($itemIDnu)-2)); }

		return $itemIDs;
	}

	/**
	* Insert or update the items in the request_items table
	* @param integer $requestID
	* @param integer $status
	* @param array $selitemIDs
	* @param array $byitem only passed when $status=MOV_INCOMPLETE
	*/
	function setItems($requestID,$status,$selitemIDs,$byitem)
	{
		global $db;

		if ($status == MOV_IN_TRANSIT)
		{
			// rather than DELETE/INSERT the items, just set rqi_shipped to YES for the ones that were selected
			// also update their box/instructions/condition/price values
			// (this has to be broken into separate UPDATE queries because we're passing the newused field as well)
			$sqls = array();
			while (list($a,$itemIDnu) = each($selitemIDs))
			{
				$itemID = substr($itemIDnu,0,(strlen($itemIDnu)-2));
				$newused = substr($itemIDnu,-2);
				$data = $byitem[$itemIDnu];
				$box = $data['box'];
				$instructions = $data['instructions'];
				$condition = $data['condition'];
				$price = $data['price'];

				$setvals = array(
					"rqi_received=".YES,
					"rqi_box=$box",
					"rqi_instructions=$instructions",
					"rqi_condition=$condition",
					"rqi_price=$price"
				);

				$sqls[] = "UPDATE request_items SET ".implode(',',$setvals)." WHERE rqi_requestID=$requestID AND rqi_itemID=$itemID AND rqi_newused=$newused";
			}

			while (list($a,$sql) = each($sqls))
			{
				set_time_limit(30);
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}
		}
		else
		{
			$itemdata = array(); // format: $itemdata[itemID] = array(data)
			if ($status == MOV_INCOMPLETE)
			{
				// since we're INSERTing the items for the first time, get their data
				$itemIDs = $this->onlyItemIDs(array_keys($byitem));
				$sql = "SELECT * FROM items,platforms WHERE itm_itemID IN (".implode(',',$itemIDs).") AND itm_platformID=pla_platformID";
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
				while ($row = mysql_fetch_assoc($result)) { $itemdata[$row['itm_itemID']] = $row; }
			}
			else
			{
				// delete the entries in request_items so that we can insert them below
				// this also weeds out the items that aren't being saved at this step (IE: not requested, not shipped, etc)
				$sql = "DELETE FROM request_items WHERE rqi_requestID=$requestID";
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);

				// populate $itemdata from $_SESSION['request_info']['items']
				while (list($a,$arr) = each($_SESSION['request_info']['items'])) { $itemdata[$arr['rqi_itemID']] = $arr; }
			}

			// generate INSERT query
			$fromqty = $_SESSION['request_info']['qtys']['from'];
			$toqty = $_SESSION['request_info']['qtys']['to'];
			$vals = array();
			while (list($itemIDnu,$arr) = each($byitem))
			{
				$itemID = substr($itemIDnu,0,(strlen($itemIDnu)-2));
				$newused = substr($itemIDnu,-2);

				$data = $itemdata[$itemID];
				if ($status == MOV_INCOMPLETE)
				{
					$platformID = $data['pla_platformID'];
					$platform_name = $data['pla_name'];
					$title = $data['itm_title'];

					$shipped = NO;
					$received = NO;
				}
				else
				{
					$platformID = $data['rqi_platformID'];
					$platform_name = $data['rqi_platformname'];
					$title = $data['rqi_title'];

					$shipped = ($status>=MOV_ITEMS_SELECTED?YES:NO);
					$received = ($status>=MOV_IN_TRANSIT?YES:NO);
				}

				$val = array(
					$itemID,
					$requestID,
					$platformID,
					"'".mysql_escape_string($platform_name)."'",
					"'".mysql_escape_string($title)."'",
					$newused,
					$arr['box'],
					$arr['instructions'],
					$arr['condition'],
					sprintf('%0.2f',$arr['price']),
					$shipped,
					$received
				);
				$vals[] = '('.implode(',',$val).')';
			}

			$sql = "INSERT INTO request_items VALUES ".implode(',',$vals);
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}
	}

	/**
	* Set the request's status to either the next-status-up or the provided status and return the new status and it's associated text
	* @param integer $requestID
	* @param integer $curstatus
	* @param integer $setstatus [optional, default -1]
	* @return array
	*/
	function setStatus($requestID,$curstatus,$setstatus=-1)
	{
		global $db;

		$order = array(
			MOV_INCOMPLETE,
			MOV_REQUEST_SENT,
			MOV_ITEMS_SELECTED,
			MOV_IN_TRANSIT,
			MOV_ITEMS_RECEIVED
		);
		$words = array(
			MOV_INCOMPLETE=>'Inventory list sent',
			MOV_REQUEST_SENT=>'Requested inventory list sent',
			MOV_ITEMS_SELECTED=>'Shipped inventory list sent',
			MOV_IN_TRANSIT=>'Received inventory list sent'
		);

		$setstatus = ($setstatus==-1?@$order[(array_search($curstatus,$order)+1)]:$setstatus);

		if ($setstatus == MOV_COMPLETE) { $comptime = ',req_timecompleted='.time(); } else { $comptime = ''; }

		$sql = "UPDATE requests SET req_status={$setstatus}{$comptime} WHERE req_requestID=$requestID";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		return array($setstatus,@$words[$curstatus]);
	}

	/**
	* Clear the session of all request-related variables
	*/
	function clearSession()
	{
		while (list($k,$v) = each($_SESSION))
		{
			if (substr($k,0,7) == 'request') { unset($_SESSION[$k]); }
		}
	}

	/**
	* Show the from/to stores
	* @param integer $change show the 'change stores...' button? [optional, default NO]
	* @param integer $highlight highlight the active store in red? [optional, default YES]
	*/
	function showStores($change=NO,$highlight=YES)
	{
		$stores = $this->getStores();
		$qtys = $this->getQuantities();

		if (is_array($stores) && count($stores) == 2)
		{
			if ($highlight)
			{
				if ($stores['from'] == $_SESSION['storeID']) { $fo1 = '<font color="red">'; $fc1 = '</font>'; $fo2 = ''; $fc2 = ''; }
				else { $fo2 = '<font color="red">'; $fc2 = '</font>'; $fo1 = ''; $fc1 = ''; }
			}
			else { $fo1 = ''; $fc1 = ''; $fo2 = ''; $fc2 = ''; }

			$sto = new stores($this->pg);
			?>
			<?=$fo1;?><b>From Store:</b> <?=$sto->getStoreName($stores['from']);?> (qty&gt;=<?=$qtys['from'];?>)<?=$fc1;?><br />
			<?=$fo2;?><b>To Store:</b> <?=$sto->getStoreName($stores['to']);?> (qty&lt;=<?=$qtys['to'];?>)<?=$fc2;?>
			<p />
			<?php
			if ($change) { ?><input type="button" value="&lt; Change Stores/Quantities" onclick="document.location='/admin/utilities/invmove_pickstores.php'" class="btn" /><p /><?php }
		}
	}

	/**
	* Retrieve a store name
	* @param integer $storeID
	* @return string
	*/
	function getStoreName($storeID)
	{
		$sto = new stores($this->pg);
		return $sto->getStoreName($storeID);
	}

	/**
	* Delete a request and it's items
	* @param integer $requestID
	*/
	function delete($requestID)
	{
		global $db;

		$sql = "DELETE FROM requests WHERE req_requestID=$requestID";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		$sql = "DELETE FROM request_items WHERE rqi_requestID=$requestID";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
	}

	/**
	* Return YES/NO if all shipped items were received
	* @param integer $requestID
	* @return integer
	*/
	function wereAllReceived($requestID)
	{
		global $db;

		$sql = "SELECT COUNT(*) AS count FROM request_items WHERE rqi_requestID=$requestID AND rqi_shipped=" . YES . " AND rqi_received=".NO;
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		$row = mysql_fetch_assoc($result);
		$count = $row['count'];

		return (!$count?YES:NO);
	}

	/**
	* Update the given store's quantities
	* @param integer $requestID
	* @param integer $which MOV_FROM_STORE (subtract) or MOV_TO_STORE (add)
	*/
	function updateQuantities($requestID,$which)
	{
		global $db;

		$sql = "SELECT * FROM requests WHERE req_requestID=$requestID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		if (in_array($which,array(MOV_FROM_STORE,MOV_TO_STORE)) && mysql_num_rows($result))
		{
			$info = mysql_fetch_assoc($result);

			if (($which == MOV_FROM_STORE && !$info['req_fromqty_subtracted']) || ($which == MOV_TO_STORE && !$info['req_toqty_added']))
			{
				$new_itemIDs = array();
				$used_itemIDs = array();

				$sql = "SELECT * FROM request_items WHERE rqi_requestID=$requestID";
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
				while ($row = mysql_fetch_assoc($result))
				{
					if ($row['rqi_newused'] == ITEM_NEW) { $new_itemIDs[] = $row['rqi_itemID']; }
					elseif ($row['rqi_newused'] == ITEM_USED) { $used_itemIDs[] = $row['rqi_itemID']; }
				}

				$sqls = array();
				if ($which == MOV_FROM_STORE)
				{
					// subtract the quantities from the FROM store
					if (count($new_itemIDs)) { $sqls[] = "UPDATE quantity SET qty_new=(qty_new-1) WHERE qty_storeID={$info['req_fromstoreID']} AND qty_itemID IN (".implode(',',$new_itemIDs).")"; }
					if (count($used_itemIDs)) { $sqls[] = "UPDATE quantity SET qty_used=(qty_used-1) WHERE qty_storeID={$info['req_fromstoreID']} AND qty_itemID IN (".implode(',',$used_itemIDs).")"; }
				}
				else
				{
					// add the quantities to the TO store
					if (count($new_itemIDs)) { $sqls[] = "UPDATE quantity SET qty_new=(qty_new+1) WHERE qty_storeID={$info['req_tostoreID']} AND qty_itemID IN (".implode(',',$new_itemIDs).")"; }
					if (count($used_itemIDs)) { $sqls[] = "UPDATE quantity SET qty_used=(qty_used+1) WHERE qty_storeID={$info['req_tostoreID']} AND qty_itemID IN (".implode(',',$used_itemIDs).")"; }
				}

				while (list($a,$sql) = each($sqls))
				{
					mysql_query($sql,$db);
					$this->error->mysql(__FILE__,__LINE__);
				}
			}
		}
	}
}

?>