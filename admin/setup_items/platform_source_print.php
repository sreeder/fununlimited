<?php
include('../../include/include.inc');

$pg = new admin_page();
$error = new error('Platform Source Print');

$pg->setFull(NO);
$pg->head();

$array = @$_SESSION['newpricearray'];

if (!count($array))
{
	?><script type="text/javascript">alert('Session variable not set\n\nPlease tell Scott! (scott@payforstay.com)')</script><?php
}
else
{
	$platformID = $array['platformID'];
	$items = $array['items'];
	$itemIDs = array_keys($items);

	$sql = "SELECT itm_itemID,itm_title FROM items WHERE itm_itemID IN (".implode(',',$itemIDs).")";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);
	while ($row = mysql_fetch_assoc($result)) { $items[$row['itm_itemID']][2] = $row['itm_title']; }

	?>
	<font size="4"><b>Changed Price List</b></font>
	<p />
	<?php
	$pla = new platforms($pg,$platformID);
	$pla->show_platform(NO,'',YES);

	$pg->outlineTableHead();
	?>
	<tr>
		<td bgcolor="#FFFFFF" align="center"><b>Title</b></td>
		<td bgcolor="#FFFFFF" align="center"><b>New</b></td>
		<td bgcolor="#FFFFFF" align="center"><b>Used</b></td>
	</tr>
	<?php

	while (list($itemID,list($new,$used,$title)) = each($items))
	{
		?>
		<tr>
			<td bgcolor="#FFFFFF"><?=$title;?></td>
			<td bgcolor="#FFFFFF" align="right">$<?=number_format($new,2);?></td>
			<td bgcolor="#FFFFFF" align="right">$<?=number_format($used,2);?></td>
		</tr>
		<?php
	}

	$pg->outlineTableFoot();

	?><script type="text/javascript">parent.printbtn.disabled=false</script><?php
}

$pg->foot();
?>