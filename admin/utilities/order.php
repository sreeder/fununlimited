<?php
include('../../include/include.inc');

$cl = new check_login();

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

$pg = new admin_page();
$pg->setTitle('Order Generation');
$pg->head('Order Generation');

$error = new error('Order Generation');

$ord = new order($pg);

if ($act == "")
{
	// output the previous orders/generate new form button
	$ord->show_list();
}
elseif ($act == "criteria")
{
	// output the criteria form
	$ord->pull_post();
	$ord->criteria_form();
}
elseif ($act == "generate")
{
	// generate/output the order
	$ord->pull_post();
	$ord->criteria_form();
	$ord->generate();
	$ord->put_in_database();
}
elseif ($act == "view")
{
	// view the details of a previous order
	$ord->view($_POST['orderID']);
}
elseif ($act == "delete")
{
	// delete a previous order
	$ord->delete($_POST['orderID']);
	$ord->show_list();
}

$pg->foot();
?>