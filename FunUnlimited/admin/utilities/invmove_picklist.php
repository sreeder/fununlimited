<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

if (!@$_SESSION['requestID']) { header('Location: /admin/utilities/invmove_pickstores.php'); }
$requestID = $_SESSION['requestID'];
$tmp_items = @$_SESSION['request_info']['items'];

$pg = new admin_page();
$pg->setTitle('Store to Store Inventory Movement - Select Items');
$pg->head('Store to Store Inventory Movement - Select Items');

$error = new error('Store to Store Inventory Movement - Select Items');
$mov = new inventory_movement($pg);
$mov->setRequestID($requestID);
$info = $mov->getRequestInfo();

/*
levels and which store needs to do something
0 - from store
1 - to store
*/
$levels = array(
	MOV_INCOMPLETE=>0,
	MOV_REQUEST_SENT=>1,
	MOV_ITEMS_SELECTED=>0,
	MOV_IN_TRANSIT=>1,
	MOV_ITEMS_RECEIVED=>0,
	MOV_DENIED=>0,
	MOV_NOT_ALL_RECEIVED=>0,
	MOV_COMPLETE=>0
);

// format: $setup[status] = array('instructions','submit button');
$setup = array(
	MOV_INCOMPLETE=>array('Select the items you would like to include in the list sent to the above store.','Send List'),
	MOV_REQUEST_SENT=>array('Select which items you would like to receive from the above store.','Save Selected Items'),
	MOV_ITEMS_SELECTED=>array('Select which items you have shipped/are shipping to the above store.','Save Shipped Items'),
	MOV_IN_TRANSIT=>array('Select which items you have received from the above store.<p /><b>Note: Only change the discount values if the item you received is different than shown!</b>','Save Received Items')
);

