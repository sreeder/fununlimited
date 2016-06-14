<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Doubles List');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

$full = array(''=>YES,'print'=>NO,'select'=>YES,'printselect'=>NO);

$pg = new admin_page();
$pg->setFull($full[$act]);
$pg->setTitle('Doubles List');
$pg->head('Doubles List');

$sto = new stores($pg);
echo '<b>From:</b> '.$sto->getStoreName($_SESSION['storeID']).'<p />';
if ($act == 'print' || $act == 'printselect') { ?><b>Requesting Store:</b> <u>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</u><p /><?php }

if ($act == "")
{
	// output list of doubles with checkboxes
	$sql = "SELECT * FROM quantity,prices,items,platforms WHERE qty_storeID={$_SESSION['storeID']} AND (qty_new>1 OR qty_used>1) AND qty_itemID=prc_itemID AND prc_itemID=itm_itemID AND itm_platformID=pla_platformID ORDER BY pla_name,itm_title";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);

	$items = array();
	while ($row = mysql_fetch_assoc($result)) { $items[] = $row; }

	$byplatform = array(); // format: $byplatform[platform_name] = array(ITEM_NEW=>items,ITEM_USED=>items)
	while (list($a,$arr) = each($items))
	{
		if (!isset($byplatform[$arr['pla_name']])) { $byplatform[$arr['pla_name']] = array(ITEM_NEW=>array(),ITEM_USED=>array()); }
		if ($arr['qty_new'] > 1) { $byplatform[$arr['pla_name']][ITEM_NEW][] = $arr; }
		if ($arr['qty_used'] > 1) { $byplatform[$arr['pla_name']][ITEM_USED][] = $arr; }
	}
	reset($items);

	$_SESSION['doubles'] = $byplatform;

	if (count($items))
	{
		?>
		<script type="text/javascript">
			function print_window() { open_window('doubles.php?act=print','print',725,450,'YES',true); }
		</script>
		<input type="button" value="View Printable List &gt;" class="btn" onclick="print_window()" />
		<p />
		<?php
		show_items($byplatform,'select',YES);
	}
	else
	{
		$pg->error('There are no items with an in-stock quantity of 2 or more.');
	}
}
elseif ($act == "print")
{
	// output a printable list of doubles
	?><font size="1">Please write your store name above, check which items you would like to receive, and fax this list back ASAP.</font><p /><?php
	show_items($_SESSION['doubles']);
	//$pg->addOnload('window.print()');
}
elseif ($act == "select")
{
	// subtract the selected items from the store's quantity
	// (store in a session var for the printable)
	$selitems = @$_POST['selitem'];
	if (!is_array($selitems)) { $selitems = array(); }
	$time = $_POST['time'];

	if ($time != @$_SESSION['doubles_lasttime'])
	{
		$_SESSION['doubles_lasttime'] = $time;

		if (!count($selitems))
		{
			$pg->error('No items selected!');
			?>
			<p />
			<a href="doubles.php">Return to Doubles List</a>
			<?php
		}
		else
		{
			$newitemIDs = array();
			$useditemIDs = array();
			while (list($key,$val) = each($selitems))
			{
				list($itemID,$newused) = explode('|',$key);
				if ($newused == ITEM_NEW) { $newitemIDs[] = $itemID; }
				else { $useditemIDs[] = $itemID; }
			}

			// subtract 1 from the quantities
			$sql = "UPDATE quantity SET qty_new=(qty_new-1) WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID IN (".implode(',',$newitemIDs).")";
			//mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);

			$sql = "UPDATE quantity SET qty_used=(qty_used-1) WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID IN (".implode(',',$useditemIDs).")";
			//mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);

			// get new/used prices
			$sql = "SELECT * FROM prices,items,platforms WHERE prc_itemID IN (".implode(',',array_merge($newitemIDs,$useditemIDs)).") AND prc_itemID=itm_itemID AND itm_platformID=pla_platformID ORDER BY pla_name,itm_title";
			$result = mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);

			$items = array();
			while ($row = mysql_fetch_assoc($result)) { $items[] = $row; }

			$pricebyplatform = array(); // format: $pricebyplatform[platform_name]= array(ITEM_NEW=>items,ITEM_USED=>items)
			while (list($a,$arr) = each($items))
			{
				if (!isset($pricebyplatform[$arr['pla_name']])) { $pricebyplatform[$arr['pla_name']] = array(ITEM_NEW=>array(),ITEM_USED=>array()); }
				if (in_array($arr['itm_itemID'],$newitemIDs)) { $pricebyplatform[$arr['pla_name']][ITEM_NEW][] = $arr; }
				if (in_array($arr['itm_itemID'],$useditemIDs)) { $pricebyplatform[$arr['pla_name']][ITEM_USED][] = $arr; }
			}
			reset($items);

			$_SESSION['doubles_prices'] = $pricebyplatform;

			$pg->status('Removed 1 from quantity of each selected item');
			?>
			The price totals for each selected item/platform are displayed below.<br />
			Click <b>View Printable List</b> to view a printable list of the below items.
			<p />
			<script type="text/javascript">
				function print_window() { open_window('doubles.php?act=printselect','printselect',725,450,'YES',true); }
			</script>
			<input type="button" value="View Printable List &gt;" class="btn" onclick="print_window()" />
			<p />
			<?php
			show_prices($pricebyplatform);
		}
	}
	else
	{
		$pg->error("<b>DO NOT repost the form - you will mess up your quantities!</b>");
		?>
		<a href="doubles.php">Return to Doubles List</a>
		<?php
	}
}
elseif ($act == "printselect")
{
	// output a printable list of the selected items
	show_prices($_SESSION['doubles_prices']);
	$pg->addOnload('window.print()');
}

