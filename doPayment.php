<?php
/*
* Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include_once('include/include.inc');

$pg = new page();
$act = getP('act');
$key = getP('key');
$return = '/cart.php';

if ($act == 'process' && $key == md5($_SESSION['store_customerID']))
{
	$cart = new cart($pg);
	$success = $cart->processOrder();

	if ($success)
	{
		// redirect to order_complete.php to finalize the order
		$return = '/order_complete.php';
	}
	else
	{
		$return = '/review_order.php?error=' . YES;
	}
}

$pg->showUpdating('Processing your credit card; <b><u>DO NOT</u></b> stop or reload this page!',$return);
?>