<?php
/*
* Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
$check_login = true;
include('include/include.inc');

$pg = new page();
$pg->setTitle('Manage Payment Methods');
$pg->head('Manage Payment Methods');

// get the credit cards
$cc = new credit_card($pg);
$cc->setCards();
$cards = $cc->getCards();

// if applicable, show a status
if (getG('added')) { $pg->status('Payment method successfully added'); }
if (getG('edited')) { $pg->status('Payment method successfully edited'); }
if (getG('deleted')) { $pg->status('Payment method successfully deleted'); }

?>
<script type="text/javascript">
	function deleteCard(cardID)
	{
		if (confirm('Are you sure you want to delete this credit/debit card?\nOnce deleted, you cannot retrieve or use this card!\n\nOK = delete card; Cancel = do not delete card'))
		{
			document.location = '/creditcardUpdate.php?act=delete&cardID=' + cardID;
		}
	}
</script>

<table border="0" cellspacing="1" cellpadding="3" width="95%" class="payment_methods">
	<tr>
		<td colspan="2">
			<span class="orange_label">&raquo; Credit/Debit Cards</span>
		</td>
	</tr>
	<?php
	if (!count($cards))
	{
		?>
		<tr class="<?=getRowClass(0);?>">
			<td colspan="2">
				You do not have any credit/debit cards saved on your account.<br />
				<a href="/card_add.php">Click here</a> to add a credit/debit card to your account.
			</td>
		</tr>
		<?php
	}

	$idx = -1;
	while (list($cardID,$arr) = each($cards))
	{
		$idx++;

		?>
		<tr class="<?=getRowClass($idx);?>">
			<td>
				<b>Payment Method #<?=($idx+1);?></b><br />
				<?="{$arr['crc_fname']} {$arr['crc_lname']}";?><br />
				<?php
				if (strlen($arr['crc_companyname'])) { ?><?=$arr['crc_companyname'];?><br /><?php }
				?>
				<?=$arr['cct_name'];?> - <?=$cc->getMasked($arr['crc_number'],YES);?><br />
				Expires: <?=$arr['crc_expmonth'];?>/<?=$arr['crc_expyear'];?><br />
				<img src="/images/blank.gif" width="1" height="10" /><br />
				<a href="/card_edit.php?cardID=<?=$cardID;?>">Edit Card</a> |
				<a href="javascript:deleteCard(<?=$cardID;?>)">Delete Card</a>
			</td>
		</tr>
		<?php

		if (($idx+1) < count($cards))
		{
			?>
			<tr>
				<td align="left">
					<hr width="100%" size="1" color="#CCCCCC" />
				</td>
			</tr>
			<?php
		}
	}
	?>
</table>

<p />
<img src="/images/blank_black.gif" width="550" height="1" />
<p />

<div class="left">
	<a href="/card_add.php"><img src="/images/btn_addcard.gif" width="151" height="18" border="0" alt="Add Credit/Debit Card" /></a>
	<p />
	<a href="/checkout.php?calc=<?=YES;?>"><img src="/images/btn_proceed.gif" width="144" height="18" border="0" alt="Proceed to Checkout" /></a>
</div>
<?php

$pg->foot();
?>