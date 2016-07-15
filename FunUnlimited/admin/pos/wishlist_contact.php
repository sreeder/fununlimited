<?php
include('../../include/include.inc');

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Wishlist - Contact Customers');
$pg->head('Wishlist - Contact Customer');

$error = new error('Wishlist - Contact Customer');

if (count(@$_SESSION['invoice_wishlist_itemIDs']))
{
	?>
	Please contact the customers below - some items in their wishlists were just received into your inventory.
	<p />
	<font size="1"><b>Note:</b> An email has been sent to each customer below alerting them of the item<?=(count(@$_SESSION['invoice_wishlist_itemIDs'])==1?"'s":"s'");?> in-stock status.</font>
	<p />
	<?php

	$wsh = new wishlist($pg);
	$wsh->get_wishlists();
	$wsh->wishlists = $wsh->wishlist_filter($wsh->wishlists,$_SESSION['invoice_wishlist_itemIDs']);
	$wsh->send_emails($wsh->wishlists);
	$wsh->print_table($wsh->wishlists,'contact',1);
	unset($_SESSION['invoice_wishlist_itemIDs']);
}
else
{
	?>There are no newly in-stock items in any wishlists.<?php
}

?>
<p />
<input type="button" value="Close Window &gt;" onclick="window.close()" class="btn" />
<?php

$pg->foot();
?>