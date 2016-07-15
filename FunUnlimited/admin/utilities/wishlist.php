<?php
include('../../include/include.inc');

$cl = new check_login();

$act = getGP('act');
if (!strlen($act) && isset($_GET['customerID'])) { $act = 'select'; }
$page = getGP('page',1,range(1,1000));

$pg = new admin_page();
$pg->setTitle('Wishlist');
$pg->head('Wishlist');

$error = new error('Wishlist');

$wsh = new wishlist($pg);

if (getP('numcalls'))
{
	$wsh->update_num_calls(
		getP('numcalls'),
		getP('orignumcalls')
	);
	$act = '';
}
if (getP('remove_num_called'))
{
	$wsh->remove_num_called(getP('numcalledmax'));
	$act = '';
}

if ($act == '')
{
	// output the add new form and the first page of wishlist items
	$wsh->main_page($page);
}
elseif ($act == 'limit')
{
	$wsh->item_search_form(true);
}
elseif ($act == 'limit_search')
{
	// view wishlist items that match the search
	$wsh->pull_post();
	$wsh->item_search();
	$wsh->item_results();
}
elseif ($act == 'limit_select')
{
	// show the wishlists that contain the selected item
	$itemIDs = getG('itemID');
	$wsh->showWithItem($itemIDs);
}
elseif ($act == 'search')
{
	$wsh->pull_post();
	if (isset($wsh->criteria['phone']))
	{
		// perform customer search and output results
		$wsh->customer_search();
		$wsh->customer_results();
	}
	else
	{
		// perform item search and output results
		$wsh->show_customer();
		$wsh->item_search();
		$wsh->item_results();
	}
}
elseif ($act == 'select')
{
	if (getG('customerID'))
	{
		$_POST['customerID'] = getG('customerID');
	}

	if (getP('customerID'))
	{
		// the customer has been selected - show the item search form
		$wsh->set_customerID(getP('customerID'));
		$wsh->show_customer();
		$wsh->item_search_form();
	}
	elseif (getG('itemID'))
	{
		// the item has been selected - add it to the customer's wishlist and show the main page
		$itemIDs = getG('itemID');
		$status = array();
		while (list($a,$itemID) = each($itemIDs))
		{
			$status[] = $wsh->add($itemID);
		}
		$pg->status($status);
		$wsh->reset_vars();
		$wsh->main_page();
	}
}
elseif ($act == 'delete')
{
	// delete an item from a wishlist
	$wsh->delete(getP('wishlistID'),getP('itemID'));
	$wsh->main_page();
}
elseif ($act == 'delete_all')
{
	// delete ALL items from a customer's wishlist
	$wsh->deleteAllItems(getG('wishlistID'));
	$wsh->main_page();
}
elseif ($act == 'viewinstock')
{
	// view wishlist items that are in stock
	$wsh->show_in_stock();
}
elseif ($act == 'email')
{
	// output the email body contents for the user to edit
	$wsh->pull_post();
	$wsh->email_form();
}
elseif ($act == 'sendemail')
{
	// send the email
	$wsh->pull_post();
	$wsh->send_email();
	$wsh->show_in_stock();
}
else
{
	?><b>$_GET</b><pre><?=print_r($_GET);?></pre><?php
	?><b>$_POST</b><pre><?=print_r($_POST);?></pre><?php
	?><b>$_SESSION</b><pre><?=print_r($_SESSION);?></pre><?php
}

$pg->foot();
?>