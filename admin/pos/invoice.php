<?php
include('../../include/include.inc');

$cl = new check_login();

$act = getGP('act');
if (in_array($act,array('','new','newinvoice','viewopen')) && @$_SESSION['cust_invoiceID']) { $act = 'view'; }
if ($act == 'newinvoice' && !@$_SESSION['cust_invoiceID'])
{
	$act = 'makenew';
}
if ($act == 'viewopen' && !@$_SESSION['cust_invoiceID'])
{
	$act = 'dosearch';
	$criteria = array('completed' => NO);
	$openifone = YES;
}
else
{
	$openifone = NO;
}

$head_acts = array('','new','makenew','search','view');

$pg = new admin_page();
$cust = new customers($pg);
$inv = new invoice($pg);

if ($act == 'makenew') { $_SESSION['cust_invoiceID'] = 1; } // trick the POS menu

if (in_array($act,$head_acts))
{
	$head_shown = YES;
	$titles = array(
		''        => 'Customer Invoice',
		'view'    => 'Customer Invoice',
		'new'     => 'New Invoice',
		'makenew' => 'New Invoice',
		'search'  => 'Search Previous Invoices'
	);

	$pg->setTitle(@$titles[$act]);
	$pg->head(@$titles[$act],YES);
}
else { $head_shown = NO; }

if ($act == 'makenew') { unset($_SESSION['cust_invoiceID']); } // un-trick the POS menu

if ($act == '' || $act == 'new')
{
	// display the new invoice type selection
	?>
	<form method="post" action="invoice.php">
		<input type="hidden" name="act" value="">
		<input type="button" value="New Invoice &gt;" onclick="this.form.act.value='newinvoice';this.form.submit()" class="btn">
		<p />
		<input type="button" value="Re-open Existing Invoice &gt;" onclick="this.form.act.value='viewopen';this.form.submit()" class="btn">
	</form>
	<?php
}
elseif ($act == 'makenew')
{
	// create a new invoice (generate invoiceID)
	?>
	<font size="1">
		<b>Note:</b> If you didn't want to create an invoice for this customer,
		uncheck the <b>Close Customer</b> box and press <b>Cancel Invoice</b> below.
	</font>
	<p />
	<?php
	$inv->create_new();
	$inv->view();
}
elseif ($act == 'search')
{
	// output the invoice search page
	$inv->search_form('invoice.php');
}
elseif ($act == 'dosearch')
{
	// perform/output invoice search
	if (@$_GET['last'])
	{
		$criteria = $_SESSION['cust_invoice_criteria'];
	}

	if (strlen(@$_GET['quick']))
	{
		// QUICK CAN NO LONGER HAPPEN - SALES AND TRADES ARE NOT SEPARATE
		if ($_GET['quick'] == "sales") { $type = SALE; } else { $type = TRADE; }
		$criteria = array('completed'=>YES,'type'=>$type,'locale'=>"INSTORE");
	}

	if (isset($criteria)) { $inv->criteria = $criteria; }
	else { $inv->pull_post(); }

	$_SESSION['cust_invoice_criteria'] = $inv->criteria;

	$inv->search();
	$inv->search_results('invoice.php',$openifone);
}
elseif ($act == 'view')
{
	// view the current invoice
	$inv->set_invoiceID($_SESSION['cust_invoiceID'],YES);
	$inv->view();

	if (@$_SESSION['do_quickadd'])
	{
		$type = $_SESSION['quickadd_type'];
		$pg->addOnload("doQuickAdd($type)");
	}

	//$inv->t->viewTimes(); // uncomment to view the timer table
}
elseif ($act == 'close')
{
	// close/complete the current invoice
	if (isset($_POST['invoiceID']))
	{
		$inv->set_invoiceID($_POST['invoiceID'], NO);
	}

	$complete = @$_GET['complete'];
	$cancel = @$_GET['cancel'];
	$print_tradeID = @$_GET['print_tradeID'];
	$cco = @$_SESSION['cust_close_options'];

	$wsh = new wishlist($pg);

	if ($complete)
	{
		$wsh->deleteSaleItems();
		$wsh->checkInvoiceItems();
	}

	$inv->close($complete, $cancel);

	if ($_SESSION['close_after_complete'] == YES)
	{
		// redirect them to the select customer screen
		$url = $_SESSION['root_admin'] . 'pos/pos.php?act=new&print_tradeID=' . $print_tradeID;
	}
	else
	{
		// redirect them to the customer info screen
		$url = $_SESSION['root_admin'] . 'pos/pos.php?print_tradeID=' . $print_tradeID;
	}

	unset($_SESSION['invoice_focus_type']);
	unset($_SESSION['in500_last_customerID']);

	$pg->showUpdating('Invoice successfully closed...', $url);
}
elseif ($act == 'reopen')
{
	$inv->set_invoiceID($_POST['invoiceID']);
	$pg->showUpdating('Invoice successfully reopened...','/admin/pos/invoice.php?act=view');
}
elseif ($act == 'delete')
{
	unset($_SESSION['invoice_focus_type']);
	$inv->set_invoiceID($_POST['invoiceID']);
	$inv->delete();
	$pg->showUpdating('Invoice successfully deleted...','/admin/pos/invoice.php?act=dosearch&last='.YES);
}
else
{
	?>
	act: <?=$act;?>
	<p />
	<pre><b>$_POST</b><br /><?=print_r($_POST);?></pre>
	<pre><b>$_GET</b><br /><?=print_r($_GET);?></pre>
	<pre><b>$_SESSION</b><br /><?=print_r($_SESSION);?></pre>
	<?php
}

if (@$_GET['alert'])
{
	?>
	<script type="text/javascript">
		function chglocalert()
		{
			alert('There is a customer with an invoice that was not closed during the last session.\nThis customer will now be activated, and the invoice reopened.\n\nPlease be sure to close invoices before closing your browser or logging out!');
		}
	</script>
	<?php
	$pg->addOnload('chglocalert()');
}

$inv->t->stopTimer('invoice_class');
//$inv->t->viewTimes();

if ($head_shown) { $pg->foot(); }
?>