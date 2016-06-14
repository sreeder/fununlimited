<?php
include('../../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setTitle('Sales Milestone Discounts');
$pg->head('Sales Milestone Discounts');

$error = new error('Sales Milestone Discounts');
$mile = new milestone($pg);

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

if ($act == "")
{
	// show the current milestones and a form to add a new one
	$mile->show_list();
}
elseif ($act == "edit")
{
	$mile->pull_post();
	$mile->add_form();
}
elseif ($act == "add" || $act == "doedit")
{
	$mile->pull_post(NO);
	$mile->add();
	$mile->show_list(($act=='add'?'New milestone added':'Milestone edited'),YES);
}
elseif ($act == "delete")
{
	$mile->pull_post();
	$mile->delete();
	$mile->show_list('Milestone deleted',YES);
}
elseif ($act == "showcusts")
{
	$mile->pull_post();
	$mile->pull_customers();
	$mile->show_customers();
}

$pg->foot();
?>