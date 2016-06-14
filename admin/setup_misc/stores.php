<?php
include('../../include/include.inc');

$cl = new check_login(ADMIN);

$pg = new admin_page();
$pg->setTitle('Manage Stores');
$pg->head('Manages Stores');

$stores = new stores($pg);

$act = @$_POST['act'];
$active = (isset($_GET['active'])?$_GET['active']:(isset($_POST['active'])?$_POST['active']:YES));

if ($act == "")
{
	// display all stores
	$stores->show_stores($active);
}
elseif ($act == "add" || $act == "edit")
{
	// display the add/edit store form
	$storeID = @$_POST['storeID'];
	if ($storeID) { $stores->set_storeID($storeID); }
	$stores->add_form($active);
}
elseif ($act == "doadd")
{
	// add/edit a store
	$stores->pull_post();
	$stores->add();

	if ($stores->was_added())
	{
		$pg->status(($stores->storeID?'Edited':'Added').' store: <b>'.$stores->info['name'].'</b>');
		$stores->show_stores(($stores->storeID?$active:YES));
	}
	else
	{
		$stores->show_errors();
		$stores->add_form($active);
	}
}
elseif ($act == "activate")
{
	// re/de-activate a store
	$storeID = @$_POST['storeID'];
	$active = @$_POST['active'];
	$stores->set_storeID($storeID);
	$stores->activate($active);

	if ($stores->was_deleted())
	{
		$pg->status(($active==NO?'Re':'De').'activated store: <b>'.$stores->info['name'].'</b>');
		$stores->show_stores(($active==YES?NO:YES));
	}
	else { echo "There was an error...tell Scott - this shouldn't happen here..."; }
}
elseif ($act == "permdelete")
{
	// permanently delete a store and all traces of it
	$storeID = @$_POST['storeID'];
	$stores->set_storeID($storeID);
	$stores->permdelete();

	if ($stores->was_deleted())
	{
		$pg->status('Permanently deleted store (Scott - remove ALL values): <b>'.@$stores->info['name'].'</b>');
		$stores->show_stores($active);
	}
	else { echo "There was an error...tell Scott - this shouldn't happen here..."; }
}

$pg->foot();
?>