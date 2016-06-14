<?php
/*
* Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include('include/include.inc');

$from_register = getG('from_register');
if ($from_register)
{
	$_POST = $_SESSION['register_login_post'];
	unset($_SESSION['register_login_post']);
}

$act = getGP('act');
$return = getGP('return');
if (!strlen($return)) { $return = '/cart.php'; }
$error = new error('Login');

$log = new login(ONLINE);

if ($act == 'login')
{
	$user = getP('user');
	$pass = getP('pass');
	$return = getP('return');

	$valid = $log->checkLogin(NONE,$user,$pass);

	if ($valid)
	{
		$_SESSION['store_loggedin'] = YES;
		$_SESSION['store_customerID'] = $_SESSION['userinfo']['cus_customerID'];
		$_SESSION['store_name'] = "{$_SESSION['userinfo']['cus_fname']} {$_SESSION['userinfo']['cus_lname']}";

		/*
		!!! MAKE SURE THE USER HAS A WISHLIST !!!

		check/change their cart status
		if cart has:
			a) no items
					check for:
						1) Non-completed previous invoice - reopen invoice, delete current
						2) No previous - change customerID of current invoice
			b) items - change customerID of current invoice and delete old invoices
		*/

		/*
		$update_current = NO;

		// find previous invoices
		$sql = "SELECT inv_invoiceID FROM invoices WHERE inv_customerID={$_SESSION['store_customerID']} AND inv_locale=".ONLINE." AND inv_completed=".NO." ORDER BY inv_time DESC";
		$result = mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);

		if (!count($_SESSION['cust_items']))
		{
			if (mysql_num_rows($result))
			{
				// found invoice - delete the current invoice and load the last previously open invoice (there should be 1 max...EVER!)
				$row = mysql_fetch_assoc($result);
				$deleteID = $_SESSION['cust_invoiceID'];
				$loadID = $row['inv_invoiceID'];

				$inv = new invoice($pg);
				$inv->set_invoiceID($deleteID,NO);
				$inv->delete();
				$inv->set_invoiceID($loadID);
			}
			else
			{
				// no invoices - update the current invoice
				$update_current = YES;
			}
		}
		else
		{
			// items in current invoice - update
			$update_current = YES;
		}

		if ($update_current)
		{
			// delete all old, non-completed invoices
			$invoiceIDs = array();
			while ($row = mysql_fetch_assoc($result)) { $invoiceIDs[] = $row['inv_invoiceID']; }

			if (count($invoiceIDs))
			{
				$sql = "DELETE FROM invoices WHERE inv_invoiceID IN (".implode(',',$invoiceIDs).")";
				mysql_query($sql,$db);
				$error->mysql(__FILE__,__LINE__);

				$sql = "DELETE FROM invoice_items WHERE ini_invoiceID IN (".implode(',',$invoiceIDs).")";
				mysql_query($sql,$db);
				$error->mysql(__FILE__,__LINE__);
			}

			// update the current invoice
			$sql = "UPDATE invoices SET inv_customerID={$_SESSION['store_customerID']} WHERE inv_invoiceID={$_SESSION['cust_invoiceID']}";
			mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);

			$inv = new invoice($pg);
			$inv->set_invoiceID($_SESSION['cust_invoiceID']);
		}
		*/

		$return = (strlen($return) ? $return : ($from_register ? '/catalog.php' : '/index.php'));
	}
	else
	{
		$return = '/login.php?invalid=' . YES . "&user=$user&return=$return";
	}
}

$pg = new page();
$pg->showUpdating('Logging You In...',$return);
?>