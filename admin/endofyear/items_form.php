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

	// set/pull the global variables/functions
	include('items_elements.php');
	$year = $years[$yearID];

	// get total items in platform
	$result = runQuery($platformID,'COUNT(*) AS count');
	$row = mysql_fetch_assoc($result);
	$total = $row['count'];

	// make sure the platformID is marked as incomplete for the given year
	$sql = "DELETE FROM endofyear_platforms WHERE eyp_platformID=$platformID AND eyp_yearID=$yearID";
	mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);

	$sql = "INSERT INTO endofyear_platforms VALUES ($platformID,$yearID,".NO.",0)";
	mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);

	// number of items to show per-page and item index boundaries
	$perpage = 100;
	$start = (($page-1)*$perpage);

	// pull the items for the selected platform
	$items = array();
	$itemIDs = array();
	$result = runQuery($platformID,'*',"LIMIT $start,$perpage");
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

	$pg->pageHead('End of Year Item Quantities');
	$pla->show_platform(YES,'/admin/endofyear/items.php',NO);
	?>[ <b>Year:</b> <?=$year;?> / <b>Page #<?=$page;?></b> ]<p /><?php

	if (!$total)
	{
		// there are no items in this platform; mark it as completed
		$sql = "UPDATE endofyear_platforms SET eyp_completed=" . YES . ",eyp_completedtime=".time()." WHERE eyp_platformID=$platformID AND eyp_yearID=$yearID";
		mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);

		?>
		There are no items in the above platform. It has been marked as completed.
		<p />
		<input type="button" value="&lt; Return to Platform Selection" onclick="document.location='/admin/endofyear/items.php'" class="btn" />
		<?php
	}
	else
	{
		if (isset($_GET['updated'])) { $pg->status(($_GET['updated']?''.@$_GET['count'].' quantities have been stored':'No quantities were stored')); }

		if (!count($items))
		{
			?>
			There are no items in the platform/on the page you have selected.
			<p />
			<input type="button" value="&lt; Return to Platform Selection" onclick="document.location='/admin/endofyear/items.php'" class="btn" />
			<?php
		}
		else
		{
			// output the item form
			?>
			<script type="text/javascript">
				// add an amount to the quantity in the given field
				function add_amt(idx,amt)
				{
					var obj = document.getElementById(idx);
					obj.value = parseInt(obj.value)+amt;
					if (obj.value > 0) { play_sound(obj.value); }
					if (parseInt(obj.value) < 0) { obj.value = 0; }
				}

				// play the sound
				function play_sound(times)
				{
					var obj = document.getElementById('beep');
					obj.volume = 0;
					obj.loop = times;
					obj.src = '/admin/setup_items/beep.wav';
				}
			</script>
			<bgsound id="beep" loop="-1" volume="-10000" src="/admin/setup_items/beep.wav" />

			Enter the new/used quantity for the following items. When <b>Complete Platform</b> is pressed,<br />
			the quantities entered will become the current on-hand quantities. Any fields left blank will default to 0.<br />
			<b>Keep in mind that this has an impact on many aspects of the software!</b>
			<p />
			<?php

			// get the boundaries
			$start = 0; $end = $perpage;
			if ($end > count($items)) { $end = count($items); }
			$addforindex = (($page-1)*$perpage);
			$heads_every = 20;

			$pg->outlineTableHead();
			?>
			<form method="post" action="/admin/endofyear/itemsUpdate.php" id="itmfrm">
			<input type="hidden" name="act" value="updateitems" />
			<input type="hidden" name="page" value="<?=$page;?>" />
			<input type="hidden" name="yearID" value="<?=$yearID;?>" />
			<?php
			$idx = -1;
			$head_count = -1;
			for ($i=$start; $i<$end; $i++)
			{
				$idx++;
				$head_count++;
				$arr = $items[$i];
				$itemID = $arr['itm_itemID'];
				$bg = (($idx%2)?$pg->color('table-cell'):$pg->color('table-cell2'));

				if (!($head_count%$heads_every))
				{
					// output the colum headers
					?>
					<tr bgcolor="<?=$pg->color('table-head');?>">
						<td rowspan="2">&nbsp;</td>
						<td rowspan="2" align="center"><b>Title</b></td>
						<td colspan="3" align="center"><b>New Qty</b></td>
						<td colspan="3" align="center"><b>Used Qty</b></td>
					</tr>
					<tr bgcolor="<?=$pg->color('table-head');?>">
						<td align="center"><b>OH</b></td>
						<td colspan="2" align="center"><b>Actual</b></td>
						<td align="center"><b>OH</b></td>
						<td colspan="2" align="center"><b>Actual</b></td>
					</tr>
					<?php
				}

				$new_field = "$itemID|".ITEM_NEW;
				$used_field = "$itemID|".ITEM_USED;
				$newqty = @$eoy_qtys[$itemID][ITEM_NEW];
				$usedqty = @$eoy_qtys[$itemID][ITEM_USED];
				if (!$newqty) { $newqty = 0; }
				if (!$usedqty) { $usedqty = 0; }

				/*
				// !!! TEMPORARY !!!
				if (!$newqty) { $newqty = $arr['qty_new']; }
				if (!$usedqty) { $usedqty = $arr['qty_used']; }
				*/

				?>
				<tr bgcolor="<?=$bg;?>">
					<td align="right"><font color="#CCCCCC"><?=($addforindex+$i+1);?></font></td>
					<td><?=$arr['itm_title'];?></td>
					<td align="right" style="width:25px"><?=$arr['qty_new'];?></td>
					<td>
						<input type="text" id="qty<?=$new_field;?>" name="qtys[<?=$itemID;?>][<?=ITEM_NEW;?>]" size="3" value="<?=$newqty;?>" onkeypress="return onlynumbers(this.value,event,true)" />
					</td>
					<td align="right">
						<a href="javascript:add_amt('qty<?=$new_field;?>',1)" title="New+1">+1</a><br />
						<a href="javascript:add_amt('qty<?=$new_field;?>',-1)" title="New-1">-1</a>
					</td>
					<td align="right" style="width:25px"><?=$arr['qty_used'];?></td>
					<td>
						<input type="text" id="qty<?=$used_field;?>" name="qtys[<?=$itemID;?>][<?=ITEM_USED;?>]" size="3" value="<?=$usedqty;?>" onkeypress="return onlynumbers(this.value,event,true)" />
					</td>
					<td align="right">
						<a href="javascript:add_amt('qty<?=$used_field;?>',1)" title="Used+1">+1</a><br />
						<a href="javascript:add_amt('qty<?=$used_field;?>',-1)" title="Used-1">-1</a>
					</td>
				</tr>
				<?php
			}
			$pg->outlineTableFoot();
			?>
			<p />
			<?=submitNavCode($platformID,$page);?>
			</form>
			<?php
		}
	}
}
else { $pg->error('Invalid criteria'); }

