<?php
include('../../include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$type = (isset($_GET['type'])?$_GET['type']:@$_POST['type']);
$resultIDX = (isset($_GET['resultIDX'])?$_GET['resultIDX']:(isset($_POST['resultIDX'])?$_POST['resultIDX']:-1));
$newused = @$_GET['newused'];
$itemID = (isset($_GET['itemID'])?$_GET['itemID']:@$_POST['itemID']);
$returndata = (isset($_GET['returndata'])?$_GET['returndata']:@$_POST['returndata']);

$pg = new admin_page();
$error = new error('Add Item');
$its = new item_search($pg);
$its->action = $_SESSION['root_admin'].'pos/invoice_lookup.php';
$its->max_results = 250;

$pg->setFull(NO);
$pg->head();

echo "act($act) type($type) resultIDX($resultIDX) newused($newused) itemID($itemID)<p />";

if ($act == "")
{
	// do/show nothing
}
elseif ($act == 'search')
{
	// perform the item search
	$dosearch = YES;

	if (getP('from_quickadd') && @$_SESSION['do_quickadd'])
	{
		// get the criteria from the quick add
		$_POST['criteria'] = array(
			'itemIDs' => $_SESSION['quickadd_itemIDs']
		);

		// no need to show the status info...
		unset($_SESSION['quickadd_status']);
	}

	if (@$_POST['dolast'])
	{
		if (isset($_SESSION['invoice_last_search'][$type]))
		{
			$_POST['criteria'] = array(
				'upctitle'   => @$_SESSION['invoice_last_search'][$type]['upctitle'],
				'platformID' => @$_SESSION['invoice_last_search'][$type]['platformID']
			);
		}
		else { $dosearch = NO; }
	}

	if ($dosearch)
	{
		if (isset($_GET['getcriteria']))
		{
			$type = @$_GET['type'];
			$_POST['criteria'] = array(
				'upctitle'   => @$_GET['upctitle'],
				'platformID' => @$_GET['platformID']
			);
		}

		$_SESSION['invoice_last_search'][$type] = @$_POST['criteria'];
		$its->pull_post();
		$results = $its->search();

		$settooltip = array();
		if (strlen(@$_POST['criteria']['upctitle'])) { $settooltip[] = $_POST['criteria']['upctitle']; }
		if (strlen(@$_POST['criteria']['platformID']))
		{
			$pla = new platforms($pg, $_POST['criteria']['platformID']);
			$settooltip[] = $pla->platform_name();
		}
		$settooltip = implode(' / ', $settooltip);
		$_SESSION['invoice_last_search'][$type]['tooltip'] = $settooltip;

		?>
		<script type="text/javascript">
			parent.setLastTooltip(<?=$type;?>, '<?=$settooltip;?>')
		</script>
		<?php
	}
	else { $results = array(); }

	if (!count($results))
	{
		?>no_results<script type="text/javascript">parent.no_results(<?=$type;?>)</script><?php
	}
	elseif (count($results) > 1 || $type == RETURNS)
	{
		?>view_results<script type="text/javascript">parent.view_results(<?=$type;?>)</script><?php
	}
	else
	{
		if ($type == RETURNS)
		{

			// !!! THE POPUP WINDOW ALWAYS SHOWS UP NOW !!!

			// output the invoice checking script
			invoice_check(array(0), $type);
		}
		else
		{
			// output the quantity checking script
			//$itemID = $results[$resultIDX]['itm_itemID'];
			$resultIDX = 0; // select the first item
			$its->criteria = array();

			qty_check(array($resultIDX), $type);
		}
	}
}
elseif ($act == 'select')
{
	// find idx of itemIDs from multiple results form in session results
	$results = $_SESSION['search_results'];

	// perform some cleanup
	unset($_SESSION['do_quickadd']);
	unset($_SESSION['quickadd_itemIDs']);
	unset($_SESSION['quickadd_type']);

	if ($resultIDX == -1)
	{
		$itemID = explode('|', $itemID);
		$resultIDXs = array();

		while (list($a, $arr) = each($results))
		{
			if (in_array($arr['itm_itemID'], $itemID)) { $resultIDXs[] = $a; }
		}
	}
	else
	{
		$resultIDXs = array($resultIDX);
	}

	if ($type == RETURNS)
	{
		if (strlen($returndata))
		{
			// set the item as selected, rather than opening the returns screen
			$act = 'doadd';
			list($itemID, $_GET['purchdate'], $_GET['purchprice'], $_GET['add_newused']) = explode('|', $returndata);
			//$_GET['purchdate'] = date('m/d/Y', $purchtime);

			// get the resultIDX
			$resultIDX = -1;
			while (list($a, $arr) = each($_SESSION['search_results']))
			{
				if ($arr['itm_itemID'] = $itemID) { $resultIDX = $a; break; }
			}
			$_GET['addIDXs'] = $resultIDX;
		}
		else { invoice_check($resultIDXs, $type); }
	}
	else
	{
		qty_check($resultIDXs, $type);
	}
}

if ($act == 'doadd')
{
	$inv = new invoice($pg);
	$ccp = new ccpercs($pg);

	$options = (isset($_SESSION['quickadd_options']) ? $_SESSION['quickadd_options'] : array());
	unset($_SESSION['quickadd_options']);

	$addIDXs = explode('|',@$_GET['addIDXs']);
	$add_newused = explode('|',@$_GET['add_newused']);
	$toomanyIDXs = explode('|',@$_GET['toomanyIDXs']);
	$zeroqtyIDXs = explode('|',@$_GET['zeroqtyIDXs']);
	$purchdate = @$_GET['purchdate'];
	$purchprice = @$_GET['purchprice'];

	if (!strlen($addIDXs[0])) { $addIDXs = array(); }
	if (!strlen($add_newused[0])) { $add_newused = array(); }
	if (!strlen($toomanyIDXs[0])) { $toomanyIDXs = array(); }
	if (!strlen($zeroqtyIDXs[0])) { $zeroqtyIDXs = array(); }

	$results = $_SESSION['search_results'];
	$params = array();
	while (list($a, $idx) = each($addIDXs))
	{
		$itemID = (!isset($itemID) || $a ? $results[$idx]['itm_itemID'] : $itemID);
		$this_options = getA($options, $itemID, array());
		$newused = getA($this_options, 'newused', $add_newused[$a]);
		$params[] = array(
			'type'       => $type,
			'itemID'     => $itemID,
			'newused'    => $newused,
			'purchdate'  => $purchdate,
			'purchprice' => $purchprice,
			'options'    => $this_options
		);
	}

	// pull item info, add it to the database, and update the parent invoice table
	$idx = $inv->getNextIDX($type);
	/*
	if ($idx == 1)
	{
		// idx is zero-based!
		$idx = 0;
	}
	*/
	$type = -1;
	while (list($num, $itmarr) = each($params))
	{
		$type       = $itmarr['type'];
		$itemID     = $itmarr['itemID'];
		$newused    = $itmarr['newused'];
		$purchdate  = $itmarr['purchdate'];
		$purchprice = $itmarr['purchprice'];
		$options    = $itmarr['options'];

		if (!$purchdate) { $purchdate = 0; }
		if (!$purchprice) { $purchprice = 0; }

		// build info line
		$itminfo = array();
		$sql = "
			SELECT
				*
			FROM
				items,
				platforms,
				prices,
				quantity
			WHERE
				itm_itemID=$itemID
				AND itm_platformID=pla_platformID
				AND itm_itemID=prc_itemID
				AND prc_itemID=qty_itemID
				AND qty_storeID={$_SESSION['storeID']}
		";
		$result = mysql_query($sql, $db);
		$error->mysql(__FILE__,__LINE__);

		if (!mysql_num_rows($result))
		{
			echo "Error loading information for item $itemID.";
		}
		else
		{
			$row = mysql_fetch_assoc($result);

			// find the company names
			$row['com_name1'] = '-';
			$row['com_name2'] = '-';
			$compIDs = array($row['itm_company1ID'], $row['itm_company2ID']);
			while (list($a, $compID) = each($compIDs))
			{
				if ($compID)
				{
					$csql = "SELECT com_name FROM companies WHERE com_companyID=$compID";
					$cresult = mysql_query($csql, $db);
					$error->mysql(__FILE__,__LINE__);
					$crow = mysql_fetch_assoc($cresult);
					$row['com_name'.($a+1)] = $crow['com_name'];
				}
			}

			$price_new = $row['prc_new'];
			$price_used = $row['prc_used'];
			$pca = (@$ccp->percs[$row['itm_platformID']][0]?$ccp->percs[$row['itm_platformID']][0]:$ccp->percs[0][0]);
			$pcr = (@$ccp->percs[$row['itm_platformID']][0]?$ccp->percs[$row['itm_platformID']][0]:$ccp->percs[0][0]);
			$pca = sprintf('%0.3f',($pca/2));

			// if the current time is already in the invoice array, inc by 1
			$time = time();
			$in_invoice = YES;
			while ($in_invoice == YES)
			{
				$found = NO;
				while (list($a, $arr) = each($_SESSION['cust_items']))
				{
					if ($arr['ini_type'] == $type && $arr['ini_timeadded'] == $time) { $time++; $found = YES; }
				}
				reset($_SESSION['cust_items']);

				if (!$found) { $in_invoice = NO; }
			}

			$price = ($type==SALE ? ($newused==ITEM_NEW ? $price_new : $price_used) : ($type==TRADE ? ($price_used*($pcr/100)) : $price_used));
			$box = getA($options, 'box', ($type==SALE || $type==RETURNS ? BOX : $row['pla_defaultbox']));

			$itminfo = array(
				'ini_itemID'               => $row['itm_itemID'],
				'ini_invoiceID'            => $_SESSION['cust_invoiceID'],
				'ini_title'                => $row['itm_title'],
				'ini_newused'              => $newused,
				'ini_type'                 => $type,
				'ini_trade_type'           => getA($options, 'pricetype', ($type==TRADE || $type==RETURNS ? CREDIT : 0)),
				'ini_box'                  => $box,
				'ini_condition'            => getA($options, 'condition', GOOD),
				'ini_opened'               => OPENED,
				'ini_return_purchdate'     => $purchdate,
				'ini_return_purchprice'    => $purchprice,
				'ini_return_charged'       => NO,
				'ini_return_occasion'      => NONE,
				'ini_return_occasion_date' => '',
                                'ini_serial_number' => '',
				'ini_platformID'           => $row['itm_platformID'],
				'ini_platform_name'        => $row['pla_name'],
				'ini_platform_abbr'        => $row['pla_abbr'],
				'ini_company1ID'           => $row['itm_company1ID'],
				'ini_company1_name'        => $row['com_name1'],
				'ini_company2ID'           => $row['itm_company2ID'],
				'ini_company2_name'        => $row['com_name2'],
				'ini_price_manual'         => NO,
				'ini_price_new'            => sprintf('%0.2f', $price_new),
				'ini_price_used'           => sprintf('%0.2f', $price_used),
				'ini_price'                => sprintf('%0.2f', $price),
				'ini_qty'                  => 1,
				'ini_percentoff'           => 0,
				'ini_salemilestoneoff'     => '0%',
				'ini_trademilestoneup'     => '0%',
				'ini_idx'                  => $idx,
				'ini_timeadded'            => $time,
				'image'                    => ''
			);

			$_SESSION['cust_items'][] = $itminfo;
			if ($type == RETURNS) { $inv->update_prices(); } // this ensures that the necessary session vars are set for any return items

			// update the image paths and get the updated info
			$inv->getImages();
			$itminfo = $_SESSION['cust_items'][(count($_SESSION['cust_items']) - 1)];

			?>
			<script type="text/javascript">
				parent.clearSearch(<?=$type;?>);
				var firstIDX = parent.getFirstIDX(<?=$type;?>);

				<?php
				echo $inv->getAddItemJS($type, $idx, $itminfo,YES);
				echo "\n";
				echo $inv->getQtyJS(array($row['itm_itemID']),YES,YES);
				?>

				parent.drawData(<?=$type;?>,<?=$idx;?>, 'addbefore',firstIDX,<?=(($num+1)==count($params) ? 'false' : 'true');?>);
				<?php
				if (($num+1) == count($params))
				{
					?>parent.updateShownPrices(<?=$type;?>);<?php
				}
				?>
			</script>
			<?php

			$idx++;
		}
	}

	$results = $_SESSION['search_results'];
	//list($toomanyIDXs, $zeroqtyIDXs) = $_SESSION['cust_additem_errors'];

	// exceeded maximum quantity errors
	$titles = array();
	while (list($a, $idx) = each($toomanyIDXs))
	{
		$upc = $results[$idx]['itm_upc'];
		$platform = $results[$idx]['pla_name'];
		$title = $results[$idx]['itm_title'];

		$titles[] = $upc . (strlen($upc) ? ' - ' : '') . "$platform - $title";
	}
	if (count($titles))
	{
		$status = 'Exceeded maximum quantity limit for item' . (count($titles)!=1 ? 's' : '') . ":\n" . implode(' / ', $titles);
		$pg->addOnload("parent.add_error('" . mysql_escape_string($status) . "'," . TRADE . ")");
		if ($type == -1) { $type = TRADE; }
	}

	// no copies in inventory errors
	$titles = array();
	while (list($a, $idx) = each($zeroqtyIDXs))
	{
		$upc = $results[$idx]['itm_upc'];
		$platform = $results[$idx]['pla_name'];
		$title = $results[$idx]['itm_title'];

		$titles[] = $upc . (strlen($upc) ? ' - ' : '') . "$platform - $title";
	}
	if (count($titles))
	{
		$status = "There are no copies in your inventory of item" . (count($titles)!=1 ? 's' : '') . ":\n" . implode("\n", $titles) . (count($titles)>1 ? "\n\nNOTE: You will only be asked about forcing the first item into your inventory" : '');
		$pg->addOnload("parent.add_error('" . mysql_escape_string($status) . "'," . SALE . ")");

		// ask about the first item
		$pg->addOnload("parent.ask_force(" . $zeroqtyIDXs[0] . ",\"'" . $titles[0] . "'\"," . SALE . ",-1)");
		$dofocus = NO;

		if ($type == -1) { $type = SALE; }

		// perform some cleanup (just in case - this stops a forever loop of asking about quantities if one didn't have on-hand)
		unset($_SESSION['do_quickadd']);
		unset($_SESSION['quickadd_itemIDs']);
		unset($_SESSION['quickadd_type']);
	}

	if ($type == -1) { $type = TRADE; }
	$focus_field = "document.getElementById('lookup{$type}').elements['criteria[upctitle]']";

	// since the page doesn't refresh, this needs to happen here in order to keep the database current with what's on the screen
	$inv = new invoice($pg);
	$inv->update_prices();
	//$inv->sort_items();

	// show the timeadded reset
	$commands = $inv->getNewTimeAdded();
	?>
	<script type="text/javascript">
		<?php echo implode(';', $commands);?>;
	</script>
	<?php

	// perform some cleanup
	unset($_SESSION['do_quickadd']);
	unset($_SESSION['quickadd_itemIDs']);
	unset($_SESSION['quickadd_options']);

	//unset($_SESSION['cust_additem_errors']);
}
elseif ($act == 'dochange')
{
	// change the item's quantity and new/used setting
	$invoiceIDX = $_GET['invoiceIDX'];
	$nu = ($newused==ITEM_NEW ? 'new' : 'used');
	$image = "nu-$nu.gif";

	$tottrades = 0;
	while (list($a, $arr) = each($_SESSION['cust_items']))
	{
		if ($arr['ini_type'] == TRADE) { $tottrades++; }
	}
	$linkID = ($tottrades*3)+(($invoiceIDX*2)+1);

	?>
	dochange
	<script type="text/javascript">
		var itmidx = array_search(<?=$itemID;?>,parent.qty_itemIDs);
		parent.qty_<?=$nu;?>_orig[itmidx]++;
		parent.qty_<?=$nu;?>[itmidx]++;
		parent.update_orig_qty(<?=$type;?>,<?=$invoiceIDX;?>,itmidx);
		parent.change_newused(<?=$type;?>,<?=$invoiceIDX;?>, '<?=$image;?>',<?=$linkID;?>,<?=$newused;?>,<?=$itemID;?>);
	</script>
	<?php
}

function qty_check($resultIDXs, $type)
{
	// !!! IF TRADING ITEMS AND BUYING SAME ITEMS (HAPPENS?) NEED TO CHECK !!!
	// !!! QTY IF NEW/USED CHANGE ON SALE                                  !!!
	//
	// (IE: WITH 5 MAX USED OH: 3 USED OH+4 USED TRADE-2 USED SALE=5 USED OH - if change sale item to NEW, what happens?)
	// check before changing values; if too many, alert and don't allow change (?)

	global $pg;

	$itemIDs = array(); // format: $itemIDs[idx] = #
	$max_copies = array(); // format: $max_copies[idx] = #
	$new_qtys = array(); // format: $new_qtys[idx] = #
	$used_qtys = array(); // format: $used_qtys[idx] = #

	$mc = new max_copies($pg);
	$itm = new items($pg);

	$results = $_SESSION['search_results'];

	while (list($a, $idx) = each($resultIDXs))
	{
		$itemID = $results[$idx]['itm_itemID'];

		$itm->set_itemID($itemID);
		$platformID = $itm->info['platformID'];

		list($nc, $uc) = @$mc->copies[$platformID];
		if (!strlen($uc)) { $uc = -1; }

		$itemIDs[] = $itemID;
		$max_copies[] = $uc;
		$new_qtys[] = $results[$idx]['qty_new'];
		$used_qtys[] = $results[$idx]['qty_used'];
	}

	?>
	qty_check
	<script type="text/javascript">
		var resultIDXs = new Array('<?=implode("', '", $resultIDXs);?>');
		var itemIDs = new Array('<?=implode("', '", $itemIDs);?>');
		var max_copies = new Array('<?=implode("', '", $max_copies);?>');
		var new_qtys = new Array('<?=implode("', '", $new_qtys);?>');
		var used_qtys = new Array('<?=implode("', '", $used_qtys);?>');

		var addIDXs = new Array();
		var add_newused = new Array();
		var toomanyIDXs = new Array();
		var zeroqtyIDXs = new Array();

		for (var i=0; i<resultIDXs.length; i++)
		{
			resultIDX = resultIDXs[i];
			itemID = itemIDs[i];
			max_copy = max_copies[i];
			new_qty = new_qtys[i];
			used_qty = used_qtys[i];

			if (resultIDX.length)
			{
				if (<?=$type;?> == <?=TRADE;?>)
				{
					var qty = parent.get_quantity(itemID,<?=ITEM_USED;?>);
					if (qty == -1) { qty = used_qty; }
					var nu = <?=ITEM_USED;?>
				}
				else
				{
					var nqty = parent.get_quantity(itemID,<?=ITEM_NEW;?>);
					if (nqty == -1) { nqty = new_qty; }
					var uqty = parent.get_quantity(itemID,<?=ITEM_USED;?>);
					if (uqty == -1) { uqty = used_qty; }

					var qty = (nqty>0?nqty:uqty);
					var nu = (nqty>0?<?=ITEM_NEW;?>:<?=ITEM_USED;?>);
				}

				var erred = false;
				if (<?=$type;?> == <?=TRADE;?>)
				{
					if (max_copy != -1)
					{
						if ((qty+1) > max_copy) { toomanyIDXs[toomanyIDXs.length] = resultIDX; erred = true; }
					}
				}
				else if (<?=$type;?> == <?=SALE;?>)
				{
					if (qty <= 0) { zeroqtyIDXs[zeroqtyIDXs.length] = resultIDX; erred = true; }
				}

				if (!erred)
				{
					addIDXs[addIDXs.length] = resultIDX;
					add_newused[add_newused.length] = nu;
				}
			}
		}

		addIDXs = implode('|',addIDXs);
		add_newused = implode('|',add_newused);
		toomanyIDXs = implode('|',toomanyIDXs);
		zeroqtyIDXs = implode('|',zeroqtyIDXs);

		document.location = '/admin/pos/invoice_lookup.php?act=doadd&addIDXs='+addIDXs+'&add_newused='+add_newused+'&toomanyIDXs='+toomanyIDXs+'&zeroqtyIDXs='+zeroqtyIDXs+'&type=<?=$type;?>&newused='+nu;
	</script>
	<?php
}

function invoice_check($resultIDXs, $type)
{
	global $pg;

	?>
	invoice_check
	<script type="text/javascript">
		open_window('/admin/pos/invoice_return_lookup.php?type=<?=$type;?>&resultIDX=<?=$resultIDXs[0];?>', 'retlookup',725,500, 'YES',true);
	</script>
	<?php
}

/*
if (count($_GET)) { ?><pre><b>$_GET</b><br /><?=print_r($_GET);?></pre><?php }
if (count($_POST)) { ?><pre><b>$_POST</b><br /><?=print_r($_POST);?></pre><?php }
*/

$pg->foot();
?>
