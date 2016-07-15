<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$pg = new admin_page();
$pg->setTitle('Store Inventory Selection Priorities');
$pg->head('Store Inventory Selection Priorities');

$sp = new store_priorities($pg);

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

if ($act == "")
{
	// display the priority form
	$sp->show_stores();
}
elseif ($act == "set")
{
	// set the priorities and show the priority form
	$sp->set_priorities();
	$pg->status('Set priorities');
	$sp->get_stores();
	$sp->show_stores();
}

$pg->foot();
?>