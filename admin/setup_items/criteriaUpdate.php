<?php
/*
* Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include('../../include/include.inc');

$act = getGP('act');

$pg = new admin_page();
$error = new error('Edit Items by Criteria - Update');

if ($act == "setcriteria")
{
	// set the criteria to a session variable
	$_SESSION['platform_item_criteria'] = array(
		'platformID'=>$_POST['platformID'],
		'elements'=>$_POST['elements']
	);
}
elseif ($act == "updateitems")
{
	// update the items in the database
	$doupdate = @$_POST['doupdate'];
	if ($doupdate && is_array(@$_POST['info']))
	{
		// update the items/information in the database
		$info = $_POST['info'];
		$sqls = array();

		while (list($itemID,$arr) = each($info))
		{
			$by_prefix = array();
			while (list($key,$val) = each($arr))
			{
				$prefix = substr($key,0,3);
				if (!isset($by_prefix[$prefix])) { $by_prefix[$prefix] = array(); }
				$by_prefix[$prefix][$key] = $val;
			}

			while (list($prefix,$values) = each($by_prefix))
			{
				// depending on the prefix, construct the query
				if ($prefix == "itm")
				{
					$vals = array();
					while (list($k,$v) = each($values)) { $vals[] = "$k='".mysql_escape_string(stripslashes($v))."'"; }
					$sqls[] = "UPDATE items SET ".implode(',',$vals)." WHERE itm_itemID=$itemID";
				}
				elseif ($prefix == "qty")
				{
					$vals = array();
					while (list($k,$v) = each($values)) { $vals[] = "$k='".mysql_escape_string($v)."'"; }
					$sqls[] = "UPDATE quantity SET ".implode(',',$vals)." WHERE qty_itemID=$itemID";
				}
				elseif  ($prefix == "prc")
				{
					$vals = array();
					while (list($k,$v) = each($values)) { $vals[] = "$k='".mysql_escape_string($v)."'"; }
					$sqls[] = "UPDATE prices SET ".implode(',',$vals)." WHERE prc_itemID=$itemID";
				}
				elseif ($prefix == "sp_")
				{
					while (list($k,$v) = each($values))
					{
						$sourceID = substr($k,3);
						$sqls[] = "DELETE FROM item_source_values WHERE isv_sourceID=$sourceID AND isv_itemID=$itemID";
						if (strlen($v)) { $sqls[] = "INSERT INTO item_source_values SET isv_itemID=$itemID,isv_sourceID=$sourceID,isv_value='$v'"; }
					}
				}
			}
		}

		// execute the queries
		$changed = 0;
		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);

			if (substr($sql,0,6) != 'DELETE') { $changed += mysql_affected_rows(); }
		}
	}

	$page = @$_POST['page'];
	$return = "/admin/setup_items/platform_items_form.php?page=$page&updated=$doupdate&count=".count(@$info)."&changed=".@$changed;
}

if (strlen($return)) { $pg->showUpdating('Updating Whole Platform Items...',$return); }
?>