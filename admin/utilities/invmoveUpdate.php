<?php
/*
* Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include('../../include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$requestID = (isset($_GET['requestID'])?$_GET['requestID']:@$_POST['requestID']);
$return = @$_POST['return'];
if (!strlen($return)) { $return = '/admin/utilities/invmove.php'; }

$pg = new admin_page();
$mov = new inventory_movement($pg);

if ($act == "load")
{
	// load a request
	if ($requestID)
	{
		$mov->setRequestID($requestID);
		$return = '/admin/utilities/invmove_picklist.php';
	}
	else { echo "Invalid requestID: $requestID<p />Press <b>Back</b> and try again."; }
}
elseif ($act == "setstores")
{
	// set the stores
	if (!isset($_SESSION['request_info'])) { $_SESSION['request_info'] = array(); }
	if (is_array(@$_POST['stores']) && is_array(@$_POST['qtys']))
	{
		$_SESSION['request_info']['stores'] = $_POST['stores'];
		$_SESSION['request_info']['qtys'] = $_POST['qtys'];
		$return = '/admin/utilities/invmove_discounts.php';
	}
	else { $return = '/admin/utilities/invmove_pickstores.php'; }
}
elseif ($act == "setdiscounts")
{
	// set the discounts
	$selplatforms = @$_POST['selplatforms'];
	if (!is_array($selplatforms)) { $selplatforms = array(); }
	$_SESSION['request_info']['selplatforms'] = $selplatforms;

	if (!isset($_SESSION['request_info'])) { $_SESSION['request_info'] = array(); }
	if (!count(@$_POST['selplatforms'])) { $return = '/admin/utilities/invmove_discounts.php?error=No platforms selected!'; }
	elseif (isset($_POST['discounts']) && is_array($_POST['discounts']))
	{
		$_SESSION['request_info']['discounts'] = $_POST['discounts'];

		// remove items for non-selected platforms
		$mov->removeNonSelectedPlatformItems();

		// create the inventory request
		$requestID = $mov->createNew();
		if ($requestID)
		{
			$_SESSION['requestID'] = $requestID;
			$return = '/admin/utilities/invmove_picklist.php';
		}
		else { $return = '/admin/utilities/invmove_discounts.php'; }
	}
	else { $return = '/admin/utilities/invmove_pickstores.php'; }
}
elseif ($act == "setitems")
{
	// set/update the request items
	$status = $_POST['status'];
	$box = @$_POST['b'];
	$instructions = @$_POST['i'];
	$condition = @$_POST['c'];
	$prices = (isset($_POST['prices'])?explode(',',$_POST['prices']):array());
	$selitemIDs = array_keys($_POST['sel']);

	$itemIDnus = $_SESSION['request_itemIDnus'];

	// separate the data by itemID.newused
	$byitem = array();
	while (list($a,$itemIDnu) = each($itemIDnus))
	{
		if (in_array($itemIDnu,$selitemIDs))
		{
			$byitem[$itemIDnu] = array(
				'box'=>yn(@$box[$itemIDnu]),
				'instructions'=>yn(@$instructions[$itemIDnu]),
				'condition'=>@$condition[$itemIDnu],
				'price'=>@$prices[$a]
			);
		}
	}

	$mov->setItems($requestID,$status,$selitemIDs,$byitem);
	list($newstatus,$statustext) = $mov->setStatus($requestID,$status);

	// if applicable, update the quantities
	if ($newstatus == MOV_IN_TRANSIT) { $mov->updateQuantities($requestID,MOV_FROM_STORE); }
	elseif ($newstatus == MOV_ITEMS_RECEIVED) { $mov->updateQuantities($requestID,MOV_TO_STORE); }

	$mov->clearSession();

	$return = "/admin/utilities/invmove.php?status=$statustext";
}
elseif (in_array($act,array('selrequested','selshipped','selreceived')))
{
	$mov->setRequestID($requestID);
	$return = '/admin/utilities/invmove_picklist.php';
}
elseif (in_array($act,array('printitems','printitemsnr')))
{
	$return = "/admin/utilities/invmove_print.php?act=$act&requestID=$requestID";
}
elseif ($act == "complete")
{
	// set a request as completed
	$mov->setStatus($requestID,-1,MOV_COMPLETE);
	$return = "/admin/utilities/invmove.php?status=Inventory request completed";
}
elseif ($act == "resend")
{
	// set a denied request as requested again
	$mov->setStatus($requestID,-1,MOV_REQUEST_SENT);
	$return = "/admin/utilities/invmove.php?status=Inventory request resent";
}
elseif ($act == "deny")
{
	// set a request as denied
	$mov->setStatus($requestID,-1,MOV_DENIED);
	$return = "/admin/utilities/invmove.php?status=Inventory request denied";
}
elseif ($act == "delete")
{
	// delete a request
	$mov->delete($requestID);
	$mov->clearSession();
	$return = "/admin/utilities/invmove.php?status=Inventory request deleted";
}

$pg->showUpdating('Updating Inventory Movement Request...',$return);

function yn($val) { return (strtolower($val)=='on'||$val==1?YES:NO); }
?>