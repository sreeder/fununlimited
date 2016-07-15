<?php
include('../../include/include.inc');

$act        = getGP('act');
$itemID     = getGP('itemID');
$val        = getGP('val');
$platformID = getGP('platformID');
$type       = getGP('type');
$multiple   = NO;
$store      = getG('store'); // is this an item request from the online store?
$isinvoice  = getGP('isinvoice');
$frombare   = getGP('frombare');
if ($isinvoice)
{
	$_SESSION['invoice_focus_type'] = getGP('focus_type',SALE);
}

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Item Information');
$pg->head();

$sto = new stores($pg);
$error = new error('Item Information');

if (!$frombare && $act == 'setinfo')
{
	// prices
	$prices = getG('prices');
	if (!$prices[ITEM_NEW]) { $prices[ITEM_NEW] = 0.0; }
	if (!$prices[ITEM_USED]) { $prices[ITEM_USED] = 0.0; }

	// set the prices
	$prc = new prices($pg);
	$prc->setPrice($itemID,$prices[ITEM_NEW],$prices[ITEM_USED]);

	// quantity
	$quantity = getG('quantity');
	if (!$quantity[ITEM_NEW]) { $quantity[ITEM_NEW] = 0; }
	if (!$quantity[ITEM_USED]) { $quantity[ITEM_USED] = 0; }

	$sql = 'UPDATE quantity SET qty_new=' . $quantity[ITEM_NEW] . ',qty_used=' . $quantity[ITEM_USED] . " WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID=$itemID";
	mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);

	$info_set = YES;
}

if (!strlen($itemID) && !strlen($val)) { quick_lookup(); $pg->error('No itemID or UPC/title provided'); }
elseif ($act == 'showpage' || (!strlen($itemID) && strlen($val)))
{
	// look up the itemID for a UPC/title

	$its = new item_search($pg);
	$its->action = $_SESSION['root_admin'].'pos/iteminfo.php';
	$its->max_results = 250;
	$its->criteria['upctitle'] = $val;
	$its->criteria['platformID'] = $platformID;
	$its->criteria['storeID'] = $_SESSION['storeID'];

	if ($act == 'showpage')
	{
		$page = getG('page');
	}
	else
	{
		$page = 1;
	}

	$its->search('yer_year DESC,itm_title','',$page);

	if (count($its->results) == 1)
	{
		// found 1 item
		$itemID = $its->results[0]['itm_itemID'];
	}
	elseif (count($its->results) > 1) { $multiple = YES; }
	else { $pg->pageHead('No Match'); quick_lookup(); echo "No item found for UPC/title <b>$val</b>"; }
}

if ($multiple)
{
	$pg->pageHead('Select Item');
	quick_lookup();

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
	<?php

	$its->showSmallResults(NO,array('type'=>$type,'hadmultiple'=>YES),YES);
}

