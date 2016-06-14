<?php
/*
* Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include('../../include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$return = @$_POST['return'];
if (!strlen($return)) { $return = '/admin/setup_items/quickadd.php'; }
$items = @$_POST['items'];

$pg = new admin_page();
$itm = new items($pg);
$error = new error('Quick Item Add - Update');

if ($act == 'add')
{
	// add the items
	$platformID = $_POST['platformID'];
	$options    = getP('options', array());
	$toinvoice  = getP('toinvoice', array());

	$errors = array();
	$added = array(); // titles of added items
	$invoice_itemIDs = array();
	$invoice_options = array();
	$defaults = array(
		'imgID'      => 0,
		'company1ID' => 0,
		'company2ID' => 0,
		'percopy'    => YES,
		'active'     => YES
	);

	foreach ($items as $a => $arr)
	{
		foreach ($arr as $k => $v)
		{
			if (!is_array($v)) { $arr[$k] = trim(stripslashes($v)); }
		}

		if (strlen(trim($arr['title'])))
		{
			$itm->info = array('platformID'=>$platformID);
			foreach ($defaults as $k => $v)
			{
				$itm->info[$k] = $v;
			}
			foreach ($arr as $k => $v)
			{
				$itm->info[$k] = $v;
			}

			$itemID = $itm->add();

			if ($itm->status[0] == ADDED)
			{
				$added[] = $itm->info['title'];
			}
			elseif ($itm->status[0] == DUPLICATE)
			{
				$this_error = "The UPC <b>{$itm->info['upc']}</b> ({$itm->info['title']}) is already in use";

				if (@$toinvoice[$a])
				{
					// find the itemID so that we can add it to the invoice
					$sql = "SELECT itm_itemID FROM items WHERE itm_upc='" . mysql_escape_string($itm->info['upc']) . "'";
					$result = mysql_query($sql, $db);
					$error->mysql(__FILE__,__LINE__);

					if (mysql_num_rows($result) == 1)
					{
						$row = mysql_fetch_assoc($result);
						$invoice_itemIDs[] = $row['itm_itemID'];
						$invoice_options[$row['itm_itemID']] = $options[$a];
						$this_error .= '<br />The existing item will be added to the invoice.';
					}
					else
					{
						$this_error .= ' by more than one item.<br />Please manually add it to the invoice.';
					}
				}

				$errors[] = $this_error;
			}

			// check if we need to add this item to the current invoice
			if (@$toinvoice[$a])
			{
				$invoice_itemIDs[] = $itemID;
				$invoice_options[$itemID] = $options[$a];
			}
		} // if title has length
	} // each item

	$_SESSION['quickadd_status'] = "<b>".count($added)." Item".(count($added)!=1?'s':'')." Added</b><p />".implode('<br />',$added);
	$_SESSION['quickadd_errors'] = $errors;

	$return .= "?numitems={$_POST['numitems']}&platformID=$platformID";

	// if applicable, make it so that the items are added to the current invoice
	if ($toinvoice && count($invoice_itemIDs))
	{
		$_SESSION['quickadd_itemIDs'] = $invoice_itemIDs;
		$_SESSION['quickadd_options'] = $invoice_options;
		$_SESSION['do_quickadd'] = YES;
		$_SESSION['quickadd_type'] = getP('quickadd_type',TRADE);
		if (!count($errors))
		{
			$return = '/admin/pos/invoice.php?act=view';
		}
		else
		{
			$return .= '&errors=' . YES;
		}
	} // if there are items to add to the current invoice
} // if act is 'add'

if (strlen($return))
{
	$pg->showUpdating('Performing Quick Add...',$return);
}
?>