/**
* Return array of pages and the first title of the page
* @param $platformID
* @return array
*/
function getFirstTitles($platformID)
{
	global $perpage;

	$titles = array();

	// get total items in platform
	$result = runQuery($platformID,'COUNT(*) AS count');
	$row = mysql_fetch_assoc($result);
	$total = $row['count'];

	$pages = ceil($total/$perpage);
	for ($i=0; $i<$pages; $i++)
	{
		$first_index = ($i*$perpage);
		$result = runQuery($platformID,'itm_title',"LIMIT $first_index,1");
		$row = mysql_fetch_assoc($result);
		$titles[$i] = $row['itm_title'];
	}

	return array($total,$pages,$titles);
}

/**
* Output the submit/navigation code
* @param integer $platformID
* @param integer $page current page
*/
function submitNavCode($platformID,$page)
{
	global $pg;

	list($total,$pages,$titles) = getFirstTitles($platformID);

	?>
	<script type="text/javascript">
		function gotoPage(page)
		{
			var obj = document.getElementById('itmfrm');
			if (!obj.doupdate.checked || confirm("Are you sure the above items' quantities are correct?\n\nNote: this does NOT change the on-hand quantities! That happens when you click 'Complete Platform'"))
			{
				if (!obj.doupdate.checked && page == <?=$page;?>) { alert("You're already there..."); } // there's no point in going to the same page...
				else
				{
					obj.page.value = page;
					obj.submit();
				}
			}
		}

		function completePlatform()
		{
			document.location = '/admin/endofyear/items_complete.php';
		}
	</script>

	<?=$pg->outlineTableHead(800);?>
		<tr>
			<td align="center" bgcolor="<?=$pg->color('table-cell2');?>">
				If selected, all submit buttons will store the quantities and take you to the named page.
				<p />
				<font color="red"><b>
					It may take a minute to submit the form and store the quantities.<br />
					Please be patient and press the button <u>ONLY ONCE!!!</u>
				</font></b>
				<p />
				<b>Current Page:</b> #<?=$page;?> of <?=$pages;?><br />
				<b>Total Items in Platform:</b> <?=number_format($total,0);?><br />
				&nbsp;
			</td>
		</tr>
		<tr>
			<td align="center" bgcolor="<?=$pg->color('table-cell');?>">
				&nbsp;<br />
				<input type="checkbox" name="doupdate" id="doupdate" value="<?=YES;?>" checked="checked" class="nb" /> <label for="doupdate">Store Quantities When Button Pressed</label> (uncheck this if you just want to switch pages)
				<p />
				<input type="button" value="&lt; Previous Page" onclick="gotoPage(<?=($page-1);?>)" alt="Page <?=($page-1);?>" class="btn" style="width:150px"<?=(!($page-1)?' disabled="disabled"':'');?> />
				<input type="button" value="Same Page" onclick="gotoPage(<?=$page;?>)" alt="Page <?=$page;?>" class="btn" style="width:150px" />
				<input type="button" value="Next Page &gt;" onclick="gotoPage(<?=($page+1);?>)" alt="Page <?=($page+1);?>" class="btn" style="width:150px"<?=(($page+1)>$pages?' disabled="disabled"':'');?> />
				<p />
				<select id="pages" size="1"><?php
					while (list($p,$title) = each($titles))
					{
						$pgnum = ($p+1);
						if ($pgnum == $page) { $s = ' selected="selected"'; } else { $s = ''; }
						?><option value="<?=$pgnum;?>"<?=$s;?>><?="Page #$pgnum - ".substr($title,0,30).(strlen($title)>30?'...':'');?></option><?php
					}
				?></select>
				<input type="button" value="Selected Page &gt;" onclick="gotoPage(document.getElementById('pages').options[document.getElementById('pages').selectedIndex].value)" class="btn" style="width:150px" />
				<p />
				<font size="1"><b>Note:</b> The title in the select box is the first title on that page.</font>
			</td>
		</tr>
	<?=$pg->outlineTableFoot();?>
	<p />
	<input type="button" value="Complete Platform &gt;" onclick="completePlatform()" class="btn" />
	<?php
}

$pg->foot();
?>