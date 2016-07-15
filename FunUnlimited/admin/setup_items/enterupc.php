<?php
include('../../include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

$pg = new admin_page();
$pg->setTitle('Item UPC Setup');
$pg->head('Item UPC Setup');

$error = new error('Item UPC Setup');

if ($act == "")
{
	// show the platform selection form
	$platforms = array();

	$sql = "SELECT pla_platformID,pla_name,COUNT(*) FROM platforms,items WHERE pla_platformID=itm_platformID AND itm_upc='' GROUP BY itm_platformID ORDER BY pla_name";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);
	while ($row = mysql_fetch_row($result)) { $platforms[] = $row; }

	?>
	<script type="text/javascript">
		function go(where,platformID)
		{
			var frm = document.pltfrm;
			frm.act.value = where;
			frm.platformID.value = platformID;
			frm.submit();
		}
	</script>
	<form method="post" action="enterupc.php" name="pltfrm">
	<input type="hidden" name="act" value="showitems">
	<input type="hidden" name="platformID" value="">
	</form>

	<font size="1"><b>Note:</b> Only platforms with items are shown.</font>
	<p />
	<?php

	$pg->outlineTableHead();
	?>
	<tr>
		<td bgcolor="<?=$pg->color('table-head-lighter');?>"><b>Platform</b></td>
		<td bgcolor="<?=$pg->color('table-head-lighter');?>"><b>NoUPC#</b></td>
		<td bgcolor="<?=$pg->color('table-head-lighter');?>">&nbsp;</td>
	</tr>
	<?php
	while (list($a,list($platformID,$name,$noupccount)) = each($platforms))
	{
		if ($noupccount)
		{
			?>
			<tr>
				<td bgcolor="<?=$pg->color('table-head-lighter');?>"><?=$name;?></td>
				<td align="right" bgcolor="<?=$pg->color('table-cell2');?>"><?=number_format($noupccount,0);?></td>
				<td bgcolor="<?=$pg->color('table-cell2');?>">
					<input style="width:150px" type="button" value="Enter UPCs &gt;" onclick="go('showitems',<?=$platformID;?>)" class="btn"><br />
					<!--
					<img src="../images/blank.gif" width="1" height="2"><br />
					<input style="width:150px" type="button" value="Create/Print UPCs &gt;" onclick="go('printupc',<?=$platformID;?>)" class="btn">
					-->
				</td>
			</tr>
			<?php
		}
	}
	$pg->outlineTableFoot();
}
elseif ($act == "showitems")
{
	// output the item form and set the UPC numbers if needed
	$platformID = (isset($_GET['platformID'])?$_GET['platformID']:@$_POST['platformID']);
	$focusnum = @$_POST['focusnum'];
	$setUPCs = @$_POST['setUPCs'];

	if (!strlen($focusnum)) { $focusnum = 0; }

	if (count($setUPCs))
	{
		// set the UPCs in the database
		$sqls = array();

		while (list($itemID,$upc) = each($setUPCs))
		{
			$sqls[] = "UPDATE items SET itm_upc='$upc' WHERE itm_platformID=$platformID AND itm_itemID=$itemID";
		}

		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);
		}

		$pg->status("Updated ".count($sqls)." UPC number".(count($sqls)==1?'':'s'));
	}

	$pla = new platforms($pg,$platformID);
	$pla->show_platform(YES,$_SESSION['root_admin'].'setup_items/enterupc.php');

	?>
	<input type="button" value="Add New Item to this Platform &gt;" onclick="document.location='/admin/setup_items/items.php?act=selplatform&platformID=<?=$platformID;?>&fromupc=<?=YES;?>'" class="btn">
	<p />
	<?php

	$splitevery = 50;

	$items = array();
	$sql = "SELECT itm_itemID,itm_title,itm_upc FROM items WHERE itm_platformID=$platformID ORDER BY itm_title";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);
	while ($row = mysql_fetch_row($result)) { $items[] = $row; }

	?>
	<form method="post" action="enterupc.php" name="itmfrm">
	<input type="hidden" name="act" value="showitems">
	<input type="hidden" name="platformID" value="<?=$platformID;?>">
	<input type="hidden" name="focusnum" value="<?=$focusnum;?>">
	<?=$pg->outlineTableHead();?>
	<tr>
		<td bgcolor="<?=$pg->color('table-head-lighter');?>"><b>Title</b></td>
		<td bgcolor="<?=$pg->color('table-head-lighter');?>"><b>UPC</b></td>
	</tr>
	<?php
	while (list($a,list($itemID,$title,$upc)) = each($items))
	{
		if ($a == $focusnum) { $focusfield = "setUPCs[$itemID]"; }

		?>
		<tr>
			<td bgcolor="<?=$pg->color('table-head-lighter');?>"><?=$title;?></td>
			<td bgcolor="<?=$pg->color('table-cell2');?>">
				<input type="text" name="setUPCs[<?=$itemID;?>]" size="25" value="<?=$upc;?>" onkeypress="return checkenter(this,event)">
			</td>
		</tr>
		<?php

		if (!(($a+1)%$splitevery) && $a)
		{
			?>
			<tr>
				<td align="center" colspan="2" bgcolor="<?=$pg->color('table-cell2');?>">
					<input type="submit" value="Set UPC Numbers &gt;" onclick="this.form.focusnum.value=<?=($a+1);?>" class="btn">
				</td>
			</tr>
			<?php
		}
	}
	?>
	<?=$pg->outlineTableFoot();?>
	<p />
	<input type="submit" value="Set UPC Numbers &gt;" name="submitbtn" class="btn" onclick="this.form.focusnum.value=0"> <input type="reset" value="Reset Form &gt;" class="btn">
	</form>
	<?php

	if (strlen(@$focusfield))
	{
		if ($focusnum) { $pg->addOnload("document.itmfrm.submitbtn.focus()"); }
		$pg->addOnload("document.itmfrm.elements['$focusfield'].focus()");
	}
}
elseif ($act == "printupc")
{
	die('COPY BARCODE CODE BACK TO /ADMIN/BARCODE/');
	// output the print barcodes page
	$bc = new barcode();
	$upc = $bc->first_upc;
	$do = 10;

	for ($i=0; $i<$do; $i++)
	{
		$upc = $bc->next_upc($upc);
		?>
		<img src="/admin/barcode/barcodeimage.php?upc=<?=$upc;?>">
		<p />
		<?php
	}
}
elseif ($act == "doprintupc")
{
	// output the printed barcodes
	// two frames: top is button back to 'Print Barcodes' page, bottom is PDF with barcodes
}

$pg->foot();
?>