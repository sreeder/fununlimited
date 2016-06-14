<?php
include('../../include/include.inc');

$cl = new check_login();

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

$pg = new admin_page();
$pg->setTitle('Whole Platform Source Pricing');
$pg->head('Whole Platform Source Pricing');

$error = new error('Whole Platform Source Pricing');

if ($act == "")
{
	// show the platform selection form
	$platformIDs = array();
	$platforms = array();
	$pla = new platforms($pg,0);
	$pla->set_item('platforms');
	while (list($a,$arr) = each($pla->values))
	{
		$platformIDs[] = $arr[0];
		$platforms[] = $arr;
	}

	$newplatforms = array();
	$sql = "SELECT pla_name,itm_platformID,COUNT(itm_platformID) AS count FROM items,platforms WHERE itm_platformID=pla_platformID GROUP BY itm_platformID ORDER BY pla_name";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);
	while ($row = mysql_fetch_assoc($result))
	{
		$platformID = $row['itm_platformID'];
		$count = $row['count'];

		$totprice = 0;
		$pla->platformID = $platformID;
		$pla->set_item('sources');
		while (list($a,$arr) = each($pla->values))
		{
			if ($arr[2] == PRICE) { $totprice++; }
		}

		if ($totprice)
		{
			$idx = array_search($platformID,$platformIDs);
			$newplatforms[] = $platforms[$idx];
		}
	}
	$platforms = $newplatforms;

	?>
	Select a platform:

	<script type="text/javascript">
		function verify(frm)
		{
			if (frm.platformID.selectedIndex == 0) { alert('You must select a platform.'); frm.platformID.focus(); return false; }
			else { return true; }
		}
	</script>
	<form method="post" action="/admin/setup_items/platform_source.php" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="selsource">
		<select name="platformID" size="1" style="vertical-align:middle"><option value=""></option><?php
			while (list($a,list($platformID,$name)) = each($platforms))
			{
				?><option value="<?=$platformID;?>"><?=$name;?></option><?php
			}
		?></select>
		<p />
		<input type="submit" value="Select Platform &gt;" class="btn">
	</form>
	<p />
	<font size="1"><b>Note:</b> Only platforms which contain items and pricing sources are selectable.</font>
	<?php
}
elseif ($act == "selsource")
{
	$platformID = (isset($_GET['platformID'])?$_GET['platformID']:@$_POST['platformID']);

	$pla = new platforms($pg,$platformID);
	$pla->show_platform(YES,$_SESSION['root_admin'].'setup_items/platform_source.php');
	$pla->set_item('sources');

	if (count($pla->values))
	{
		// show the pricing source selection form
		?>
		Select a pricing source:

		<script type="text/javascript">
			function verify(frm)
			{
				if (frm.sourceID.selectedIndex == 0) { alert('You must select a pricing source.'); frm.sourceID.focus(); return false; }
				else { return true; }
			}
		</script>
		<form method="post" action="/admin/setup_items/platform_source.php" onsubmit="return verify(this)">
			<input type="hidden" name="act" value="viewprices">
			<input type="hidden" name="platformID" value="<?=$platformID;?>">
			<select name="sourceID" size="1" style="vertical-align:middle"><option value=""></option><?php
				while (list($a,$arr) = each($pla->values))
				{
					if ($arr[2] == PRICE)
					{
						?><option value="<?=$arr[0];?>"><?=$arr[1];?></option><?php
					}
				}
			?></select>
			<p />
			<input type="submit" value="Select Pricing Source &gt;" class="btn">
		</form>
		<?php
	}
}
elseif ($act == "viewprices")
{
	$platformID = $_POST['platformID'];
	$sourceID = $_POST['sourceID'];

	$pla = new platforms($pg,$platformID);
	$pla->show_platform(YES,$_SESSION['root_admin'].'setup_items/platform_source.php',NO);
	$pla->set_item('sources');
	while (list($a,$arr) = each($pla->values))
	{
		if ($arr[0] == $sourceID)
		{
			?>
			<font size="2"><b>Pricing Source:</b> <?=$arr[1];?></font>
			<font size="1">(<a href="/admin/setup_items/platform_source.php?act=selsource&platformID=<?=$platformID;?>">Change</a>)</font>
			<p />
			<?php
			break;
		}
	}

	$itemIDs = array();
	$items = array();
	$sql = "SELECT itm_itemID,itm_title FROM items WHERE itm_platformID=$platformID ORDER BY itm_title";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);
	while ($row = mysql_fetch_assoc($result))
	{
		$itemIDs[] = $row['itm_itemID'];
		$items[] = array($row['itm_itemID'],$row['itm_title'],'');
	}

	$sql = "SELECT isv_itemID,isv_value FROM item_source_values WHERE isv_sourceID=$sourceID AND isv_itemID IN (".implode(',',$itemIDs).")";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);
	while ($row = mysql_fetch_assoc($result))
	{
		$itemID = $row['isv_itemID'];
		$idx = array_search($itemID,$itemIDs);
		$items[$idx][2] = $row['isv_value'];
	}
	?>

	<script type="text/javascript">
		// remove every visible value
		function clearall()
		{
			var frm = document.itmfrm;

			for (var i=0; i<frm.elements.length; i++)
			{
				if (frm.elements[i].type == 'text') { frm.elements[i].value = ''; }
			}
		}
	</script>

	<input type="button" value="Clear All Values &gt;" onclick="clearall()" class="btn">
	<p />

	<form method="post" action="/admin/setup_items/platform_source.php" name="itmfrm">
		<input type="hidden" name="act" value="getprices">
		<input type="hidden" name="platformID" value="<?=$platformID;?>">
		<input type="hidden" name="sourceID" value="<?=$sourceID;?>">

		<?php
		$pg->outlineTableHead();
		?>
		<tr>
			<td bgcolor="<?=$pg->color('table-head');?>"><b>Title</b></td>
			<td align="center" bgcolor="<?=$pg->color('table-head');?>"><b>OrigPrice</b></td>
			<td align="center" bgcolor="<?=$pg->color('table-head');?>"><b>NewPrice</b></td>
		</tr>
		<?php

		while (list($a,list($itemID,$title,$price)) = each($items))
		{
			?>
			<input type="hidden" name="origprice[<?=$itemID;?>]" value="<?=$price;?>">
			<tr>
				<td bgcolor="<?=$pg->color('table-cell');?>"><?=$title;?></td>
				<td align="right" bgcolor="<?=$pg->color('table-cell');?>"><?=($price?'$'.$price:'');?></td>
				<td align="center" bgcolor="<?=$pg->color('table-cell2');?>">$<input type="text" name="setprice[<?=$itemID;?>]" size="6" onkeypress="return onlynumbers(this.value,event,true,true)" onblur="this.value=format_price(this.value,false)" style="text-align:right" value="<?=$price;?>">
			</tr>
			<?php
		}

		$pg->outlineTableFoot();
		?>

		<p />
		<input type="submit" value="Update Source Pricing &gt;" class="btn">
	</form>
	<?php
}
elseif ($act == "getprices")
{
	// get the prices and output the new price selection form

	$platformID = $_POST['platformID'];
	$sourceID = $_POST['sourceID'];
	$setprice = @$_POST['setprice'];
	$origprice = @$_POST['origprice'];

	$pla = new platforms($pg,$platformID);
	$pla->show_platform(NO,'',NO);
	$pla->set_item('sources');
	while (list($a,$arr) = each($pla->values))
	{
		if ($arr[0] == $sourceID)
		{
			?><font size="2"><b>Pricing Source:</b> <?=$arr[1];?></font><p /><?php
			break;
		}
	}

	$itemIDs = array_keys($setprice);
	$changedIDs = array();

	$sql = "DELETE FROM item_source_values WHERE isv_itemID IN (".implode(',',$itemIDs).") AND isv_sourceID=$sourceID";
	mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);

	$titles = array();
	$sql = "SELECT itm_itemID,itm_title FROM items WHERE itm_itemID IN (".implode(',',$itemIDs).") ORDER BY itm_title";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);
	while ($row = mysql_fetch_assoc($result)) { $titles[$row['itm_itemID']] = $row['itm_title']; }

	$sql = "INSERT INTO item_source_values VALUES ";
	$vals = array();
	while (list($itemID,$val) = each($setprice))
	{
		if (strlen($val)) { $vals[] = "($itemID,$sourceID,$val)"; }

		if ($val != $origprice[$itemID]) { $changedIDs[] = $itemID; }
	}
	if (count($vals))
	{
		$sql .= implode(',',$vals);
		mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);
	}

	$itm = new items($pg);
	//$changedprices = $itm->update_new_prices($changedIDs,NO);
	$changedprices = array();

	//$pg->status('Changed '.count($changedIDs).' source price'.(count($changedIDs)!=1?'s':''));
	$pg->status('No new prices changed (this has been disabled!)');

	if (count($changedprices))
	{
		// some new prices are different - ask if they would like to change them

		$orignewprices = array();
		$usedprices = array();
		$getusedIDs = array_keys($changedprices);
		$sql = "SELECT prc_itemID,prc_new,prc_used FROM prices WHERE prc_itemID IN (".implode(',',$getusedIDs).")";
		$result = mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result))
		{
			$orignewprices[$row['prc_itemID']] = $row['prc_new'];
			$usedprices[$row['prc_itemID']] = $row['prc_used'];
		}

		?>
		<script type="text/javascript">
			var itemIDs = new Array();

			// uncheck every box
			var allchk = false;
			function docheckall()
			{
				for (var i=0; i<itemIDs.length; i++) { docheck(itemIDs[i],allchk); }
				chkbtn.value = (allchk==true?'Uncheck':'Check')+' All >';
				allchk = (allchk==true?false:true);
			}

			function docheck(itemID,chk)
			{
				var obj = eval("document.prcfrm.elements['doset["+itemID+"]']");
				obj.checked = chk;

				var obj = eval('prcfrm.used'+itemID);
				obj.disabled = (chk==true?false:true);
			}

			function verify(frm)
			{
				noused = false;
				for (var i=0; i<frm.elements.length; i++)
				{
					if (frm.elements[i].type == 'text' && !frm.elements[i].value.length) { noused = true; break; }
				}

				if (noused) { alert('You must enter a used value for every item'); frm.elements[i].focus(); return false; }
				else { return true; }
			}
		</script>

		Some of the items you changed source prices for may have a lower new price.<br />
		Check which new prices you would like to set below. You may also change the used price.
		<p />

		<input type="button" id="chkbtn" value="Uncheck All &gt;" onclick="docheckall()" class="btn">
		<p />

		<form method="post" action="/admin/setup_items/platform_source.php" name="prcfrm" onsubmit="return verify(this)">
			<input type="hidden" name="act" value="setprices">
			<input type="hidden" name="platformID" value="<?=$platformID;?>">
			<input type="hidden" name="sourceID" value="<?=$sourceID;?>">

			<?php
			$pg->outlineTableHead();
			?>
			<tr>
				<td bgcolor="<?=$pg->color('table-head');?>">&nbsp;</td>
				<td bgcolor="<?=$pg->color('table-head');?>"><b>Title</b></td>
				<td align="center" bgcolor="<?=$pg->color('table-head');?>"><b>OrigNewPrice</b></td>
				<td align="center" bgcolor="<?=$pg->color('table-head');?>"><b>NewPrice</b></td>
				<td align="center" bgcolor="<?=$pg->color('table-head');?>"><b>UsedPrice</b></td>
			</tr>
			<?php

			while (list($itemID,$newprice) = each($changedprices))
			{
				?>
				<script type="text/javascript">itemIDs[itemIDs.length]=<?=$itemID;?></script>
				<input type="hidden" name="setprice[<?=$itemID;?>]" value="<?=$newprice;?>">
				<tr>
					<td align="center" bgcolor="<?=$pg->color('table-cell');?>"><input type="checkbox" name="doset[<?=$itemID;?>]" value="<?=YES;?>" checked="checked" onclick="docheck(<?=$itemID;?>,this.checked)" class="nb"></td>
					<td bgcolor="<?=$pg->color('table-cell');?>"><?=$titles[$itemID];?></td>
					<td align="right" bgcolor="<?=$pg->color('table-cell');?>">$<?=number_format($orignewprices[$itemID],2);?></td>
					<td align="right" bgcolor="<?=$pg->color('table-cell');?>">$<?=number_format($newprice,2);?></td>
					<td align="center" bgcolor="<?=$pg->color('table-cell2');?>">$<input type="text" id="used<?=$itemID;?>" name="usedprice[<?=$itemID;?>]" size="6" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value,false)" style="text-align:right" value="<?=@$usedprices[$itemID];?>">
				</tr>
				<?php
			}

			$pg->outlineTableFoot();
			?>

			<p />
			<input type="submit" value="Set New/Used Pricing &gt;" class="btn"> <input type="reset" value="Reset Form &gt;" class="btn">
		</form>
		<?php
	}
	else
	{
		// no new prices are different
		?>
		<script type="text/javascript">
			function go(where)
			{
				var frm = document.actfrm;

				if (where == 'newplatform') { frm.submit(); }
				else if (where == 'newsource') { frm.act.value = 'selsource'; frm.submit(); }
			}
		</script>
		<form method="post" action="/admin/setup_items/platform_source.php" name="actfrm">
			<input type="hidden" name="act" value="">
			<input type="hidden" name="platformID" value="<?=$platformID;?>">
		</form>

		None of the source prices you entered are the new lowest for the items.<br />
		No new item prices will be set.
		<p />
		<input type="button" value="&lt; Select New Platform" onclick="go('newplatform')" class="btn">
		<p />
		<input type="button" value="&lt; Select New Pricing Source" onclick="go('newsource')" class="btn">
		<?php
	}
}
elseif ($act == "setprices")
{
	// set the checked prices

	$platformID = $_POST['platformID'];
	$sourceID = $_POST['sourceID'];
	$doset = @$_POST['doset'];
	$setprice = @$_POST['setprice'];
	$usedprice = @$_POST['usedprice'];

	if (!is_array($doset)) { $doprice = array(); }
	if (!is_array($setprice)) { $setprice = array(); }
	if (!is_array($usedprice)) { $usedprice = array(); }

	$pla = new platforms($pg,$platformID);
	$pla->show_platform(NO,'',YES);

	$itm = new items($pg);
	$prc = new prices($pg);

	$setitemIDs = array_keys($doset);

	if (count($setitemIDs))
	{
		$itemarr = array();
		$sqls = array();

		while (list($itemID,$new) = each($setprice))
		{
			// set the prices
			$new = $itm->fix_price($new);
			$used = $itm->fix_price($usedprice[$itemID]);
			$prc->setPrice($itemID,-1,$used); // $new is now -1 - new prices changed manually

			$itemarr[$itemID] = array($new,$used,'');
		}

		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);
		}

		$totalset = count($sqls);
	}
	else
	{
		// none of the checkboxes were checked
		$totalset = 0;
	}

	?>
	<script type="text/javascript">
		function go(where)
		{
			var frm = document.actfrm;

			if (where == 'newplatform') { frm.submit(); }
			else if (where == 'newsource') { frm.act.value = 'selsource'; frm.submit(); }
		}
	</script>
	<form method="post" action="/admin/setup_items/platform_source.php" name="actfrm">
		<input type="hidden" name="act" value="">
		<input type="hidden" name="platformID" value="<?=$platformID;?>">
	</form>

	<?php
	if ($totalset)
	{
		$_SESSION['newpricearray'] = array('platformID'=>$platformID,'items'=>$itemarr);

		?>
		<?=$totalset;?> new/used price<?=($totalset!=1?'s':'');?> set.
		<p />
		<input type="button" value="Print Changed Price List &gt;" onclick="itmifrm.print()" id="printbtn" class="btn" disabled="true">
		<iframe name="itmifrm" src="/admin/setup_items/platform_source_print.php" width="1" height="1" frameborder="0" marginwidth="0" marginheight="0">
			Your browser does not support iframes. Please upgrade.
		</iframe>
		<?php
	}
	else { echo count($sqls).' new/used price'.(count($sqls)!=1?'s':'').' set'; }
	?>
	<p />
	<input type="button" value="&lt; Select New Platform" onclick="go('newplatform')" class="btn">
	<p />
	<input type="button" value="&lt; Select New Pricing Source" onclick="go('newsource')" class="btn">
	<?php
}

$pg->foot();
?>