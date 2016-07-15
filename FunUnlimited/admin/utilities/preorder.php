<?php
include('../../include/include.inc');

$cl = new check_login();

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
if ($act == "" && @$_SESSION['requestID']) { $act = 'new'; }

$pg = new admin_page();
$pg->setTitle('Preorders');
$pg->head('Preorders');

$error = new error('Preorders');

$pre = new preorder($pg);

?>This feature is not available at this time.<?php
$pg->foot();
die();

if ($act == "")
{
	// output list of current products with preorders
	$pre->pull_post();
	$pre->show_list();
}
elseif ($act == "new")
{
	// show the preorder item search screen
	?>Search below for the item you would like to add to the preorder.<p /><?php
	$pre->item_form();
}
elseif ($act == "search")
{
	// perform the item search
	$its = new item_search($pg);
	$its->action = $_SESSION['root_admin'].'utilities/preorder.php';
	$its->max_results = 250;
	$its->pull_post();
	$results = $its->search();

	if (!count($results))
	{
		echo "Your search did not match any items. Please try again.<p />";
		$pre->item_form();
	}
	elseif (count($results) > 1)
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
		<?=$only2;?>Click <a href="/admin/utilities/preorder.php?act=new">here</a> to search again.
		<?php
		$its->showSmallResults(NO);
	}
	else
	{
		$pre->set_itemID($results[0]['itm_itemID']);
		$pre->item_info_form();
	}
}
elseif ($act == "select")
{
	// use itemID from search
	$pre->pull_post();
	$pre->item_info_form();
}
elseif ($act == "createnew")
{
	// add the new item and show the customer entry form
	$pre->pull_post();
	$pre->add_item();
	$pre->customer_form('New preorder item created');
}
elseif ($act == "edit")
{
	// show the edit preorder item info screen
	$pre->pull_post();
	$pre->item_info_form(YES);
}
elseif ($act == "edititem")
{
	// edit the item and show the customer entry form
	$pre->pull_post();
	$pre->add_item(YES);
	$pre->customer_form('Edited preorder item information');
}
elseif ($act == "view")
{
	// view the customer entry form (active)/customer info form (completed)
	$pre->pull_post();
	$pre->customer_form();
}
elseif ($act == "addcustomer")
{
	// add a customer to the current preorder
	$pre->pull_post();
	$pre->add_customer();
	$pre->set_preorderID($pre->preorderID);
	$pre->customer_form('Customer added to preorder');
}
elseif ($act == "remcustomer")
{
	// remove a customer from the current preorder
	$pre->pull_post();
	$pre->remove_customer();
	$pre->set_preorderID($pre->preorderID);
	$pre->customer_form('Customer removed from preorder');
}
elseif ($act == "close")
{
	// close the active preorder
	$pre->pull_post();
	$pre->show_list();
}
elseif ($act == "complete")
{
	// shows the customer status screen for the preorder
	$pre->pull_post();
	$pre->complete_form();
}
elseif ($act == "docomplete")
{
	// completes a preorder
	$pre->pull_post();
	$msg = $pre->complete();
	$pre->show_list($msg);
}
elseif ($act == "delete")
{
	// delete a preorder
	$pre->pull_post();
	$pre->dodelete();
	$pre->show_list('Preorder deleted');
}

$pg->foot();
?>