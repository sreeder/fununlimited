<?php
include('../../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setTitle('Manage Employees');
$pg->head('Manage Employees');

$empl = new employees($pg);

$act = @$_POST['act'];
$active = (isset($_GET['active'])?$_GET['active']:(isset($_POST['active'])?$_POST['active']:YES));

if ($act == "")
{
	// display all employees
	$empl->show_employees($active);
}
elseif ($act == "add" || $act == "edit")
{
	// display the add/edit employee form
	$employeeID = @$_POST['employeeID'];
	if ($employeeID) { $empl->set_employeeID($employeeID); }
	$empl->add_form($active);
}
elseif ($act == "doadd")
{
	// add/edit a employee
	$empl->pull_post();
	$empl->add();

	if ($empl->was_added())
	{
		$pg->status(($empl->employeeID?'Edited':'Added').' employee: <b>'.$empl->info['fname'].' '.$empl->info['lname'].'</b>');
		$empl->show_employees(($empl->employeeID?$active:YES));
	}
	else
	{
		$empl->show_errors();
		$empl->add_form($active);
	}
}
elseif ($act == "activate")
{
	// re/de-activate a employee
	$employeeID = @$_POST['employeeID'];
	$active = @$_POST['active'];
	$empl->set_employeeID($employeeID);
	$empl->activate($active);

	if ($empl->was_deleted())
	{
		$pg->status(($active==NO?'Re':'De').'activated employee: <b>'.$empl->info['fname'].' '.$empl->info['lname'].'</b>');
		$empl->show_employees(($active==YES?NO:YES));
	}
	else { echo "There was an error...tell Scott - this shouldn't happen here..."; }
}
elseif ($act == "permdelete")
{
	// permanently delete a employee and all traces of it
	$employeeID = @$_POST['employeeID'];
	$empl->set_employeeID($employeeID);
	$empl->permdelete();

	if ($empl->was_deleted())
	{
		$pg->status('Permanently deleted employee (Scott - remove ALL values): <b>'.@$empl->info['fname'].' '.@$empl->info['lname'].'</b>');
		$empl->show_employees($active);
	}
	else { echo "There was an error...tell Scott - this shouldn't happen here..."; }
}
elseif ($act == "history")
{
	// output the employee history
	// (select date range and then display the history)
	echo "Employee History";
}

$pg->foot();
?>