if (strlen($itemID))
{
	$itm = new items($pg);
	$itm->set_itemID($itemID);

	$yr = new years(); // pull in the current years
	$pla = new platforms($pg,$itm->info['platformID']);

	if (!$store && $type == 'instock')
	{
		// increment the lookup count for this item

		// delete all entries from 30+ days ago
		$monthago = (strtotime(date('m/d/Y'))-(60*60*24*30));
		$sql = "DELETE FROM quick_lookups WHERE qck_storeID={$_SESSION['storeID']} AND qck_itemID=$itemID AND qck_time<=$monthago";
		mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);

		// insert a new lookup request
		$sql = "INSERT INTO quick_lookups VALUES ({$_SESSION['storeID']},$itemID,'" . time() . "')";
		mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);
	}

	// platform
	$pla->set_item('platforms');
	while (list($a,$arr) = each($pla->values))
	{
		if ($arr[0] == @$itm->info['platformID'])
		{
			$platform = $arr[1];
			break;
		}
	}

	// year
	while (list($a,$arr) = each($yr->years))
	{
		if ($arr['yer_yearID'] == @$itm->info['yearID'])
		{
			$year = $arr['yer_year'];
			break;
		}
	}

	// company 1
	$pla->set_item('companies');
	while (list($a,$arr) = each($pla->values))
	{
		if ($arr[0] == @$itm->info['company1ID'])
		{
			$company1 = $arr[1];
			break;
		}
	}

	// company 2
	reset($pla->values);
	while (list($a,$arr) = each($pla->values))
	{
		if ($arr[0] == @$itm->info['company2ID'])
		{
			$company2 = $arr[1];
			break;
		}
	}

	// type
	$pla->set_item('types');
	while (list($a,$arr) = each($pla->values))
	{
		if ($arr[0] == @$itm->info['typeID'])
		{
			$type = $arr[1];
			break;
		}
	}

	$output = array();
	$output[] = array('Item Information');
	$output[] = array('UPC','<span id="upc">'.$itm->info['upc'].'</span>'.(!$frombare&&!$store?' <a href="javascript:set_upc(\''.$itm->info['upc'].'\')"><img src="/images/setupc.gif" width="54" height="11" border="0" /></a>':''));
	$output[] = array('Title',$itm->info['title']);
	$output[] = array('Platform',$platform);
	$output[] = array('Description',nl2br($itm->info['description']));
	$output[] = array('Age',$itm->info['age']);
	$output[] = array('Year',@$year);
	$output[] = array('Company'.(isset($company2)?' 1':''),@$company1);
	$output[] = array('Company 2',@$company2);
	$output[] = array('Type',@$type);

	// custom fields
	$pla->set_item('fields');
	while (list($a,$arr) = each($pla->values))
	{
		$desc = $arr[1];
		$val = @$itm->info['item_field_values'][$arr[0]];
		$output[] = array($desc,$val);
	}

	// features
	$features = array();
	$pla->set_item('features');
	while (list($a,$arr) = each($pla->values))
	{
		if (in_array($arr[0],@$itm->info['features']))
		{
			$features[] = $arr[1];
		}
	}
	$output[] = array('Features',implode('<br />',$features));

	// prices/ratings
	$pla->set_item('sources');

	$price = array();
	$rating = array();
	$vals = array();

	while (list($a,$arr) = each($pla->values))
	{
		if ($arr[2] == PRICE) { $price[] = $arr; } else { $rating[] = $arr; }
	}
	while (list($a,$arr) = each($price))
	{
		if (strlen(@$itm->info['iteminfo'][$arr[0]])) { $vals[] = "$arr[1]: \$".@$itm->info['iteminfo'][$arr[0]]; }
	}
	if (count($vals)) { $vals[] = ''; }
	while (list($a,$arr) = each($rating))
	{
		if (strlen(@$itm->info['iteminfo'][$arr[0]])) { $vals[] = "$arr[1]: ".@$itm->info['iteminfo'][$arr[0]]; }
	}
	$output[] = array('Prices/Ratings',implode('<br />',$vals));

	// notes
	$output[] = array('Notes',(!$store&&!$frombare?note_form($itemID,@$itm->info['notes']):nl2br(@$itm->info['notes'])));

	?>
	<script type="text/javascript">
		function allstoreqtys(itemID) { document.location = '/admin/pos/all_store_quantities.php?itemID='+itemID; }
	</script>

	<font size="4"><b><?=$itm->info['title'];?></b></font><br />
	<font size="1"><b><?=$platform;?></b></font>
	<p />
	<?php
	quick_lookup();

	if (!$frombare && @$info_set)
	{
		// output the status line and force an update of the current invoice
		$pg->status("Prices set to <b>".$prices[ITEM_NEW]."/".$prices[ITEM_USED]."</b><br />Quantities set to <b>".$quantity[ITEM_NEW]."/".$quantity[ITEM_USED]."</b>");

		if ($isinvoice)
		{
			?>
			<script type="text/javascript">
				window.opener.location='/admin/pos/invoice.php?act=view';
				window.close();
			</script>
			<?php
		}
	}

	if (!$store)
	{
		?>
		<script type="text/javascript">
			var oqnew = <?=$itm->info['qty_new'];?>;
			var oqused = <?=$itm->info['qty_used'];?>;

			function verify(frm)
			{
				var fqnew = frm.elements['quantity[<?=ITEM_NEW;?>]'].value;
				var fqused = frm.elements['quantity[<?=ITEM_USED;?>]'].value;
				if (!fqnew.length) { fqnew = 0; }
				if (!fqused.length) { fqused = 0; }

				if (fqnew != oqnew || fqused != oqused) { return confirm('You altered the quantities - are you ABSOLUTELY SURE they are correct?\nNote: Invalid quantities can mess many things up!'); }
				else { return true; }
			}

			// set the UPC for an item
			function set_upc(fillupc)
			{
				var itemID = <?=$itemID;?>;
				var upc = prompt('Please enter the UPC for the item titled: <?=mysql_real_escape_string($itm->info['title']);?>',fillupc);

				if (upc != null)
				{
					setupcifrm.location = '/admin/setup_items/begqty_setupc.php?idx=0&itemID='+itemID+'&upc='+upc;
				}
			}
			// alert that the UPC is already set
			function upc_exists(upc,title,platform)
			{
				alert('The UPC '+upc+' already exists\n\nPlatform: '+platform+'\nItem: '+title);
			}
			// change the shown UPC for a given item
			function change_upc(idx,upc)
			{
				var obj = document.getElementById('upc');
				obj.innerText = upc;
				unlock_upc();
			}
			// dummy function
			function unlock_upc() { }

			function editItem()
			{
				var url = '/admin/setup_items/items.php?act=edit&itemID=<?php echo $itemID;?>&popup=<?php echo YES;?>&isinvoice=<?php echo $isinvoice;?>';
				<?php
				if ($isinvoice)
				{
					?>
					go(url);
					<?php
				}
				else
				{
					?>
					open_window(url,'edititem',500,500,'YES',true);
					<?php
				}
				?>
			}
		</script>

		<iframe name="setupcifrm" src="/admin/setup_items/begqty_setupc.php" width="1" height="1" frameborder="0" marginwidth="0" marginheight="0">
			Your browser does not support iframes. Please upgrade.
		</iframe>

		<?=$pg->outlineTableHead();?>
		<tr>
			<td bgcolor="<?=$pg->color('table-cell');?>" align="center">
				<table border="0" cellspacing="3" cellpadding="0">
					<form method="get" action="iteminfo.php" onsubmit="return verify(this)">
					<input type="hidden" name="act" value="setinfo">
					<input type="hidden" name="frombar" value="<?=$frombare;?>">
					<input type="hidden" name="itemID" value="<?=$itemID;?>">
					<input type="hidden" name="isinvoice" value="<?=$isinvoice;?>">
					<tr>
						<td width="130">
							<b>New Price:</b> $<input type="text" name="prices[<?=ITEM_NEW;?>]" size="7" value="<?=(strlen(@$itm->info['price'][ITEM_NEW])?@$itm->info['price'][ITEM_NEW]:'');?>" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value)" style="text-align:right" />
						</td>
						<td align="right" class="medgray">
							<?=(strlen(@$itm->info['price'][ITEM_NEW]) ? 'Orig: $' . @$itm->info['price'][ITEM_NEW] : '');?>
						</td>
						<td rowspan="2">&nbsp;</td>
						<td rowspan="2" bgcolor="#000000" width="1"></td>
						<td rowspan="2">&nbsp;</td>
						<td width="130">
							<b>Used Price:</b> $<input type="text" name="prices[<?=ITEM_USED;?>]" size="7" value="<?=(strlen(@$itm->info['price'][ITEM_USED])?@$itm->info['price'][ITEM_USED]:'');?>" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value)" style="text-align:right" />
						</td>
						<td align="right" class="medgray">
							<?=(strlen(@$itm->info['price'][ITEM_USED]) ? 'Orig: $' . @$itm->info['price'][ITEM_USED] : '');?>
						</td>
					</tr>
					<tr>
						<td width="130">
							<b>New Quantity:</b> <input type="text" name="quantity[<?=ITEM_NEW;?>]" size="3" value="<?=(strlen(@$itm->info['qty_new'])?@$itm->info['qty_new']:'0');?>" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right" />
						</td>
						<td align="right" class="medgray">
							<?=(strlen(@$itm->info['qty_new']) ? 'Orig: ' . @$itm->info['qty_new'] : '');?>
						</td>
						<td width="130">
							<b>Used Quantity:</b> <input type="text" name="quantity[<?=ITEM_USED;?>]" size="3" value="<?=(strlen(@$itm->info['qty_used'])?@$itm->info['qty_used']:'0');?>" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right" />
						</td>
						<td align="right" class="medgray">
							<?=(strlen(@$itm->info['qty_new']) ? 'Orig: ' . @$itm->info['qty_used'] : '');?>
						</td>
					</tr>
				</table>
			</td>
		</tr>
		<?php
		if (!$frombare)
		{
			?>
			<tr bgcolor="<?=$pg->color('table-head');?>">
				<td align="center">
					<input type="submit" value="Set Prices/Quantities &gt;" class="btn" /> <input type="reset" value="Reset Prices/Quantities &gt;" class="btn">
				</td>
			</tr>
			<?php
		}
		?>
		<?=$pg->outlineTableFoot();?>
		</form>
		<p />
		<?php
	}

	if (!$isinvoice && getG('hadmultiple'))
	{
		?>
		<input type="button" value="&lt; Return to Search Results" onclick="history.go(-1)" class="btn" />
		<p />
		<?php
	}
	if ($frombare)
	{
		?>
		<input type="button" value="&lt; Return to Barebones Item Search" onclick="document.location='/admin/bare/items.php'" class="btn" />
		<input type="button" value="View Stock Status in Every Store &gt;" onclick="allstoreqtys(<?=$itemID;?>)" class="btn" />
		<p />
		<?php
	}
	else
	{
		?>
		<input type="button" value="View Stock Status in Every Store &gt;" onclick="allstoreqtys(<?=$itemID;?>)" class="btn" />
		<input type="button" value="Add Item to Wishlist &gt;" onclick="window.opener.location='/admin/utilities/wishlist.php?use_itemID=<?=$itemID;?>';window.close()" class="btn" />
		<p />
		<?php
	}
	?>
	<p />
	<table border="0" cellspacing="5" cellpadding="0">
		<tr>
			<td colspan="2" align="center" valign="top">
				<?php
				if ($itm->info['box_imgID'])
				{
					$path = $itm->image_path($itm->info['box_imgID']);
					?>
					<p class="bold">
						<span class="note">Box Image</span><br />
						<img src="<?=$path;?>" width="150" border="0" alt="Box Image" />
					</p>
					<?php
				}
				if ($itm->info['nobox_imgID'])
				{
					$path = $itm->image_path($itm->info['nobox_imgID']);
					?>
					<p class="bold">
						<span class="note">No Box Image</span><br />
						<img src="<?=$path;?>" width="150" border="0" alt="No Box Image" />
					</p>
					<?php
				}
				?>
			</td>
			<td valign="top" align="center">
				<?php
				$pg->outlineTableHead('200');
				while (list($a,$arr) = each($output))
				{
					if (count($arr) == 1)
					{
						?>
						<tr>
							<td colspan="2" align="center" bgcolor="<?=$pg->color('table-label');?>"><b><?=$arr[0];?></b></td>
						</tr>
						<?php
					}
					else
					{
						list($t,$v) = $arr;

						if (strlen(trim($v)))
						{
							?>
							<tr>
								<td bgcolor="<?=$pg->color('table-label');?>"><b><?=$t;?>:</b></td>
								<td bgcolor="<?=$pg->color('table-cell');?>"><?=$v;?></td>
							</tr>
							<?php
						}
					}
				}
				$pg->outlineTableFoot();

				if (!$store)
				{
					?>
					<p />
					<input type="button" value="&lt; Close" onclick="self.close()" class="btn" />
					<input type="button" value="Edit Item Information &gt;" onclick="editItem()" class="btn" />
					<?php
				}
				?>
			</td>
			<td valign="top">
				<?php
				// output the cash/credit values
				$ccp = new ccpercs($pg);
				$def_credit = $ccp->percs[0][0];
				$cr = @$ccp->percs[$itm->info['platformID']][0];
				if (!strlen($cr)) { $cr = $def_credit; }
				$cr = sprintf('%0.3f',$cr);
				$ca = sprintf('%0.3f',($cr/2));
				$crv = number_format(($itm->info['price'][ITEM_USED]*($cr/100)),2);
				$cav = number_format(($itm->info['price'][ITEM_USED]*($ca/100)),2);

				$pg->outlineTableHead();
				?>
				<tr><td colspan="3" align="center" bgcolor="<?=$pg->color('table-head');?>"><b>Base Trade Amounts</b> <font color="red">*</font></td></tr>
				<tr>
					<td bgcolor="<?=$pg->color('table-label');?>"><b>Credit</b></td>
					<td align="right" bgcolor="<?=$pg->color('table-cell');?>"><?=rtrim(rtrim($cr,'0'),'.');?>%</td>
					<td align="right" bgcolor="<?=$pg->color('table-cell');?>">$<?=$crv;?></td>
				</tr>
				<tr>
					<td bgcolor="<?=$pg->color('table-label');?>"><b>Cash</b></td>
					<td align="right" bgcolor="<?=$pg->color('table-cell');?>"><?=rtrim(rtrim($ca,'0'),'.');?>%</td>
					<td align="right" bgcolor="<?=$pg->color('table-cell');?>">$<?=$cav;?></td>
				</tr>
				<tr>
					<td colspan="2" bgcolor="<?=$pg->color('table-label');?>"><b>New Price</b></td>
					<td align="right" bgcolor="<?=$pg->color('table-cell');?>">$<?=number_format($itm->info['price'][ITEM_NEW],2);?></td>
				</tr>
				<?php
				$pg->outlineTableFoot();
				?>
			</td>
		</tr>
	</table>
	<p />
	<font size="1"><font color="red">*</font> Base trade amounts are before any additional discounts (box, condition, etc)</font>
	<?php
}