if (!count($info) || !is_array($info))
{
	?>
	The current requestID does not exist.
	<p />
	<input type="button" value="&lt; Start Over" onclick="document.location='/admin/utilities/invmove.php?reset=<?=YES;?>'" class="btn" />
	<?php
}
else
{
	$mov->showStores();
	$status = $info['req_status'];
	$discounts = $info['discounts'];
	$level = $levels[$status];
	list($instructions,$button) = $setup[$status];
	if ($status == MOV_INCOMPLETE) { $_SESSION['request_info']['items'] = $tmp_items; }

	if ($info['stores']['from'] == $_SESSION['storeID']) { $from = YES; $to = NO; } else { $from = NO; $to = YES; }

	// format: $fields = array('itemID field','platformID field','platform name field')
	$fields = array('','','');

	$items = $mov->getRequestItems();
	if ($status == MOV_INCOMPLETE)
	{
		$print_requestID = 'incomplete';
		$fields = array('itm_itemID','pla_platformID','pla_name');
	}
	else
	{
		$print_requestID = $requestID;
		$fields = array('rqi_itemID','rqi_platformID','rqi_platformname');
	}

	?>
	<?=$instructions;?>
	<p />
	<input type="button" value="Print Item List &gt;" onclick="open_window('/admin/utilities/invmove_print.php?act=printitems&requestID=<?=$print_requestID;?>','printitems',725,500,'YES',true)" class="btn" /><br />
	<?php

	// break items out into platforms
	$show = array(); // format: $show[platformID] = array(platform_name,items)
	$itemIDs = array();
	$itemIDnus = array();
	$item_platformIDs = array();
	$platform_names = array();
	while (list($a,$arr) = each($items))
	{
		if (!isset($show[$arr[$fields[1]]]))
		{
			$show[$arr[$fields[1]]] = array($arr[$fields[2]],array());
		}
		$platform_names[$arr[$fields[1]]] = $arr[$fields[2]];
		$show[$arr[$fields[1]]][1][] = $arr; // add to the items element
		$itemIDs[] = $arr[$fields[0]];
		$item_platformIDs[] = $arr[$fields[1]];
	}
	$platformIDs = array_keys($show);
	reset($items);

	if ($status == MOV_INCOMPLETE)
	{
		// reorder by platform name and then item title
		$sort = new sort();
		$new_show = array();
		asort($platform_names);
		while (list($platformID,$platform_name) = each($platform_names))
		{
			$show[$platformID][1] = $sort->doSort($show[$platformID][1],'itm_title',SORT_ASC,SORT_STRING);
			$new_show[$platformID] = $show[$platformID];
		}
		$show = $new_show;
	}

	?>
	<script type="text/javascript">
		// platform/item/discount data at bottom of page

		// [un]apply discounts
		function c(itemID,nu) { d('c',itemID,nu); }
		function b(itemID,nu) { d('b',itemID,nu); }
		function i(itemID,nu) { d('i',itemID,nu); }
		function d(type,itemID,nu)
		{
			if (type != 'v') { var obj = document.getElementById('selitemfrm').elements[type+'['+itemID+nu+']']; }

			var val = -1;
			if (type == 'v') { type == 'visibility'; }
			else if (type == 'c') { type = 'condition'; val = obj.value; }
			else if (type == 'b') { type = 'box'; val = (obj.checked?1:0); }
			else if (type == 'i') { type = 'instructions'; val = (obj.checked?1:0); }

			// get/set the price
			var base = getBasePrice(itemID,nu);
			if (type != 'visibility') { setItemDiscount(itemID,nu,type,val) }
			var newprice = getPrice(base,itemID,nu);
			setPrice(itemID,nu,newprice);
		}

		// set the item's price and update the shown prices/totals
		function setPrice(itemID,nu,newprice)
		{
			var idx = getItemIDX(itemID,nu);
			var platformID = item_platformIDs[idx];
			var platidx = array_search(platformID,platformIDs);
			current_prices[idx] = newprice;

			var obj = document.getElementById('ip'+itemID+nu);
			obj.innerText = '$'+format_price(newprice);

			updatePlatformTotal(platformID);
			updateAllTotal();
		}

		// get the base price of an item (unapply any discounts)
		function getBasePrice(itemID,nu)
		{
			var idx = getItemIDX(itemID,nu);
			var platformID = item_platformIDs[idx];
			var platidx = array_search(platformID,platformIDs);
			var d = ''+base_discounts[idx];
			var box = d.substring(0,1)*1;
			var instructions = d.substring(1,2)*1;
			var condition = d.substring(2,3)*1;

			var base = base_prices[idx];
			if (!box) { base += nobox[platidx]; }
			if (!instructions) { base += noinstructions[platidx]; }
			if (condition == <?=FAIR;?>) { base += condfair[platidx]; }
			if (condition == <?=POOR;?>) { base += condpoor[platidx]; }

			return base;
		}

		// get the price of an item (apply any discounts)
		function getPrice(base,itemID,nu)
		{
			var idx = getItemIDX(itemID,nu);
			var platformID = item_platformIDs[idx];
			var platidx = array_search(platformID,platformIDs);
			var d = ''+current_discounts[idx];
			var box = d.substring(0,1)*1;
			var instructions = d.substring(1,2)*1;
			var condition = d.substring(2,3)*1;

			var price = base;
			if (!box) { price = orHalf(price,nobox[platidx]); }
			if (!instructions) { price = orHalf(price,noinstructions[platidx]); }
			if (condition == <?=FAIR;?>) { price = orHalf(price,condfair[platidx]); }
			if (condition == <?=POOR;?>) { price = orHalf(price,condpoor[platidx],true); }

			return format_price(price);
		}

		// subtract the given amount from the given price, or cut it in half if subtracting the price goes below half the price
		function orHalf(price,sub,halftwice)
		{
			var half = format_price(price/2);
			if ((price-sub) < half)
			{
				price = format_price(price/2);
				if (halftwice) { price = format_price(price/2); }
			}
			else { price = (price-sub); }

			return price;
		}

		// change the given discount for the given item
		function setItemDiscount(itemID,nu,discount,val)
		{
			var idx = getItemIDX(itemID,nu);
			var d = ''+current_discounts[idx];
			var box = d.substring(0,1);
			var instructions = d.substring(1,2);
			var condition = d.substring(2,3);

			if (discount == 'box') { box = val; }
			else if (discount == 'instructions') { instructions = val; }
			else if (discount == 'condition') { condition = val; }

			var newd = oz(box)+''+oz(instructions)+''+condition;
			current_discounts[idx] = newd;
		}

		// update a platform's total
		function updatePlatformTotal(platformID)
		{
			var total = 0;
			for (var i=0; i<itemIDs.length; i++)
			{
				var item_platformID = item_platformIDs[i];
				if (total && item_platformID != platformID) { break; }
				else if (item_platformID == platformID) { total += (current_prices[i]*1); } // I hate you javascript - half the numbers get added as numbers, and then a few get concatenated as strings
			}

			var platidx = array_search(platformID,platformIDs);
			platform_totals[platidx] = total;

			var count = platform_count[platidx];

			var obj = document.getElementById('pt'+platformID);
			obj.innerText = count+' item'+(count!=1?'s':'')+' / $'+format_price(total);
		}

		// update the entire request's total
		function updateAllTotal()
		{
			var total_price = 0;
			var total_count = 0;

			for (var i=0; i<platformIDs.length; i++)
			{
				total_price += platform_totals[i];
				total_count += platform_count[i];
			}

			var obj = document.getElementById('at');
			obj.innerText = total_count+' item'+(total_count!=1?'s':'')+' / $'+format_price(total_price);
		}

		// get the itemIDs index; this accounts for new/used
		function getItemIDX(itemID,nu)
		{
			var idx = array_search(itemID,itemIDs);
			if (newused[idx] != nu) { idx++; }

			return idx;
		}

		// return 1 for true, 0 for false
		function oz(val) { return (val*1?1:0); }

		// show/hide the options and set the price to 0 if hiding
		function sh(itemID,nu)
		{
			var obj = document.getElementById('selitemfrm').elements['sel['+itemID+nu+']'];
			var sel = obj.checked;

			var optobj = document.getElementById('o'+itemID+nu);

			var idx = getItemIDX(itemID,nu);
			var platformID = item_platformIDs[idx];
			var platidx = array_search(platformID,platformIDs);

			if (!sel)
			{
				platform_count[platidx]--;
				setPrice(itemID,nu,0);
				optobj.style.visibility = 'hidden';
			}
			else
			{
				platform_count[platidx]++;
				optobj.style.visibility = 'visible';
				d('v',itemID,nu);
			}
		}
	</script>
	<style type="text/css">
		.ip { text-align:right; }
		.l { background-color:<?=$pg->color('table-cell2');?>; }
		.d { background-color:<?=$pg->color('table-cell2');?>; }
	</style>

	<form method="post" action="/admin/utilities/invmoveUpdate.php" id="selitemfrm" onsubmit="this.prices.value=current_prices">
	<input type="hidden" name="act" value="setitems" />
	<input type="hidden" name="requestID" value="<?=$requestID;?>" />
	<input type="hidden" name="status" value="<?=$status;?>" />
	<input type="hidden" name="prices" value="" />
	<?php
	$fromqty = $_SESSION['request_info']['qtys']['from'];
	$toqty = $_SESSION['request_info']['qtys']['to'];
	$base_discounts = array();
	$base_prices = array();
	$all_total = 0;
	$all_count = 0;
	$all_newused = array();
	$platform_totals = array();
	$_SESSION['request_force_used'] = array();
	while (list($platformID,list($platform_name,$items)) = each($show))
	{
		$width_shown = NO;
		?>
		<table border="0" cellspacing="0" cellpadding="5" width="700">
		<tr><td><font size="2"><b><i>� <?=$platform_name;?></i></b></font></td></tr>
		<tr>
			<td>
				<?php
				$pg->outlineTableHead('100%');
				//while (list($a,$arr) = each($items))
				for ($i=0; $i<count($items); $i++)
				{
					$arr = $items[$i];
					$class = (($i%2)?'d':'l');

					if ($status == MOV_INCOMPLETE)
					{
						$itemID = $arr['itm_itemID'];
						$title = $arr['itm_title'];
						$boxc = ' checked="checked"';
						$instc = ' checked="checked"';
						$cond = GOOD;
						$base_discounts[] = YES.YES.GOOD;
						$show_options = YES;

						$fromnewqty = $arr['fromqty_new'];
						$fromusedqty = $arr['fromqty_used'];
						$tonewqty = $arr['toqty_new'];
						$tousedqty = $arr['toqty_used'];

						// if need to do new and used, array_splice another row into $items and create $_SESSION['request_force_used'] array containing itemIDs
						if (!in_array($itemID,$_SESSION['request_force_used']) && $fromnewqty >= $fromqty && $tonewqty <= $toqty)
						{
							$newused = 'New';
							$nuval = ITEM_NEW;
							$price = $arr['prc_new'];

							// check if they need to do a used one as well
							if (@$_SESSION['request_duped'] != $requestID && $fromusedqty >= $fromqty && $tousedqty <= $toqty)
							{
								// stick it in the next slot in $items
								$_SESSION['request_force_used'][] = $itemID;
								$putin = array($arr);
								array_splice($_SESSION['request_info']['items'],($i+1),0,$putin);
								array_splice($items,($i+1),0,$putin);
								array_splice($itemIDs,($i+1),0,$itemID);
								array_splice($item_platformIDs,($i+1),0,$arr['pla_platformID']);
							}
						}
						else
						{
							$newused = 'Used';
							$nuval = ITEM_USED;
							$price = $arr['prc_used'];
						}

						// apply the percent discount
						$percent = $discounts[($nuval==ITEM_NEW?'new':'used').'percent'][$arr['pla_platformID']];
						$timesby = ($percent/100);
						$price = sprintf('%0.2f',($price*$timesby));

						// $all_count is current overall index
						$_SESSION['request_info']['items'][$all_count]['newused'] = $newused;
						$_SESSION['request_info']['items'][$all_count]['price'] = $price;
					}
					else
					{
						if (in_array($status,array(MOV_ITEMS_SELECTED,MOV_IN_TRANSIT))) { $show_options = YES; } // they're selecting the shipped items
						else { $show_options = NO; }

						$itemID = $arr['rqi_itemID'];
						$title = $arr['rqi_title'];
						$newused = ($arr['rqi_newused']==ITEM_NEW?'New':'Used');
						$nuval = $arr['rqi_newused'];
						$boxc = ($arr['rqi_box']?' checked="checked"':'');
						$instc = ($arr['rqi_instructions']?' checked="checked"':'');
						$cond = $arr['rqi_condition'];
						$base_discounts[] = "{$arr['rqi_box']}{$arr['rqi_instructions']}{$arr['rqi_condition']}";
						$price = $arr['rqi_price'];
					}
					$base_prices[] = $price;

					if (!isset($platform_totals[$platformID])) { $platform_totals[$platformID] = 0; }
					$platform_totals[$platformID] += $price;
					if (!isset($platform_count[$platformID])) { $platform_count[$platformID] = 0; }
					$platform_count[$platformID]++;
					$all_total += $price;
					$all_count++;
					$all_newused[] = $nuval;
					$itemIDnus[] = $itemID.$nuval;

					if (!$width_shown) { $widtht = ' width="100%"'; } else { $widtht = ''; }
					$width_shown = YES;

// tabs are removed to cut down output file size (example: ~112 tabs per item * 3000 items = 336,000 bytes...ouch)
?>
<tr class="<?=$class;?>">
<td><input type="checkbox" name="sel[<?=$itemID.$nuval;?>]" onclick="sh(<?=$itemID;?>,<?=$nuval;?>)" class="nb" checked="checked" /></td>
<td<?=$widtht;?>><?=stripslashes($title);?></td>
<td><input type="hidden" name="nu[<?=$itemID.$nuval;?>]" value="<?=$nuval;?>" /><?=$newused;?></td>
<td>
<span id="o<?=$itemID.$nuval;?>">
<?php
if ($show_options)
{
?>
<input type="checkbox" name="b[<?=$itemID.$nuval;?>]" onclick="b(<?=$itemID;?>,<?=$nuval;?>)" class="nb"<?=$boxc;?> /> Box
<input type="checkbox" name="i[<?=$itemID.$nuval;?>]" onclick="i(<?=$itemID;?>,<?=$nuval;?>)" class="nb"<?=$instc;?> /> Instructions
<select name="c[<?=$itemID.$nuval;?>]" onchange="c(<?=$itemID;?>,<?=$nuval;?>)"><?php
	$conditions = array(GOOD=>'Good',FAIR=>'Fair',POOR=>'Poor');
	while (list($val,$name) = each($conditions))
	{
		if ($val == $cond) { $s = ' selected="selected"'; } else { $s = ''; }
		?><option value="<?=$val;?>"<?=$s;?>><?=$name;?></option><?php
	}
?></select> Condition
<?php
}
else
{
// just show the values of the options with hidden fields
?>
<input type="hidden" name="b[<?=$itemID.$nuval;?>]" value="<?=$arr['rqi_box'];?>" />
<input type="hidden" name="i[<?=$itemID.$nuval;?>]" value="<?=$arr['rqi_instructions'];?>" />
<input type="hidden" name="c[<?=$itemID.$nuval;?>]" value="<?=$arr['rqi_condition'];?>" />
<?=(!$arr['rqi_box']?'No ':'');?>Box /
<?=(!$arr['rqi_instructions']?'No ':'');?>Instructions /
<?=($arr['rqi_condition']==GOOD?'Good':($arr['rqi_condition']==FAIR?'Fair':'Poor'));?> Condition
<?php
}
?>
</span>
</td>
<td id="ip<?=$itemID.$nuval;?>" class="ip">
$<?=sprintf('%0.2f',$price);?>
</td>
</tr>
<?php
				}
				reset($items);

				$pg->outlineTableFoot();
				?>
			</td>
		</tr>
		<tr>
			<td colspan="2" align="right">
				<font size="1"><b><?=$platform_name;?> Total:</b> <span id="pt<?=$platformID;?>"><?=count($items);?> items / $<?=sprintf('%0.2f',$platform_totals[$platformID]);?></span></font>
			</td>
		</tr>
		</table>
		<p />
		<?php
	}
	?>
	<p />
	<font size="3"><b>Request Total:</b> <span id="at"><?=$all_count;?> item<?=($all_count!=1?'s':'');?> / $<?=sprintf('%0.2f',$all_total);?></span></font>
	<p />
	<input type="button" value="&lt; Reset Form" onclick="if (confirm('Are you SURE you want to reset the form?')) { this.form.reset(); }" class="btn" />
	<input type="submit" value="<?=$button;?> &gt;" class="btn" />
	</form>

	<p />
	<?php
	if ($status == MOV_INCOMPLETE) { ?><input type="button" value="&lt; Cancel" onclick="if (confirm('Are you sure you want to cancel this request?')) { document.location='/admin/utilities/invmoveUpdate.php?act=delete&requestID=<?=$requestID;?>'; }" class="btn" /><?php }
	else { ?><input type="button" value="&lt; Return to Request List" onclick="document.location='/admin/utilities/invmove.php?reset=<?=YES;?>'" class="btn" /><?php }
	?>

	<script type="text/javascript">
		var itemIDs = [<?=implode(',',$itemIDs);?>];
		var item_platformIDs = [<?=implode(',',$item_platformIDs);?>];
		var platformIDs = [<?=implode(',',$platformIDs);?>];
		var nobox = [<?=implode(',',$discounts['nobox']);?>];
		var noinstructions = [<?=implode(',',$discounts['noinstructions']);?>];
		var condfair = [<?=implode(',',$discounts['condfair']);?>];
		var condpoor = [<?=implode(',',$discounts['condpoor']);?>];
		var current_discounts = [<?=implode(',',$base_discounts);?>];
		var current_prices = [<?=implode(',',$base_prices);?>];
		var base_discounts = [<?=implode(',',$base_discounts);?>];
		var base_prices = [<?=implode(',',$base_prices);?>];
		var platform_totals = [<?=implode(',',$platform_totals);?>];
		var platform_count = [<?=implode(',',$platform_count);?>];
		var newused = [<?=implode(',',$all_newused);?>];
		var all_total = <?=$all_total;?>;
		var all_count = <?=$all_count;?>;
	</script>
	<?php

	// save the itemID.newused in the session
	$_SESSION['request_itemIDnus'] = $itemIDnus;
	if ($status == MOV_INCOMPLETE) { $_SESSION['request_duped'] = $requestID; }
}

$pg->foot();
?>