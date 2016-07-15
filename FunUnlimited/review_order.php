<?php
/*
* Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
$check_login = true;
include('include/include.inc');

$cardID = @$_SESSION['cart_cardID'];
if (!$cardID) { headerLocation('/cart.php'); }
if (!$_SESSION['cart_qty']) { headerLocation('/cart.php'); }

$pg = new page();
$pg->setTitle('Review Your Order');
$pg->head('Review Your Order');

// get the shopping cart items
$cart = new cart($pg);
$cart->setCartItems();
$items = $cart->getCartItems();

// get the selected credit card info
$cc = new credit_card($pg);
$cc->setCards($cardID);
$card_info = $cc->getCards($cardID);

$error = getG('error');
if ($error)
{
	$pg->status('There was an error while processing your credit card:<br />' . $_SESSION['payment_error']);
}

?>
<form method="post" action="/doPayment.php" id="processForm">
<input type="hidden" name="act" value="process" />
<input type="hidden" name="key" value="<?=md5($_SESSION['store_customerID']);?>" />

<div class="left">
	Please review your order:
	<p />

	<b>Total Price:</b> $<?=number_format($_SESSION['cart_total'],2);?><br />
	<b>Total Items:</b> <?=$_SESSION['cart_qty'];?><br />
	<b>Payment Method:</b> <?=$card_info['cct_name'];?> - <?=$cc->getMasked($card_info['crc_number'],YES);?> / <?="{$card_info['crc_lname']}, {$card_info['crc_fname']}";?>

	<p />

	<table border="0" cellspacing="1" cellpadding="3" width="95%">
		<?php
		$colspan = 4;
		while (list($nu,$nuitems) = each($items))
		{
			if (count($nuitems))
			{
				?>
				<tr>
					<td colspan="<?=$colspan;?>">
						<span class="orange_label">&raquo; <?=getNU($nu);?> Items</span>
					</td>
				</tr>
				<tr class="cart_label">
					<td>Item</td>
					<td>Price Each</td>
					<td>Qty</td>
					<td>Total Price</td>
				</tr>
				<?php

				$idx = -1;
				while (list($itemID,$arr) = each($nuitems))
				{
					$idx++;

					?>
					<tr class="<?=getRowClass($idx);?>">
						<td class="cart_title">
							<font size="2"><b><?=$arr['itm_title'];?></b></font><br />
							<?=$arr['pla_name'];?> - <?=$arr['typ_type'];?>
						</td>
						<td align="right">
							$<?=$arr['cart_each'];?>
						</td>
						<td align="right">
							<?=$arr['cart_qty'];?>
						</td>
						<td align="right">
							$<?=$arr['cart_total'];?>
						</td>
					</tr>
					<?php
				}

				?>
				<tr><td colspan="<?=$colspan;?>">&nbsp;</td></tr>
				<?php
			}
		}
		?>
	</table>

	<p />
	Please press the <b>Process Order</b> button only <b><u>ONCE</u></b>!<br />
	Pressing it multiple times could result in multiple charges to your credit card!
	<p />

	<input type="submit" value="Process Order &gt;" class="btn" onclick="disableButton(this)" />

	<p />
	<img src="/images/blank_black.gif" width="550" height="1" />
	<p />

	<a href="/cart.php"><img src="/images/btn_returntocart.gif" width="110" height="18" border="0" alt="Return to Your Shopping Cart" /></a>
</div>
</form>
<?php

$pg->foot();
?>