if (!$frombare)
{
	?><div style="width:98%;text-align:right;padding-bottom:1px"><input type="button" value="Close &gt;" class="btn" onclick="window.close()"></div><?php
}

$pg->foot();

function note_form($itemID,$notes)
{
	$return = '';

	$return .= '<form method="get" action="'.$_SESSION['root_admin'].'pos/iteminfo_setnotes.php" target="noteifrm">';
	$return .= '<input type="hidden" name="itemID" value="'.$itemID.'">';
	$return .= '<textarea name="notes" rows="5" cols="40">'.$notes.'</textarea><br />';
	$return .= '<input type="submit" value="Update Notes &gt;" class="btn">';
	$return .= '</form>';
	$return .= '<iframe name="noteifrm" width="200" height="15" src="'.$_SESSION['root_admin'].'pos/iteminfo_setnotes.php" frameborder="0" marginwidth="0" marginheight="0" scrolling="no">';
	$return .= 'You\'re browser is too old to support this page!  Please <a href="http://www.microsoft.com/ie">update your browser.</a>';
	$return .= '</iframe>';

	return $return;
}

function quick_lookup()
{
	global $pg,$frombare;

	$pg->outlineTableHead();
	?>
	<tr><td bgcolor="<?=$pg->color('table-cell');?>"><?=$pg->quick_lookup_form(NO,'#000000',YES,$frombare);?></td></tr>
	<?php
	$pg->outlineTableFoot();
	?><p /><?php
}
?>