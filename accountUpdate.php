<?php
/*
* Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include('include/include.inc');

$pg = new page();
$cust = new customers($pg);

$act = getGP('act');
$return = getP('return');
if (!strlen($return)) { $return = '/account.php'; }

if ($act == 'edit')
{
	$_POST['customerID'] = $_SESSION['store_customerID'];
	$_POST['info']['from_edit'] = YES;
	$cust->pull_post();
	$customerID = $cust->add(YES);

	if ($cust->was_added())
	{
		// the customer was edited - re-pull the info for the session
		$log = new login(ONLINE);
		$log->checkLogin($_SESSION['storeID'],'','',$_SESSION['store_customerID']);

		$return = '/account.php?saved=' . YES;
		unset($_SESSION['register_info']);
	}
	else
	{
		$return = '/edit_info.php?action=info&invalid=' . YES;
		$_SESSION['register_errors'] = $cust->show_errors(YES);
	}
}
elseif ($act == 'set_password')
{
	$dbcurrentpass = $cust->getCurrentPassword($_SESSION['store_customerID']);
	$currentpass = getP('currentpass');
	$newpass = getP('newpass');
	$newpass_verify = getP('newpass_verify');

	$errors = array();
	if (!strlen($currentpass) || !strlen($newpass) || !strlen($newpass_verify))
	{
		$errors[] = 'Incomplete entry - please fill out all fields';
	}
	elseif ($dbcurrentpass != $currentpass)
	{
		$errors[] = 'Current password is incorrect';
	}
	elseif ($newpass != $newpass_verify)
	{
		$errors[] = 'New passwords do not match';
	}
	else
	{
		// valid input - save the password
		$cust->savePassword($_SESSION['store_customerID'],$newpass);
	}

	if (count($errors))
	{
		$return = '/edit_info.php?action=password&invalid=' . YES;
		$_SESSION['register_errors'] = $errors;
	}
	else
	{
		$return = '/account.php';
	}
}

$pg->showUpdating('Updating Your Account...',$return);
?>