<?php
/*
* Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
$check_login = true;
include('include/include.inc');

if (!$_SESSION['cart_qty']) { headerLocation('/cart.php'); }

$pg = new page();
$cart = new cart($pg);

if (getG('calc'))
{
	// recalculate the totals
	$cart->setCartItems();
}

// get the credit cards
$cc = new credit_card($pg);
$cc->setCards();
$cards = $cc->getCards();

$pg->setTitle('Check Out');
$pg->head('Check Out');

?>
<div class="left">
	Please select your method of payment:
	<p />

	<?php
	if (!count($cards))
	{
		$pg->error('There were no payment methods found on your account.<br /><a href="/card_add.php">Click here</a> to add a credit/debit card to your account.');
	}
	else
	{
		?>
		<form method="post" action="/cartUpdate.php">
		<input type="hidden" name="act" value="set_method" />
		<b>Payment Method:</b>
		<select name="cardID" size="1">
			<?php
			while (list($cardID,$arr) = each($cards))
			{
				?><option value="<?=$cardID;?>"><?=$arr['cct_name'];?> - <?=$cc->getMasked($arr['crc_number'],YES);?> / <?="{$arr['crc_lname']}, {$arr['crc_fname']}";?></option><?php
			}
			?>
		</select>
		<input type="submit" value="Continue &gt;" class="btn" />
		</form>

		<p />
		<img src="/images/blank_black.gif" width="550" height="1" />
		<p />

		<a href="/pay_methods.php"><img src="/images/btn_paymethods.gif" width="178" height="18" border="0" alt="Manage Your Payment Methods" /></a>
		<p />
		<a href="/cart.php"><img src="/images/btn_returntocart.gif" width="110" height="18" border="0" alt="Return to Your Shopping Cart" /></a>
		<?php
	}

	?>
</div>
<?php

$pg->foot();
?>