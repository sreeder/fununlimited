<?php
/*
* Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include('include/include.inc');

$act = getGP('act');
$return = getGP('return');
if (!strlen($return)) { $return = '/cart.php'; }
$items = getGP('items');

$pg = new page();
$cart = new cart($pg);
$error = new error('Cart Update');

if ($act == 'add')
{
	// add an item to the cart
	$cart->addItem(getG('newused'),getG('itemID'));
	$return = '/cart.php?added=' . YES;
}
elseif ($act == 'remove')
{
	// remove an item from the cart
	$cart->removeItem(getG('newused'),getG('itemID'));
	$return = '/cart.php?removed=' . YES;
}
elseif ($act == 'recalc')
{
	// recalculate the cart items
	$qty = getP('qty');
	$checkout = getP('checkout');

	if (is_array($qty))
	{
		$cart->recalculateCartItems($qty);

		if (!$checkout)
		{
			$return = '/cart.php';
		}
		else
		{
			if ($_SESSION['cart_qty']) { $return = '/checkout.php?calc=' . NO; }
			else { $return = '/cart.php'; }
		}
	}
}
elseif ($act == 'set_method')
{
	// save the payment method
	$cardID = getP('cardID');
	if ($cardID)
	{
		$cart->setPaymentMethod($cardID);
		$return = '/review_order.php';
	}
}

$pg->showUpdating('Updating Shopping Cart',$return);
?>