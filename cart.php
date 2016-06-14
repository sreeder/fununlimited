<?php
/*
* Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include('include/include.inc');

$pg = new page();
$cart = new cart($pg);
$cart->setCartItems();
$items = $cart->getCartItems();

$pg->setTitle('View Shopping Cart');
$pg->head('View Shopping Cart');

if (getG('added'))
{
	$pg->status('The selected item has been added to your shopping cart!<br /><center><a href="/search.php?last=' . YES . '">&lt; Return to Your Search Results</a></center>');
}
if (getG('removed'))
{
	$pg->status('The selected item has been removed from your shopping cart');
}

if (@$_SESSION['cart_qty_note'])
{
	?>
	<font class="note_red">
		<b>Note:</b> We had to adjust the requested quantity for some of the items in your<br />
		shopping cart - you requested more than is available from our inventory at this time.
	</font>
	<p />
	<?php

	$_SESSION['cart_qty_note'] = NO;
}

if (!$_SESSION['cart_qty'])
{
	// no items
	?>
	You currently do not have any items in your shopping cart.
	<p />
	Enter some criteria below to search our inventory!
	<p />
	<?php

	$its = new item_search($pg);
	$its->setAction('search.php');
	$its->advanced_form();
}
else
{
	?>
	<script type="text/javascript">
		// recalculate the cart totals
		function doRecalc()
		{
			document.getElementById('cartForm').submit();
			return false;
		}

		// recalculate and proceed to checkout
		function doCheckout()
		{
			document.getElementById('cartForm').checkout.value = <?=YES;?>;
			return doRecalc();
		}
	</script>

	<form method="post" action="/cartUpdate.php" id="cartForm">
	<input type="hidden" name="act" value="recalc" />
	<input type="hidden" name="checkout" value="<?=NO;?>" />
	<table border="0" cellspacing="1" cellpadding="3" width="95%">
		<?php
		$colspan = 5;
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
					<td>&nbsp;</td>
				</tr>
				<?php

				$idx = -1;
				while (list($itemID,$arr) = each($nuitems))
				{
					$idx++;

					?>
					<tr class="<?=getRowClass($idx);?>">
						<td class="cart_title">
							<font size="2"><a href="javascript:iteminfo_window(<?=$arr['itm_itemID'];?>)"><b><?=$arr['itm_title'];?></b></a></font><br />
							<?=$arr['pla_name'];?> - <?=$arr['typ_type'];?>
						</td>
						<td align="right">
							$<?=$arr['cart_each'];?>
						</td>
						<td>
							<input type="text" name="qty[<?=$nu;?>][<?=$itemID;?>]" size="3" value="<?=$arr['cart_qty'];?>" />
						</td>
						<td align="right">
							$<?=$arr['cart_total'];?>
						</td>
						<td>
							<a href="/cartUpdate.php?act=remove&newused=<?=$nu;?>&itemID=<?=$itemID;?>"><img src="/images/remfromcart.gif" width="20" height="20" border="0" alt="Remove From Cart" /></a>
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

	<img src="/images/blank_black.gif" width="550" height="1" />
	<p />

	<table border="0" cellspacing="0" cellpadding="3" width="95%" class="cart_totals">
		<tr>
			<td rowspan="2" align="left">
				<a href="javascript:void(0)" onclick="return doRecalc()"><img src="/images/btn_recalc.gif" width="118" height="18" border="0" alt="Recalculate Totals" /></a>
				<p />
				<a href="javascript:void(0)" onclick="return doCheckout()"><img src="/images/btn_proceed.gif" width="144" height="18" border="0" alt="Proceed to Checkout" /></a>
			</td>
			<td align="right" valign="top">
				Total Items:<br />
				Total Price:
			</td>
			<td align="right" valign="top">
				<?=$_SESSION['cart_qty'];?><br />
				$<?=$_SESSION['cart_total'];?>
			</td>
		</tr>
	</table>

	</form>
	<?php
}

$pg->foot();
?>