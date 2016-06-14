<?php
/*
* Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include('../../include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$return = @$_POST['return'];
if (!strlen($return)) { $return = '/admin/endofyear/items_form.php'; }

$pg = new admin_page();
$error = new error('End of Year Item Quantities - Update');

if ($act == "setcriteria")
{
	// set the criteria to a session variable
	$platformID = (isset($_GET['platformID'])?$_GET['platformID']:@$_POST['platformID']);
	$yearID = (isset($_GET['yearID'])?$_GET['yearID']:@$_POST['yearID']);
	$_SESSION['endofyear_item_criteria'] = array(
		'platformID'=>$platformID,
		'yearID'=>$yearID
	);
}
elseif ($act == "updateitems")
{
	// update the items in the database
	$doupdate = @$_POST['doupdate'];
	if ($doupdate && isset($_POST['yearID']) && is_array(@$_POST['qtys']))
	{
		// update the quantities in the endofyear_items table
		$qtys = $_POST['qtys'];
		$yearID = $_POST['yearID'];
		$vals = array();
		$itemIDs = array();
		$sqls = array();

		while (list($itemID,$arr) = each($qtys))
		{
			$itemIDs[] = $itemID;
			$vals[] = "($itemID,$yearID,{$arr[ITEM_NEW]},{$arr[ITEM_USED]})";
		}

		if (count($itemIDs))
		{
			$sqls = array(
				"DELETE FROM endofyear_items WHERE eyi_yearID=$yearID AND eyi_itemID IN (".implode(',',$itemIDs).")",
				"INSERT INTO endofyear_items VALUES ".implode(',',$vals)
			);
		}

		// execute the queries
		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);
		}
	}

	$page = @$_POST['page'];
	$return = "/admin/endofyear/items_form.php?page=$page&updated=$doupdate&count=".count(@$qtys);
}
elseif ($act == "complete")
{
	// update the actual on-hand quantities, set the platform as completed, and redirect to the platform selection page
	$platformID = $_SESSION['endofyear_item_criteria']['platformID'];
	$yearID = $_SESSION['endofyear_item_criteria']['yearID'];
	$default = @$_SESSION['endofyear_defaults'];
	if (!is_array($default)) { $default = array(); }

	if ($platformID && $yearID)
	{
		if (count($default))
		{
			$vals = array();
			while (list($a,$itemID) = each($default)) { $vals[] = "($itemID,$yearID,0,0)"; }
			$sql = "INSERT IGNORE INTO endofyear_items VALUES ".implode(',',$vals);
			mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);
		}

		// build update queries
		$sqls = array();
		$itemIDs = array();
		$sql = "SELECT * FROM endofyear_items,items WHERE eyi_yearID=$yearID AND eyi_itemID=itm_itemID AND itm_platformID=$platformID ORDER BY itm_itemID";
		$result = mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result))
		{
			$itemIDs[] = $row['itm_itemID'];
			$sqls[] = "UPDATE quantity SET qty_new={$row['eyi_new']},qty_used={$row['eyi_used']} WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID={$row['itm_itemID']}";
		}

		$affected = 0;
		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);
			$affected += mysql_affected_rows();
		}

		// delete the end of year items and set the year as completed
		$sql = "DELETE FROM endofyear_items WHERE eyi_yearID=$yearID AND eyi_itemID IN (".implode(',',$itemIDs).")";
		mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);

		$sql = "UPDATE endofyear_platforms SET eyp_completed=" . YES . ",eyp_completedtime=".time()." WHERE eyp_platformID=$platformID AND eyp_yearID=$yearID";
		mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);

		unset($_SESSION['endofyear_defaults']);

		$return = "/admin/endofyear/items.php?completed=" . YES . "&platformID=$platformID&yearID=$yearID&affected=$affected";
	}
}

$pg->showUpdating('Updating Item Quantities...',$return);
?>