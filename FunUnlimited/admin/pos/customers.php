<?php
include('../../include/include.inc');

$cl = new check_login();
$pg = new admin_page();
$cust = new customers($pg);

$pg->setTitle('Manage Customers');
$pg->head('Manage Customers',YES);

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$customerID = (isset($_GET['customerID'])?$_GET['customerID']:@$_POST['customerID']);

if ($act == "")
{
	headerLocation('/admin/pos/pos.php');
}
elseif ($act == "add" || $act == "edit")
{
	// display the add/edit customer form
	if (@$_GET['fill'] == YES) { $cust->criteria_to_info(); }

	if (!isset($active)) { $active = YES; }

	if ($customerID) { $cust->set_customerID($customerID); }
	$cust->add_form($active,@$_GET['return']);
}
elseif ($act == "doadd")
{
	// add/edit a customer
	$active = @$_POST['active'];
	$cust->pull_post();
	$newcustomerID = $cust->add();

	if ($cust->was_added())
	{
		if (!strlen($cust->return))
		{
			// set the customerID and redirect to the correct page
			$curcustID = $cust->customerID;
			$cust->set_customerID($newcustomerID);
			$_SESSION['last_customer'] = $cust->info;

			if (!$curcustID)
			{
				// create a new invoice
				$cust->return = "{$_SESSION['root_admin']}pos/invoice.php?act=newinvoice";
			}
			else
			{
				if (getP('after') == 'invoice')
				{
					// return to the current invoice
					$cust->return = "{$_SESSION['root_admin']}pos/invoice.php?act=view";
				}
				else
				{
					// return to the customer info
					$cust->return = "{$_SESSION['root_admin']}pos/pos.php";
				}
			}
		}

		$cust->return = str_replace('%customerID%',$newcustomerID,$cust->return);
		headerLocation($cust->return);
	}
	else
	{
		$cust->show_errors();
		$cust->add_form($active);
	}
}
elseif ($act == "activate")
{
	// re/de-activate a customer
	$active = @$_POST['active'];
	$cust->set_customerID($customerID);
	$cust->activate($active);

	if ($cust->was_deleted())
	{
		$pg->status(($active==NO?'Re':'De').'activated customer: <b>'.$cust->info['fname'].' '.$cust->info['lname'].'</b>');
		$cust->show_customers(($active==YES?NO:YES));
	}
	else { echo "There was an error...tell Scott - this shouldn't happen here..."; }
}
elseif ($act == "permdelete")
{
	// permanently delete a customer and all traces of it
	$cust->set_customerID($customerID);
	$cust->permdelete();

	if ($cust->was_deleted())
	{
		$pg->status('Permanently deleted customer (Scott - remove ALL values): <b>'.@$cust->info['fname'].' '.@$cust->info['lname'].'</b>');
		$cust->show_customers($active);
	}
	else { echo "There was an error...tell Scott - this shouldn't happen here..."; }
}
elseif ($act == "history")
{
	// output the customer history
	// (select date range and then display the history)
	echo "Customer History";
}

$pg->foot();
?>