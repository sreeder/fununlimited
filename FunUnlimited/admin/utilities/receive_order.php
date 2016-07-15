<?php
include('../../include/include.inc');

$cl = new check_login();

$act = getGP('act');
if (!in_array($act,array('','complete','add','search','delete','view')) && @$_SESSION['receive_orderID']) { $act = 'form'; }

$pg = new admin_page();
$pg->setTitle('Receive Order');
$pg->head('Receive Order');

$error = new error('Receive Order');

$ord = new receive_order($pg);

if ($act == '')
{
	// output the previous orders/generate new form button
	$ord->show_list(@$_GET['limit']);
}
elseif ($act == 'new')
{
	// create a new order and output the form
	$ord->create_new();
	$ord->form();
}
elseif ($act == 'reopen')
{
	// reopen a non-completed order
	$ord->set_orderID($_POST['orderID']);
	$ord->form();
}
elseif ($act == 'form')
{
	// output the form
	$ord->form();
}
elseif ($act == 'search')
{
	// perform the item search
	$_SESSION['receive_newused'] = getP('newused');
	$_SESSION['receive_last_platformID'] = getP('platformID');

	$its = new item_search($pg);
	$its->action = "{$_SESSION['root_admin']}utilities/receive_orderUpdate.php";
	$its->max_results = 250;
	$its->criteria['upctitle'] = getP('upctitle');
	$its->criteria['platformID'] = getP('platformID');
	$results = $its->search();

	if (!count($results))
	{
		$pg->error("Your search for title <b>{$_POST['upctitle']}</b> did not match any items. Please try again.");
		$ord->form();
	}
	else
	{
		$only1 = ''; $only2 = '';
		$count = count($results);
		if ($count > $its->max_results)
		{
			$only1 = " Only the first $its->max_results are shown.";
			$only2 = 'Please narrow your search criteria. ';

			$results = array_slice($results,0,$its->max_results);
			$its->results = $results;
		}

		?>
		<?=$count;?> item<?=($count==1?'':'s');?> matched your criteria.<?=$only1;?>
		<p />
		<?=$only2;?>Click <a href="/admin/utilities/receive_order.php?act=form">here</a> to search again/return to the order.
		<?php
		$its->showSmallResults(NO);
	}
}
elseif ($act == 'view')
{
	// view the details of a previous order
	$ord->view($_GET['orderID']);
}
elseif ($act == 'delete')
{
	// delete a previous order
	$orderID = (isset($_POST['orderID'])?$_POST['orderID']:$_SESSION['receive_orderID']);

	$ord->delete($orderID);
	$ord->show_list();
}
else
{
	?><pre><b>$_POST</b><br /><?=print_r($_POST);?></pre><?php
	?><pre><b>$_GET</b><br /><?=print_r($_GET);?></pre><?php
}

$pg->foot();
?>