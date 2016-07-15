<?php
include('../../include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

$pg = new admin_page();
$inv = new invoice($pg);
$inv->set_invoiceID(-1,NO);
$error = new error('Invoice Update');

$pg->setFull(NO);
$pg->setCenter(NO);
$pg->head();

$status = 'Awaiting user input';
$parent_refresh = NO;

if ($act == "remove")
{
	echo "remove from invoice<p />";

	// remove some items from the invoice
	$remove_sale = explode(',',@$_POST['remove_sale']);
	$remove_trade = explode(',',@$_POST['remove_trade']);
	$remove_return = explode(',',@$_POST['remove_return']);
	if ($remove_sale[0] == '') { $remove_sale = array(); }
	if ($remove_trade[0] == '') { $remove_trade = array(); }
	if ($remove_return[0] == '') { $remove_return = array(); }

	$sale_idxs = -1;
	$trade_idxs = -1;
	$return_idxs = -1;
	$newitems = array();
	$invoice_qtys = array(
		SALE    => array(),
		TRADE   => array(),
		RETURNS => array()
	);

	while (list($a,$arr) = each($_SESSION['cust_items']))
	{
		if ($arr['ini_type'] == SALE)
		{
			if (!in_array($arr['ini_idx'],$remove_sale)) { $newitems[] = $arr; }
			else { $invoice_qtys[$arr['ini_type']][$arr['ini_idx']] = $arr['ini_qty']; }
		}
		elseif ($arr['ini_type'] == TRADE)
		{
			if (!in_array($arr['ini_idx'],$remove_trade)) { $newitems[] = $arr; }
			else { $invoice_qtys[$arr['ini_type']][$arr['ini_idx']] = $arr['ini_qty']; }
		}
		elseif ($arr['ini_type'] == RETURNS)
		{
			if (!in_array($arr['ini_idx'],$remove_return)) { $newitems[] = $arr; }
			else { $invoice_qtys[$arr['ini_type']][$arr['ini_idx']] = $arr['ini_qty']; }
		}
	}
	$_SESSION['cust_items'] = $newitems;
	reset($_SESSION['cust_items']);

	$removes = array(
		SALE    => $remove_sale,
		TRADE   => $remove_trade,
		RETURNS => $remove_return
	);
	$commands = array();
	$where_or = array();

	$removed_types = array();

	while (list($type,$arr) = each($removes))
	{
		while (list($a,$idx) = each($arr))
		{
			if (isset($invoice_qtys[$type][$idx]))
			{
				$where_or[] = "(ini_type=$type AND ini_idx=$idx)";
				$commands[] = "parent.drawData($type,$idx,'remove',-1,true)";
				$commands[] = "itemIDX = array_search(parent.".invType($type)."_itemIDs[$idx],parent.qty_itemIDs)";
				$commands[] = "parent.".invType($type)."_invoice_qtys[itemIDX]-={$invoice_qtys[$type][$idx]}";
				//$commands[] = "parent.".invType($type)."_invoice_qtys[$idx]-={$invoice_qtys[$type][$idx]}";
			}
		}
		if (count($arr))
		{
			// commands to redraw the table
			$commands[] = "parent.rebuildIDXs($type)";
			$commands[] = "parent.drawAllData($type)";
		}
	}

	if (count($where_or))
	{
		echo "deleting ".count($where_or)." item(s) from invoice<p />";
		$sql = "DELETE FROM invoice_items WHERE ini_invoiceID={$_SESSION['cust_invoiceID']} AND (".implode(' OR ',$where_or).")";
		mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);

		$commands[] = "parent.updateShownPrices()";
		$commands[] = "parent.clearRemove()";
		$commands[] = "parent.clearSearch(".SALE.")";

		$commands = array_merge($commands,$inv->getNewTimeAdded());
		?>
		<hr />
		<?php echo implode(';<br />',$commands);?>;
		<hr />
		<script type="text/javascript"><?="\n".implode(";\n",$commands).";\n";?></script>
		<?php
	}
	else { echo "did not remove any items from invoice<br />maybe they were already removed?"; }

	// since the page doesn't refresh, this needs to happen here in order to keep the database current with what's on the screen
	$inv->update_prices();
	//$inv->sort_items();
}
elseif ($act == "change")
{
	$type = $_POST['type'];
	$idx = $_POST['idx'];
	$timeadded = explode('|',$_POST['timeadded']);
	$field = explode('|',$_POST['field']);
	$to = explode('||',$_POST['to']);
	$employeeID = $_POST['employeeID'];
	$closecustomer = $_POST['closecustomer'];

	if (strlen($employeeID))
	{
		// change the employeeID
		$_SESSION['cust_invoice_info']['inv_employeeID'] = $employeeID;
		$_SESSION['last_employeeID'] = $employeeID;

		$sql = "UPDATE invoices SET inv_employeeID=$employeeID WHERE inv_invoiceID=".$_SESSION['cust_invoiceID'];
		mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);

		$status = 'Database updated!';
	}
	elseif (strlen($closecustomer))
	{
		// change the 'close customer after complete' value
		$_SESSION['close_after_complete'] = ($closecustomer=='true' ? YES : NO);

		$status = 'Database updated!';
	}
	else
	{
		// change an item line
		$change_items = array();

		//echo 'cust_items is ' . count($_SESSION['cust_items']) . ' in length<br />';

		$sort = new sort();
		$_SESSION['cust_items'] = $sort->doSort($_SESSION['cust_items'],'ini_idx');

		$oftype = -1;
		while (list($a,$arr) = each($_SESSION['cust_items']))
		{
			//echo "(idx = $idx) type: " . $arr['ini_type'] . " == $type<br />";
			if ($arr['ini_type'] == $type)
			{
				$oftype++;
				if ($idx == -1)
				{
					//echo "(oftype = $oftype) timeadded: " . $arr['ini_timeadded'] . ' == ' . $timeadded[$oftype] . '<br />';
				}
				if (($idx > -1 && $arr['ini_timeadded'] == $timeadded[0]) || ($idx == -1 && $arr['ini_timeadded'] == $timeadded[$oftype]))
				{
					$change_items[] = $a;
				}
			}
		}
		reset($_SESSION['cust_items']);

		/*
		?><div align="left">$change_items<pre><?php echo print_r($change_items,true);?></pre></div><?php
		*/

		if (count($change_items) || !count($_SESSION['cust_items']))
		{
			while (list($a,$item_idx) = each($change_items))
			{
				while (list($b,$k) = each($field))
				{
					$toexp = explode('|',$to[$a]);
					if ($toexp[$b] == '{BLANK}') { $toexp[$b] = ''; }
					echo "idx $item_idx field $k old[" . $_SESSION['cust_items'][$item_idx][$k] . '] new[' . $toexp[$b] . ']<br />';
					$_SESSION['cust_items'][$item_idx][$k] = $toexp[$b];
				}
				reset($field);
			}

			$inv->update_db_items();
			$commands = $inv->getNewTimeAdded();
			?>
			<script type="text/javascript">
				<?php echo implode(';',$commands);?>;
			</script>
			<?php

			$status = 'Database updated!';
		}
		else
		{
			// unable to find item(s) in cust_items array
			$status = "ERROR - TELL SCOTT!!!";
			echo 'ERROR ERROR ERROR';
			//$parent_refresh = YES;
		}
	}
}

//for ($i=0; $i<10000000; $i++) { } // 'fake' a delay

?><div align="left"><b>$_POST</b><pre><?=print_r($_POST);?></pre></div><?php
?>
<script type="text/javascript">
	function setParentStatus(num)
	{
		if (!parent.set_status)
		{
			if (num == 4)
			{
				// reload the parent page
				parent.location = parent.location;
			}
			else
			{
				num++;
				setTimeout('setParentStatus(' + num + ')',250);
			}
		}
		else
		{
			parent.set_status('<?=$status;?>',false)
			if (<?php echo ($parent_refresh ? 'true' : 'false');?>)
			{
				// refresh the parent page - happens when an error occurs
				alert('There was an error with your previous action.\nAs a result, the invoice will be refreshed.\nPlease try your action again when the invoice has reloaded.\n\nIf this error continues to happen, please contact Scott Carpenter');
				parent.location = parent.location;
			}
		}
	}
</script>
<?php

$pg->addOnload('setParentStatus(0)');

$pg->foot();
?>