<?php
include('../../include/include.inc');

$cl = new check_login();

$act = getP('act');
if (isset($_POST['invoiceID']))
{
	$_SESSION['cust_invoiceID'] = $_POST['invoiceID'];
}

$pg = new admin_page();
$cust = new customers($pg);
$inv = new invoice($pg);
$inv->set_invoiceID();

$error = new error('Invoice Payment');

if ($act == '')
{
	$pg->setTitle('Invoice Payment Options');
	$pg->head('Invoice Payment Options',YES);

	if (!@$_SESSION['cust_invoiceID'])
	{
		$error->show('There is no open invoice',NO);
		?>
		<input type="button" value="&lt; Return to Invoice" onclick="document.location='/admin/pos/invoice.php'" class="btn">
		<?php
	} // if no invoice
	else
	{
		$total_sale_items = 0;
		$total_sale = 0;
		$total_cash = 0;
		$total_credit = 0;

		while (list($a,$arr) = each($_SESSION['cust_items']))
		{
			if ($arr['ini_type'] == SALE)
			{
				$total_sale_items++;
				$total_sale += $arr['ini_price'];
			}
			elseif (($arr['ini_type'] == TRADE || $arr['ini_type'] == RETURNS) && $arr['ini_trade_type'] == CASH)
			{
				$total_cash += $arr['ini_price'];
			}
			elseif (($arr['ini_type'] == TRADE || $arr['ini_type'] == RETURNS) && $arr['ini_trade_type'] == CREDIT)
			{
				$total_credit += $arr['ini_price'];
			}
		} // each item
		reset($_SESSION['cust_items']);

		// output the payment options screen
		$cust->set_customerID($_SESSION['customerID']);
		$cust_credit = $cust->info['creditamount'];

		$inv->show_payment_options(
			$total_sale,
			$total_cash,
			$total_credit,
			$cust_credit
		);
	} // else open invoice

	$pg->foot();
} // if show options
elseif ($act == 'complete')
{
	$_SESSION['cust_close_options'] = getP();

	$has_trade = NO;
	while (list($a,$arr) = each($_SESSION['cust_items']))
	{
		if ($arr['ini_type'] == TRADE)
		{
			$has_trade = YES;
			break;
		}
	}
	reset($_SESSION['cust_items']);

	if ($has_trade)
	{
		$goto = '/admin/pos/invtradeinfo.php';
	}
	else
	{
		$goto = '/admin/pos/invoice.php?act=close&complete=' . YES;
	}

	$pg->showUpdating('Updating invoice...',$goto);
} // elseif complete
?>