// output item list
function show_items($byplatform,$act='',$check=NO)
{
	$cols = 3;
	$rows = count($byplatform);
	$keys = array_keys($byplatform);
	$width = floor(100/$cols);

	$table_width = 650;
	$col_width = ceil($table_width/$cols)-ceil(10/$cols);

	$max_title_len = 35;

	if ($check == YES)
	{
		?>
		Click <b>View Printable List</b> to print a faxable-version of the below list.<br />
		When the list is returned to you, check which items you have shipped<br />
		and click <b>Select Sent Items</b>.
		<p />
		<script type="text/javascript">
			function check_all(frm,chck)
			{
				for (var i=0; i<frm.elements.length; i++)
				{
					if (frm.elements[i].type == 'checkbox') { frm.elements[i].checked = chck; }
				}
			}
		</script>
		<form method="post" action="doubles.php">
		<input type="hidden" name="act" value="<?=$act;?>">
		<input type="hidden" name="time" value="<?=time();?>">
		<input type="button" value="Check All &gt;" onclick="check_all(this.form,true)" class="btn" />
		<input type="button" value="Uncheck All &gt;" onclick="check_all(this.form,false)" class="btn" />
		<p />
		<?php
	}
	?>
	<style type="text/css">
		.cr { font-family:Courier New;font-size:8pt; }
	</style>

	<table border="0" bordercolor="#000000" cellspacing="0" cellpadding="2" width="100%">
	<?php
	for ($i=0; $i<$rows; $i++)
	{
		$key = @$keys[$i];
		$items = @$byplatform[$key];
		$show = array($items[ITEM_NEW],$items[ITEM_USED]);

		?>
		<tr><td colspan="<?=$cols+1;?>" align="center">
			<hr width="100%" size="-1" color="#000000" noshade="noshade" /><br />
			<b><?=$key;?></b>
		</td></tr>
		<?php
		while (list($a,$nu) = each($show))
		{
			if (count($nu))
			{
				if ($a && count($show[0])) { ?><tr><td colspan="<?=$cols+1;?>" align="center"><hr width="100%" size="-1" color="#000000" noshade="noshade" /></td></tr><?php }
				?>
				<tr>
					<td background="/images/vert_<?=(!$a?'new':'used');?>.gif" width="10"><img src="/images/blank.gif" width="10" height="1" /></td>
					<?php
					if (is_array($nu))
					{
						$percol = ceil(count($nu)/$cols);

						for ($j=0; $j<$cols; $j++)
						{
							$start = ($j*$percol);
							$end = (($j+1)*$percol)-1;

							?>
							<td valign="top" width="<?=$width;?>%">
								<table border="0" cellspacing="2" cellpadding="0" width="100%">
									<tr>
										<td valign="top" style="font-size:9">
											<?php
											$newused = (!$a?ITEM_NEW:ITEM_USED);
											$shown = 0;

											for ($k=$start; $k<=$end; $k++)
											{
												$arr = @$nu[$k];

												if (is_array($arr))
												{
													$shown++;
													$price = ($newused==ITEM_NEW?$arr['prc_new']:$arr['prc_used']);
													$formatprice = ($price>0?number_format($price,2):'');

													$title = $arr['itm_title'];
													if (strlen($title) > $max_title_len) { $title = substr($title,0,($max_title_len-3)).'...'; }

													if ($check == YES) { ?><input type="checkbox" name="selitem[<?=$arr['itm_itemID'];?>|<?=$newused;?>]" id="chk<?=$arr['itm_itemID'];?><?=$newused;?>" class="nb"><label for="chk<?=$arr['itm_itemID'];?><?=$newused;?>"><?php }
													else { ?>[&nbsp;&nbsp;] <?php }
													?><span class="cr"><?=(strlen($formatprice)<6?str_repeat('&nbsp;',(6-strlen($formatprice))):'').$formatprice;?></span> <?=$title;?><?php
													if ($check == YES) { ?></label><?php }
													?><br /><?php
													echo "\n";
												}
											}

											if (!$shown) { echo "&nbsp;"; }
											?>
										</td>
									</tr>
								</table>
							</td>
							<?php
						}
					}
					else { ?><td colspan="<?=$cols;?>">&nbsp;</td><?php }
				?></tr><?php
			}
		}
	}
	?>
	</table>
	<?php

	if ($check == YES)
	{
		?>
		<p />
		<font size="1">
			<b>Note:</b> If you check ANY items which you did not ship, your quantities will be messed up!<br />
			Please make sure you have checked <b>ONLY</b> the items you shipped!
		</font>
		<p />
		<input type="submit" value="Select Sent Items &gt;" class="btn" />
		</form>
		<?php
	}
}

