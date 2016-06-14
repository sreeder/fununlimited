<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$pg = new admin_page();
$pg->setTitle('Manage Items');
$pg->setFull((getGP('popup') ? NO : YES));
$pg->head('Manage Items');

$itm = new items($pg);
$its = new item_search($pg);

$act = getGP('act');
$addupc = @$_GET['addupc'];
$setupc = getGP('setupc');
$its->findingmatch = getGP('findingmatch');
$itm->info['upc'] = $its->findingmatch;

if ($act == "")
{
	// display the item search
	search_form($its);
	$pg->cancel($_SESSION['root_admin'].'index.php','Home Page');
}
elseif ($act == "search")
{
	$its->pull_post();
	$_SESSION['search_criteria'] = $its->criteria;

	if (isset($_GET['searchupc'])) { $its->criteria['upc'] = $_GET['searchupc']; $its->per_page = $_SESSION['search_per_page']; }

	$results = $its->search();

	if (!count($results) && count($its->criteria) == 2 && strlen(@$its->criteria['upc'])) { $onlyupc = YES; }
	else { $onlyupc = NO; }

	if (!count($results) && $onlyupc)
	{
		// they searched for only the UPC - ask if they'd like to link it to an item in the database
		$its->findingmatch = $its->criteria['upc'];
		search_form($its);
		$pg->cancel($_SESSION['root_admin'].'setup_items/items.php','Search Page');
	}
	elseif (!count($results) && !$onlyupc)
	{
		// display the add page
		$key = md5(time());

		$_SESSION['add_criteria'] = $its->criteria;
		$_SESSION['add_key'] = $key;
		?>
		Your search returned no results.
		<p />
		Please <a href="items.php">click here</a> to search again, <?php
		if ($its->criteria['active'])
		{
			?>or add the item below.
			<p /><hr width="75%" size="-1" color="#CCCCCC"><p />
			<?php
			$itm->platform_form("{$_SESSION['root_admin']}setup_items/items.php",'',$key,@$its->criteria['platformID']);
			$pg->cancel($_SESSION['root_admin'].'setup_items/items.php','Search Page');
		}
		else
		{
			?>or <a href="items.php?act=add">click here</a> to go to the add item screen.<?php
		}

		if (strlen($its->findingmatch))
		{
			?><p /><a href="items.php?act=add&addupc=<?=$its->findingmatch;?>">Click here</a> to add an item with the UPC <b><?=$its->findingmatch;?></b>.<?php
		}
	}
	else
	{
		// display the search results
		$its->page = 1;
		$its->showResults();
		$pg->cancel($_SESSION['root_admin'].'setup_items/items.php','Search Page');
	}
}
elseif ($act == "page" || $act == "lastsearch")
{
	$its->criteria = $_SESSION['search_criteria'];
	$its->page = ($act=='page' ? getGP('page') : $_SESSION['search_page']);
	$its->results = $_SESSION['search_results'];
	$its->per_page = $_SESSION['search_per_page'];
	$its->showResults(@$_GET['deleteID']);
	$pg->cancel($_SESSION['root_admin'].'setup_items/items.php','Search Page');
}
elseif ($act == "add")
{
	// display the select platform form (when adding an item)
	$itm->info['upc'] = $addupc;
	$itm->platform_form("{$_SESSION['root_admin']}setup_items/items.php",'','',$setupc,@$_GET['fromreceiveorder'],@$_GET['fromreturns']);
	$pg->cancel($_SESSION['root_admin'].'setup_items/items.php','Search Page');
}
elseif ($act == "selplatform")
{
	// platform selected, show the add form
	$itm->info['upc'] = $setupc;
	$itm->info['platformID'] = (isset($_GET['platformID'])?$_GET['platformID']:@$_POST['platformID']);
	$itm->add_form('','',@$_GET['frominvoice'],@$_GET['fromupc'],@$_GET['fromqty'],@$_POST['fromreceiveorder'],@$_POST['fromreturns']);
	$pg->cancel($_SESSION['root_admin'].'setup_items/items.php','Search Page');
}
elseif ($act == "edit")
{
	$itemID = getGP('itemID');
	$itm->set_itemID($itemID);
	if (strlen($its->findingmatch))
	{
		$itm->info['upc'] = $its->findingmatch;
	}
	$itm->add_form(
		(strlen($its->findingmatch) ? 'To assign the provided UPC to this item, press <b>Edit Item</b>. Not the correct item? <a href="items.php?act=lastsearch&findingmatch=' . $its->findingmatch . '">Click here.</a>' : ''),
		'',
		getG('frominvoice'),
		getG('fromupc'),
		getG('fromqty'),
		NO,
		NO,
		getG('popup')
	);
	if (!getG('popup'))
	{
		$pg->cancel($_SESSION['root_admin'].'setup_items/items.php?act=lastsearch'.(strlen($its->findingmatch)?"&findingmatch=$its->findingmatch":''),'Search Results');
	}
}
elseif ($act == "doadd" || $act == "doedit")
{
	// add/edit an item (or refresh, if needed)
	$refresh = getP('refresh');
	$refreshto = getP('refreshto');
	$frominvoice = getP('frominvoice');
	$fromupc = getP('fromupc');
	$fromqty = getP('fromqty');
	$fromreceiveorder = getP('fromreceiveorder');
	$fromreturns = getP('fromreturns');
	$popup = getP('popup');
	$itm->pull_post();

	if ($refresh)
	{
		// redisplay the add form (happens after clicking "Add * >" on the add/edit form)
		$itm->add_form("",$refreshto,$frominvoice,$fromupc,$fromqty,$fromreceiveorder,$fromreturns,$popup);
	}
	else
	{
		$newitemID = $itm->add();

		if ($itm->was_added())
		{
			if ($frominvoice || $fromupc || $fromqty || $fromreceiveorder || $fromreturns || $popup)
			{
				$location = '';
				$js = '';
				if ($frominvoice)
				{
					$location = "{$_SESSION['root_admin']}pos/invoice.php?act=view";
				}
				elseif ($fromupc)
				{
					$location = "{$_SESSION['root_admin']}setup_items/enterupc.php?act=showitems&platformID={$itm->info['platformID']}";
				}
				elseif ($fromqty)
				{
					$location = "{$_SESSION['root_admin']}setup_items/begqty.php?act=showitems&platformID={$itm->info['platformID']}&page={$_SESSION['begqty_page']}&itemID=$newitemID";
				}
				elseif ($fromreceiveorder)
				{
					$location = "{$_SESSION['root_admin']}utilities/receive_order.php?itemID=$newitemID&newused={$_SESSION['receive_newused']}";
				}
				elseif ($fromreturns)
				{
					$location = "{$_SESSION['root_admin']}pos/returns.php?itemID=$newitemID&newused={$_SESSION['return_newused']}";
				}
				elseif ($popup)
				{
					$js = array(
						'window.opener.document.location.reload()',
						'self.close()'
					);
				}

				if (strlen($location))
				{
					// redirect to a page
					$pg->showUpdating('Adding/Editing Item',$location);
				}
				else
				{
					// run the javascript
					?>
					<script type="text/javascript">
						<?php echo implode(';',$js);?>;
					</script>
					<?php
				}
			}
			else
			{
				// show the search form
				search_form($its,($itm->itemID?'Edited':'Added').' item: <b>'.$itm->info['title'].'</b>');
			}
		}
		else
		{
			$itm->add_form("",$refreshto,$frominvoice,$fromupc,$fromqty,$fromreceiveorder,$fromreturns,$popup);
		}
	}
	if (($refresh || ($refresh == NO && $frominvoice != YES && $fromupc != YES && $fromqty != YES && $fromreceiveorder != YES && $fromreturns != YES)) && !$popup)
	{
		$pg->cancel($_SESSION['root_admin'].'setup_items/items.php','Search Page');
	}
}
elseif ($act == "delete")
{
	// delete an item (set is as inactive)
	$itemID = (isset($_GET['itemID'])?$_GET['itemID']:@$_POST['itemID']);
	$itm->set_itemID($itemID);
	$itm->delete();

	if ($itm->was_deleted())
	{
		$pg->status('Deleted item: <b>'.$itm->info['title'].'</b>');
		?>
		<script type="text/javascript">function gotosearch() { document.location='/admin/setup_items/items.php?act=lastsearch&findingmatch=<?=$its->findingmatch;?>&deleteID=<?=$itemID;?>'; }</script>

		Please hold while you are redirected...
		<p />
		Click <a href="javascript:gotosearch()">here</a> if you are not automatically redirected.
		<?php

		$pg->addOnload('gotosearch()');
	}
	else { echo "There was an error...tell Scott - this shouldn't happen here..."; }
}
elseif ($act == "undelete")
{
	// undelete an item (set is as active)
	$itemID = (isset($_GET['itemID'])?$_GET['itemID']:@$_POST['itemID']);
	$itm->set_itemID($itemID);
	$itm->undelete();

	if ($itm->was_undeleted())
	{
		search_form($its,'Activated item: <b>'.$itm->info['title'].'</b>');
	}
	else { echo "There was an error...tell Scott - this shouldn't happen here..."; }
}

function search_form(&$its,$status="")
{
	global $pg;

	?>
	<font size="3"><b>Add/Edit Item</b></font>
	<p />
	<?=(strlen($status)?$pg->status($status):'');?>

	<?php
	if (!strlen($its->findingmatch))
	{
		?>
		Please enter as much search criteria as you would like.
		<p />
		If there are no matching items, you will be redirected to the <a href="items.php?act=add"><b>Add Item</b></a> screen,<br />
		unless you enter only a UPC, after which you will be asked to search for the corresponding item.
		<?php

		$upc = YES;
	}
	else
	{
		?>
		Your UPC search did not match any items in the database. The provided UPC number may<br />
		correspond to a product that is already in the database, but does not have an assigned UPC.
		<p />
		Please search for the corresponding item below, <a href="items.php?act=add&addupc=<?=$its->criteria['upc'];?>">click here</a>
		to continue<br />to the item add screen, or <a href="items.php">click here</a> to search again.
		<?php

		$upc = NO;
	}
	?>
	<p />
	<?=$its->form($upc);?>
	<?php
}

$pg->foot();
?>