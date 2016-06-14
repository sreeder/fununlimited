<?php
/*
* Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include_once('include/include.inc');

$act = getGP('act');

$pg = new page();
$cc = new credit_card($pg);

$return = '/pay_methods.php';

if ($act == 'add')
{
	// add/edit a credit card
  $added = $cc->addCard();
  if ($added)
  {
  	$return = '/pay_methods.php?' . (getP('cardID') ? 'edited' : 'added') . '=' . YES;
  }
  else
  {
  	$return = '/card_add.php?error_code=' . $cc->getErrorCode();
  }
}
elseif ($act == 'delete')
{
	$cc->deleteCard(getG('cardID'));
	$return = '/pay_methods.php?deleted=' . YES;
}

$pg->showUpdating('Updating credit card...',$return);
?>