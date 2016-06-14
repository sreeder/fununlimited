<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Item Information');
$pg->head();

$itm = new items($pg);
$its = new item_search($pg);
$its->action = "{$_SESSION['root_admin']}bare/items.php";

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$itemID = (isset($_GET['itemID'])?$_GET['itemID']:@$_POST['itemID']);

// if they searched with the quick lookup box, catch it
if (isset($_GET['val']))
{
	$criteria['upctitle'] = $_GET['val'];
	$criteria['platformID'] = $_GET['platformID'];
	$_POST['criteria'] = $criteria;
	$act = 'search';
}

if ($act == "")
{
	// display the item search
	$pg->pageHead('Item Information');
	?>
	To view an item's pricing and information, enter as much criteria as possible.
	<p />
	<?php
	$its->form(YES,array(),NO);
}
elseif ($act == "search")
{
	$its->pull_post();
	$_SESSION['search_criteria'] = $its->criteria;

	if (isset($_GET['searchupc'])) { $its->criteria['upc'] = $_GET['searchupc']; $its->per_page = $_SESSION['search_per_page']; }

	$results = $its->search();

	if (!count($results) && count($its->criteria) == 2 && strlen(@$its->criteria['upc'])) { $onlyupc = YES; }
	else { $onlyupc = NO; }

	if (count($its->results) == 1)
	{
		// found 1 item
		$itemID = $its->results[0]['itm_itemID'];
	}
	elseif (count($its->results) > 1) { $multiple = YES; }
	else
	{
		// display the add page
		?>
		Your search returned no results. Please try again.
		<p />
		<?php
		$its->form(YES,array(),NO);
	}
}

if (@$multiple)
{
	$pg->pageHead('Select Item');

	$only = '';
	$count = count($its->results);
	if ($count >= $its->max_results)
	{
		$only = " Only the first $its->max_results are shown.";

		$its->results = array_slice($its->results,0,$its->max_results);
		$_SESSION['search_results'] = $its->results;
	}

	?>
	<?=$count;?> item<?=($count==1?'':'s');?> matched your criteria.<?=$only;?>
	<p />
	<input type="button" value="&lt; Search Again" onclick="document.location='/admin/bare/items.php'" class="btn" />
	<?php

	$its->showSmallResults(NO,array(),YES,NO);
}

if (strlen($itemID))
{
	// redirect to the item information page
	?>
	Please hold while you are redirected to the item information page.
	<p />
	<a href="/admin/pos/iteminfo.php?itemID=<?=$itemID;?>&frombare=<?=YES;?>">Click here</a> if you are not automatically redirected.
	<?php
	$pg->addOnload("document.location='/admin/pos/iteminfo.php?itemID=$itemID&frombare=" . YES . "'");
}

?>
<p />
<input type="button" value="&lt; Return to Barebones Utilities" onclick="document.location='/admin/bare/index.php'" class="btn" />
<?php

$pg->foot();
?>