// show platform totals
function show_prices($byplatform)
{
	die('TELL SCOTT CARPENTER WHEN YOU SEE THIS!');
	$cols = 3;
	$rows = ceil(count($byplatform)/$cols);
	$keys = array_keys($byplatform);
	$width = floor(100/$cols);
	$alltotal = 0;

	?>
	<table border="1" cellspacing="0" cellpadding="5" width="100%">
	<?php
	for ($i=0; $i<$rows; $i++)
	{
		?><tr><?php
		for ($j=0; $j<$cols; $j++)
		{
			$key = @$keys[($i*$cols)+$j];
			$items = @$byplatform[$key];

			if (is_array($items))
			{
				?>
				<td valign="top" width="<?=$width;?>%">
					<table border="0" cellspacing="2" cellpadding="0" width="100%">
						<tr>
							<td valign="top" style="font-size:10">
								<?php
								$total = 0;
								$show = array($items[ITEM_NEW],$items[ITEM_USED]);

								while (list($a,$nu) = each($show))
								{
									if (count($nu))
									{
										$newused = (!$a?ITEM_NEW:ITEM_USED);
										$nutot = 0;
										if ($a) { ?><p /><?php }
										?>
										<b><?=$key;?> (<?=(!$a?'New':'Used');?>)</b><br />
										<?php
										while (list($b,$arr) = each($nu))
										{
											$price = ($newused==ITEM_NEW?$arr['prc_new']:$arr['prc_used']);
											$nutot += $price;
											$formatprice = '$'.number_format($price,2);
											$total += $price;
											?><span style="font-family:Courier"><?=str_pad($formatprice,6,'�',STR_PAD_LEFT);?></span> <?=$arr['itm_title'];?><br /><?php
											echo "\n";

											if (($b+1) == count($nu)) { ?><span style="font-family:Courier;font-size:6"><b><?=str_pad('$'.number_format($nutot,2),6,'�',STR_PAD_LEFT);?></b></span><?php }
										}
									}
								}

								$alltotal += $total;
								?>
								<p />
								<font size="2"><b><u><?=strtoupper($key);?> TOTAL</u>:<br />$<?=number_format($total,2);?></b></font>
							</td>
						</tr>
					</table>
				</td>
				<?php
			}
			else { ?><td>&nbsp;</td><?php }
		}
		?></tr><?php
	}
	?>
	</table>
	<p />
	<b><u>GRAND TOTAL</u>: $<?=number_format($alltotal,2);?></b>
	<?php
}

$pg->foot();
?>