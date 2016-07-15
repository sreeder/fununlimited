<?php
$force_strip_tabs = 1;
include('../../include/include.inc');

$cl = new check_login();

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$total = (isset($_GET['total'])?$_GET['total']:@$_POST['total']);
if (!strlen($total)) { $total = @$_SESSION['begqty_total']; } else { $_SESSION['begqty_total'] = $total; }
$page = (isset($_GET['page'])?$_GET['page']:@$_POST['page']);
if (!strlen($page)) { $page = 1; }
$per_page = 1500; // number of items to show per-page

$pg = new admin_page();
$pg->setTitle('Whole Platform Quantities');
$pg->head('Whole Platform Quantities');

$error = new error('Whole Platform Quantities');

if ($act == "")
{
	// show the platform selection form
	$platforms = array();
	$platformIDs = array();
	$unique_total = array();
	$all_total = 0;

	// get the current inventory values
	$sumbyplatform = array(); // format: $sumbyplatform[$platformID] = array(ITEM_NEW=>total,ITEM_USED=>total)
	$price_total = array(ITEM_NEW=>0,ITEM_USED=>0);
	$qty_total = array(ITEM_NEW=>0,ITEM_USED=>0);
	$sql = "SELECT itm_platformID,qty_new,qty_used,prc_new,prc_used FROM items,quantity,prices WHERE qty_storeID={$_SESSION['storeID']} AND (qty_new>0 OR qty_used>0) AND qty_itemID=prc_itemID AND prc_itemID=itm_itemID AND itm_active=".YES;
	$result = mysql_query($sql,$db);
	while ($row = mysql_fetch_assoc($result))
	{
		if (!isset($sumbyplatform[$row['itm_platformID']])) { $sumbyplatform[$row['itm_platformID']] = array(ITEM_NEW=>0,ITEM_USED=>0); }
		$sumbyplatform[$row['itm_platformID']][ITEM_NEW] += ($row['qty_new']*$row['prc_new']);
		$sumbyplatform[$row['itm_platformID']][ITEM_USED] += ($row['qty_used']*$row['prc_used']);
		$price_total[ITEM_NEW] += ($row['qty_new']*$row['prc_new']);
		$price_total[ITEM_USED] += ($row['qty_used']*$row['prc_used']);
		$qty_total[ITEM_NEW] += $row['qty_new'];
		$qty_total[ITEM_USED] += $row['qty_used'];
	}

	// get all platforms/unique totals (item count)
	$sql = "SELECT pla_platformID,pla_name,COUNT(qty_itemID),SUM(qty_new),SUM(qty_used) FROM items,platforms,quantity WHERE itm_active=" . YES . " AND itm_platformID=pla_platformID AND itm_itemID=qty_itemID AND qty_storeID={$_SESSION['storeID']} GROUP BY itm_platformID ORDER BY pla_name";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);
	while ($row = mysql_fetch_row($result))
	{
		set_time_limit(30);
		$row[] = @$sumbyplatform[$row[0]][ITEM_NEW];
		$row[] = @$sumbyplatform[$row[0]][ITEM_USED];
		$all_total += $row[2];
		$platformIDs[] = $row[0];
		$unique_total[$row[0]] = 0;
		$platforms[] = $row;
	}

	// get unique totals (on-hand count)
	$sql = "SELECT pla_platformID,COUNT(qty_itemID) AS count FROM items,platforms,quantity WHERE itm_active=" . YES . " AND itm_platformID=pla_platformID AND itm_itemID=qty_itemID AND qty_storeID={$_SESSION['storeID']} AND (qty_new>0 OR qty_used>0) GROUP BY itm_platformID ORDER BY pla_name";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);
	while ($row = mysql_fetch_assoc($result)) { $unique_total[$row['pla_platformID']] = $row['count']; }

	?>
	<script type="text/javascript">
		function go(where,platformID,total)
		{
			var frm = document.pltfrm;
			frm.act.value = where;
			frm.platformID.value = platformID;
			frm.total.value = total;
			frm.submit();
		}
	</script>
	<form method="post" action="begqty.php" name="pltfrm">
		<input type="hidden" name="act" value="showitems">
		<input type="hidden" name="platformID" value="">
		<input type="hidden" name="total" value="">
		<input type="hidden" name="page" value="1">
	</form>

	<?=$pg->outlineTableHead();?>
	<tr>
		<td colspan="4" align="center" bgcolor="<?=$pg->color('table-head');?>">
			<b>Total Inventory Values</b><br />
			(<?=number_format($all_total,0);?> items in database)
		</td>
	</tr>
	<tr>
		<td bgcolor="<?=$pg->color('table-label');?>"><b>New</b></td>
		<td align="right" bgcolor="<?=$pg->color('table-cell');?>"><?=number_format($qty_total[ITEM_NEW],0);?></td>
		<td align="right" bgcolor="<?=$pg->color('table-cell');?>"><?=sprintf('%0.1f',($qty_total[ITEM_NEW]/$all_total*100));?>%</td>
		<td align="right" bgcolor="<?=$pg->color('table-cell');?>">$<?=number_format($price_total[ITEM_NEW],2);?></td>
	</tr>
	<tr>
		<td bgcolor="<?=$pg->color('table-label');?>"><b>Used</b></td>
		<td align="right" bgcolor="<?=$pg->color('table-cell');?>"><?=number_format($qty_total[ITEM_USED],0);?></td>
		<td align="right" bgcolor="<?=$pg->color('table-cell');?>"><?=sprintf('%0.1f',($qty_total[ITEM_USED]/$all_total*100));?>%</td>
		<td align="right" bgcolor="<?=$pg->color('table-cell');?>">$<?=number_format($price_total[ITEM_USED],2);?></td>
	</tr>
	<tr>
		<td bgcolor="<?=$pg->color('table-label');?>"><b>All</b></td>
		<td align="right" bgcolor="<?=$pg->color('table-label');?>"><b><?=number_format(array_sum($qty_total),0);?></b></td>
		<td align="right" bgcolor="<?=$pg->color('table-label');?>">&nbsp;</td>
		<td align="right" bgcolor="<?=$pg->color('table-label');?>"><b>$<?=number_format(array_sum($price_total),2);?></b></td>
	</tr>
	<?=$pg->outlineTableFoot();?>

	<p />
	<font size="1"><b>Note:</b> Only platforms with items are shown.</font>
	<p />
	<?php

	$pg->outlineTableHead();
	?>
	<tr>
		<td bgcolor="<?=$pg->color('table-head-lighter');?>"><b>Platform</b></td>
		<td bgcolor="<?=$pg->color('table-head-lighter');?>" align="center"><b>InDB</b></td>
		<td bgcolor="<?=$pg->color('table-head-lighter');?>" align="center" colspan="2"><b>Total Unique</b></td>
		<td colspan="2" align="center" bgcolor="<?=$pg->color('table-head-lighter');?>"><b>Total New</b></td>
		<td colspan="2" align="center" bgcolor="<?=$pg->color('table-head-lighter');?>"><b>Total Used</b></td>
		<td align="center" bgcolor="<?=$pg->color('table-head-lighter');?>"><b>Value</b></td>
		<td bgcolor="<?=$pg->color('table-head-lighter');?>">&nbsp;</td>
	</tr>
	<?php
	while (list($a,list($platformID,$name,$totitems,$totnew,$totused,$sumnew,$sumused)) = each($platforms))
	{
		if ($totitems)
		{
			$unique = $unique_total[$platformID];

			?>
			<tr>
				<td bgcolor="<?=$pg->color('table-head-lighter');?>"><?=$name;?></td>
				<td align="right" bgcolor="<?=$pg->color('table-cell2');?>"><?=number_format($totitems,0);?></td>
				<td align="right" bgcolor="<?=$pg->color('table-cell');?>"><?=number_format($unique,0);?></td>
				<td align="right" bgcolor="<?=$pg->color('table-cell');?>"><?=sprintf('%0.1f',($unique/$totitems)*100).'%';?></td>
				<td align="right" bgcolor="<?=$pg->color('table-cell2');?>"><?=number_format($totnew,0);?></td>
				<td align="right" bgcolor="<?=$pg->color('table-cell2');?>">$<?=number_format($sumnew,2);?></td>
				<td align="right" bgcolor="<?=$pg->color('table-cell');?>"><?=number_format($totused,0);?></td>
				<td align="right" bgcolor="<?=$pg->color('table-cell');?>">$<?=number_format($sumused,2);?></td>
				<td align="right" bgcolor="<?=$pg->color('table-cell2');?>">$<?=number_format(($sumnew+$sumused),2);?></td>
				<td bgcolor="<?=$pg->color('table-cell2');?>">
					<input style="width:150px" type="button" value="Enter Quantities &gt;" onclick="go('showitems',<?=$platformID;?>,<?=$totitems;?>)" class="btn">
				</td>
			</tr>
			<?php
		}
	}
	$pg->outlineTableFoot();
}
elseif ($act == "showitems")
{
	// output the item form and set the quantities if needed
	$platformID = (isset($_GET['platformID'])?$_GET['platformID']:@$_POST['platformID']);
	$setqtys = @$_POST['setqtys'];
	$continue = YES;

	if (count($setqtys))
	{
		// set the quantities in the database
		$sqls = array();

		while (list($itemID,$arr) = each($setqtys))
		{
			$nqty = $arr[ITEM_NEW];
			$uqty = $arr[ITEM_USED];
			$sqls[] = "UPDATE quantity SET qty_new=$nqty,qty_used=$uqty WHERE qty_storeID=".$_SESSION['storeID']." AND qty_itemID=$itemID";
		}

		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);
		}

		$pg->status("Updated ".count($sqls)." quantit".(count($sqls)==1?'y':'ies'));

		if (@$_POST['tonewitem'] == YES || @$_POST['tochangeplatform'])
		{
			$continue = NO;

			if (@$_POST['tonewitem'] == YES) { $location = $_SESSION['root_admin'].'setup_items/items.php?act=selplatform&platformID='.$platformID.'&fromqty='.YES.'&setupc='.$_POST['newitemupc']; $words = 'add new item page'; }
			if (@$_POST['tochangeplatform'] == YES) { $location = $_SESSION['root_admin'].'setup_items/begqty.php'; $words = 'platform selection page'; }

			?>
			<script type="text/javascript">function goto() { document.location='<?=$location;?>'; }</script>

			Please hold while you are redirected to the <?=$words;?>.
			<p />
			Click <a href="javascript:goto()">here</a> if you are not automatically redirected.
			<?php

			$pg->addOnload('goto()');
		}
	}

	if ($continue == YES)
	{
		$pla = new platforms($pg,$platformID);
		$pla->show_platform(YES,'javascript:changeplatform()');
		$platform_name = $pla->platform_name();

		//$c1 = (!isset($_SESSION['begqty_newused'])||$_SESSION['begqty_newused']==ITEM_NEW?' checked="checked"':'');
		//$c2 = (@$_SESSION['begqty_newused']==ITEM_USED?' checked="checked"':'');
		$c1 = ''; // new check
		$c2 = ' checked="checked"'; // used check

		$_SESSION['begqty_page'] = $page;
		if ($total > $per_page) { $limit = " LIMIT ".(($page-1)*$per_page).",$per_page"; }
		else { $limit = ''; }

		// pull the items to show
		$items = array();
		$sql = "SELECT itm_itemID,itm_title,itm_upc,yer_year,qty_new,qty_used FROM items,years,quantity WHERE itm_active=" . YES . " AND itm_platformID=$platformID AND itm_yearID=yer_yearID AND itm_itemID=qty_itemID AND qty_storeID={$_SESSION['storeID']} ORDER BY itm_title$limit";
		$result = mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_row($result))
		{
			if ($row[0] == @$_GET['itemID'] && isset($_SESSION['begqty_newused']))
			{
				// add 1 to new/used quantity for the newly-added item
				$nu = $_SESSION['begqty_newused'];
				$qtyidx = ($nu==ITEM_NEW?3:4);
				$qtyfield = ($nu==ITEM_NEW?'new':'used');
				$row[$qtyidx]++;

				$qsql = "UPDATE quantity SET qty_$qtyfield={$row[$qtyidx]} WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID={$row[0]}";
				mysql_query($qsql,$db);
				$error->mysql(__FILE__,__LINE__);

				unset($_SESSION['begqty_newused']); // this must be done so that refreshing the page doesn't continue to increment the quantity
			}

			$items[] = $row;
		}

		?>
		<font size="1"><b>Hotkeys:</b> n - new / u - used / z - undo last</font>
		<p />

		<bgsound id="beep" loop="-1" volume="-10000" src="beep.wav" />

		<script language="javascript" src="/scripts/begqty.php?platformID=<?=$platformID;?>&platform_name=<?=$platform_name;?>"></script>

		<div id="upctable" style="position:absolute;top:3;left:3;height:150">
			<?=$pg->outlineTableHead(250);?>
			<tr><td align="center" bgcolor="<?=$pg->color('table-label');?>"><b>Scan Item UPC</b></td></tr>
			<tr>
				<td align="center" bgcolor="<?=$pg->color('table-cell');?>">
					<form name="upc" onsubmit="return false">
						<input type="radio" name="newused" id="n" class="nb"<?=$c1;?> /> <label for="n">New</label> <input type="radio" name="newused" id="u" class="nb"<?=$c2;?> /> <label for="u">Used</label><br />
						UPC: <input type="text" name="upc" size="25" onkeypress="return checkpress(this,event,0,true)" onfocus="lastfocus=this" /><br />
						Title: <input type="text" name="title" size="20" onkeypress="return checkpress(this,event,0,false)" onfocus="lastfocus=this" />
						<!--<input type="button" value="Check &gt;" onclick="if (!locked) { checkpress(this.form.upc,event,13); }" class="btn" />-->
					</form>
				</td>
			</tr>
			<tr><td align="center" bgcolor="<?=$pg->color('table-label');?>"><b>Scanned Item Details</b></td></tr>
			<tr>
				<td align="left" bgcolor="<?=$pg->color('table-cell');?>">
					<b>UPC:</b> <span id="scan_upc"></span><br />
					<b>Title:</b> <span id="scan_title"></span><br />
					<b>Qty:</b> <span id="scan_qty"></span>
				</td>
			</tr>
			<?=$pg->outlineTableFoot();?>
		</div>

		<iframe name="checkupcifrm" src="/admin/setup_items/begqty_checkupc.php" width="1" height="1" frameborder="1" marginwidth="0" marginheight="0">
			Your browser does not support iframes. Please upgrade.
		</iframe>
		<iframe name="setupcifrm" src="/admin/setup_items/begqty_setupc.php" width="1" height="1" frameborder="0" marginwidth="0" marginheight="0">
			Your browser does not support iframes. Please upgrade.
		</iframe>
		<?php

		?>
		<form method="post" action="begqty.php" name="itmfrm">
		<input type="hidden" name="act" value="showitems">
		<input type="hidden" name="platformID" value="<?=$platformID;?>">
		<input type="hidden" name="total" value="<?=$total;?>">
		<input type="hidden" name="page" value="<?=$page;?>">
		<input type="hidden" name="tonewitem" value="<?=NO;?>">
		<input type="hidden" name="newitemupc" value="">
		<input type="hidden" name="tochangeplatform" value="<?=NO;?>">

		<input type="submit" value="Set Quantities &gt;" name="submitbtn" class="btn">
		<input type="button" value="Reset Form &gt;" onclick="reset_form(this.form)" class="btn">
		<p />
		<input type="button" value="&lt; Change Platform" onclick="changeplatform()" class="btn">
		<input type="button" value="Add New Item to this Platform &gt;" onclick="newitem('')" class="btn">
		<p />

		<?php
		if ($total > $per_page)
		{
			?>
			<b>Go To Page:</b> <?php
			$pages = ceil($total/$per_page);
			$links = array();
			for ($i=1; $i<=$pages; $i++)
			{
				if ($i != $page)
				{
					// retrieve the title ranges for that page
					$firstlimit = (($i-1)*$per_page);
					$lastlimit = (($i*$per_page)-1);
					if ($lastlimit > ($total-1)) { $lastlimit = ($total-1); }

					$sqlbase = "SELECT itm_itemID,itm_title,itm_upc FROM items WHERE itm_active=" . YES . " AND itm_platformID=$platformID ORDER BY itm_title ";
					$trimto = 25;

					$sql = $sqlbase."LIMIT $firstlimit,1";
					$result = mysql_query($sql,$db);
					$error->mysql(__FILE__,__LINE__);
					$row = mysql_fetch_assoc($result);
					$firstbound = trim(substr($row['itm_title'],0,$trimto)).(strlen($row['itm_title'])>$trimto?'...':'');

					$sql = $sqlbase."LIMIT $lastlimit,1";
					$result = mysql_query($sql,$db);
					$error->mysql(__FILE__,__LINE__);
					$row = mysql_fetch_assoc($result);
					$lastbound = trim(substr($row['itm_title'],0,$trimto)).(strlen($row['itm_title'])>$trimto?'...':'');
				}

				$links[] = ($i!=$page?'<a href="javascript:goto_page('.$i.')" title="'.$firstbound.' - '.$lastbound.'" style="text-decoration:underline">'.$i.'</a>':"<b>$i</b>");
			}

			echo implode(' ',$links);
			?>
			<p />
			<?php
		}

		$jsinfo = array(); // javascript 'info()' lines
		$pg->outlineTableHead();
		?>
		<tr bgcolor="<?=$pg->color('table-head');?>">
			<td><b>Title</b></td>
			<td width="1">&nbsp;</td>
			<td><b>Year</b></td>
			<td colspan="2"><b>UPC</b></td>
			<td colspan="2"><b>NewQty</b></td>
			<td colspan="2"><b>UsedQty</b></td>
		</tr>
		<?php
		while (list($a,list($itemID,$title,$upc,$year,$nqty,$uqty)) = each($items))
		{
			$bg = (($a%2)?$pg->color('table-cell'):$pg->color('table-cell2'));

			if ($year == 'Unknown') { $year = '&nbsp;'; }

			$upc = trim($upc);
			$jsinfo[] = "info($a,'$upc',$itemID,'".mysql_real_escape_string($title)."')";
			?>
			<tr bgcolor="<?=$bg;?>">
				<td><?=$title;?></td>
				<td class="medgray"><?=$a;?></td>
				<td><?=$year;?></td>
				<td id="upc<?=$a;?>"><?php
					if (strlen($upc)) { echo $upc; } else { echo "&nbsp;"; }
				?></td>
				<td><a href="javascript:set_upc(<?=$a;?>,'<?=$upc;?>')"><img src="/images/setupc.gif" border="0" /></a></td>
				<td align="center">
					<input type="text" name="setqtys[<?=$itemID;?>][<?=ITEM_NEW;?>]" id="q<?=$itemID.ITEM_NEW;?>" size="3" value="<?=$nqty;?>">
				</td>
				<td align="right">
					<a href="javascript:add_amt('q<?=$itemID.ITEM_NEW;?>',1)" title="New+1">+1</a><br />
					<a href="javascript:add_amt('q<?=$itemID.ITEM_NEW;?>',-1)" title="New-1">-1</a>
				</td>
				<td align="center">
					<input type="text" name="setqtys[<?=$itemID;?>][<?=ITEM_USED;?>]" id="q<?=$itemID.ITEM_USED;?>" size="3" value="<?=$uqty;?>">
				</td>
				<td align="right">
					<a href="javascript:add_amt('q<?=$itemID.ITEM_USED;?>',1)" title="Used+1">+1</a><br />
					<a href="javascript:add_amt('q<?=$itemID.ITEM_USED;?>',-1)" title="Used-1">-1</a>
				</td>
			</tr>
			<?php
		}
		?>
		<?=$pg->outlineTableFoot();?>
		<p />
		<input type="submit" value="Set Quantities &gt;" name="submitbtn" class="btn">
		<input type="button" value="Reset Form &gt;" onclick="reset_form(this.form)" class="btn">
		</form>
		<?php

		?><script type="text/javascript"><?=implode(';',$jsinfo);?></script><?php

		$pg->addOnload('move_upc(true)');
	}
}

$pg->foot();
?>