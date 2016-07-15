<?php
include('../../include/include.inc');

$cl = new check_login();

$page = (isset($_GET['page'])?$_GET['page']:@$_POST['page']);
if (!is_numeric($page)) { $page = 1; }

$pg = new admin_page();
$pg->setTitle('End of Year Item Quantities');
$pg->head();

$error = new error('End of Year Item Quantities');

$criteria = @$_SESSION['endofyear_item_criteria'];

if (isset($criteria['platformID']) && isset($criteria['yearID']))
{
	$platformID = $criteria['platformID'];
	$yearID = $criteria['yearID'];
	$pla = new platforms($pg,$platformID);
	include_once('items_elements.php');
	$year = $years[$yearID];

	// pull the items for the selected platform
	$itemIDs = array();
	$items = array();
	$result = runQuery($platformID,'*');
	while ($row = mysql_fetch_assoc($result))
	{
		$itemIDs[] = $row['itm_itemID'];
		$items[] = $row;
	}

	// get the end-of-year quantities
	$eoy_qtys = array();
	if (count($itemIDs))
	{
		$sql = "SELECT * FROM endofyear_items WHERE eyi_yearID=$yearID AND eyi_itemID IN (".implode(',',$itemIDs).")";
		$result = mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result)) { $eoy_qtys[$row['eyi_itemID']] = array(ITEM_NEW=>$row['eyi_new'],ITEM_USED=>$row['eyi_used']); }
	}
	$eoy_count = count($eoy_qtys);
	$not_entered = (count($itemIDs)-$eoy_count);

	$pg->pageHead('Complete Platform');
	$pla->show_platform(YES,'/admin/endofyear/items.php',NO);
	?><b>Year:</b> <?=$year;?>

	<p />

	Please verify the following information before completing the above platform.
	<p />

	<?=$pg->outlineTableHead();?>
		<tr>
			<td bgcolor="<?=$pg->color('table-label');?>"><b>Total Items in Platform:</b></td>
			<td bgcolor="<?=$pg->color('table-cell');?>" align="right"><?=count($itemIDs);?></td>
		</tr>
		<tr>
			<td bgcolor="<?=$pg->color('table-label');?>"><b>Total Quantities Entered:</b></td>
			<td bgcolor="<?=$pg->color('table-cell');?>" align="right"><?=$eoy_count;?></td>
		</tr>
		<?php
		if ($not_entered)
		{
			?>
			<tr>
				<td bgcolor="<?=$pg->color('table-label');?>"><b>Total Quantities <u>NOT</u> Entered: <font color="red">*</font></b></td>
				<td bgcolor="<?=$pg->color('table-cell');?>" align="right"><?=$not_entered;?></td>
			</tr>
			<?php
		}
		?>
	<?=$pg->outlineTableFoot();?>
	<p />
	<?php

	if ($not_entered)
	{
		?><font color="red"><b>ITEMS WITH NO QUANTITIES WILL BE DEFAULTED TO 0 NEW/USED ON-HAND!!!</b></font><p /><?php
	}

	// output the table of quantities that will be changed
	?>The following items will have their quantities changed:<p /><?php
	$changed = 0;
	$default = array();
	$pg->outlineTableHead();
	?>
	<tr bgcolor="<?=$pg->color('table-head');?>">
		<td rowspan="2" align="center"><b>Title</b></td>
		<td colspan="2" align="center"><b>New</b></td>
		<td colspan="2" align="center"><b>Used</b></td>
	</tr>
	<tr bgcolor="<?=$pg->color('table-head');?>">
		<td align="center"><b>Current</b></td>
		<td align="center"><b>Actual</b></td>
		<td align="center"><b>Current</b></td>
		<td align="center"><b>Actual</b></td>
	</tr>
	<?php
	while (list($a,$arr) = each($items))
	{
		$itemID = $arr['itm_itemID'];
		$ohnew = $arr['qty_new'];
		$ohused = $arr['qty_used'];
		$actnew = @$eoy_qtys[$itemID][ITEM_NEW];
		$actused = @$eoy_qtys[$itemID][ITEM_USED];

		if (!strlen($actnew) || !strlen($actused))
		{
			// blank value; default to 0
			$actnew = 0;
			$actused = 0;
			$defaulted = YES;

			$default[] = $itemID;
		}
		else { $defaulted = NO; }

		if ($ohnew != $actnew || $ohused != $actused)
		{
			$changed++;
			$bg = (($changed%2)?$pg->color('table-cell'):$pg->color('table-cell2'));

			$shownew = ($ohnew!=$actnew?YES:NO);
			$showused = ($ohused!=$actused?YES:NO);

			?>
			<tr style="background-color:<?=$bg;?>;font-weight:<?=($defaulted?'bold':'none');?>">
				<td><?=$arr['itm_title'];?></td>
				<?php
				if ($shownew)
				{
					?>
					<td align="right"><?=$ohnew;?></td>
					<td align="right"><?=$actnew;?></td>
					<?php
				}
				else { ?><td align="center" colspan="2"><font color="#DDDDDD">NC</font></td><?php }

				if ($showused)
				{
					?>
					<td align="right"><?=$ohused;?></td>
					<td align="right"><?=$actused;?></td>
					<?php
				}
				else { ?><td align="center" colspan="2"><font color="#DDDDDD">NC</font></td><?php }
				?>
			</tr>
			<?php
		}
	}
	if (!$changed) { ?><tr><td colspan="5" bgcolor="<?=$pg->color('table-cell');?>" align="center">--- No Changes ---</td></tr><?php }
	$pg->outlineTableFoot();

	if (count($default)) { ?><font size="1"><b>Note:</b> bold titles/quantities indicate that quantities were defaulted to 0</font><?php }
	$_SESSION['endofyear_defaults'] = $default;
	?>

	<script type="text/javascript">
		function makeSure()
		{
			if (!<?=$changed;?> || confirm('Are you ABSOLUTELY sure you want to complete this platform?\nThis will PERMANENTLY change the on-hand quantities of all items listed.\nTHIS IS NOT REVERSIBLE!!!'))
			{
				document.location = '/admin/endofyear/itemsUpdate.php?act=complete';
			}
		}
	</script>

	<p />
	<input type="button" value="&lt; Return to Quantity Entry" onclick="document.location='/admin/endofyear/items_form.php'" class="btn" />
	<input type="button" value="Complete Platform &gt;" onclick="makeSure()" class="btn" />
	<?php
}
else { $pg->error('Invalid criteria'); }

$pg->foot();
?>