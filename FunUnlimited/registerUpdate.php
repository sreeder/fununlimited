<?php
/*
* Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include('include/include.inc');

$pg = new page();
$cust = new customers($pg);

$act = getGP('act');
$return = getP('return');
if (!strlen($return)) { $return = '/login.php'; }

if ($act == 'register')
{
	$cust->pull_post();
	$newcustomerID = $cust->add(YES);

	if ($cust->was_added())
	{
		// the customer was added - log them in
		$_SESSION['register_login_post'] = array(
			'act'    => 'login',
			'user'   => $_SESSION['register_info']['user'],
			'pass'   => $_SESSION['register_info']['pass'],
			'return' => $return,
		);
		$return = '/loginUpdate.php?from_register=' . YES;
		unset($_SESSION['register_info']);
	}
	else
	{
		$return = '/login.php?action=register&invalid=' . YES . '&return=' . $return;
		$_SESSION['register_errors'] = $cust->show_errors(YES);
	}
}

$pg->showUpdating('Registering Your Account...',$return);
?>