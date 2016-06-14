<?php
/*
* Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include('../../include/include.inc');

$act = getGP('act');
$return = @$_POST['return'];
if (!strlen($return)) { $return = '/admin/utilities/receive_order.php?act=form'; }

$pg = new admin_page();
$ord = new receive_order($pg);

if ($act == "select")
{
	// add an item to the current order
	$ord->pull_post();
	$itemID = getGP('itemID');
	$newused = getP('newused',@$_SESSION['receive_newused']);
	$platformID = getP('platformID',@$_SESSION['receive_last_platformID']);
	$ord->add($itemID,$newused,$platformID);
}
elseif ($act == "remove")
{
	// remove items from the current order
	$ord->pull_post();
	$ord->remove();
}
elseif ($act == "update")
{
	// update quantities/prices/UPCs
	$ord->updateValues();
}
elseif ($act == "complete")
{
	// complete the current order
	$ord->pull_post();
	$ord->complete();
	$return = '/admin/utilities/receive_order.php';
}

$pg->showUpdating('Updating Received Order...',$return);
?>