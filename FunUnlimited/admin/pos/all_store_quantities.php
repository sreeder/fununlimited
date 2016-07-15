<?php
include('../../include/include.inc');

$itemID = (isset($_GET['itemID'])?$_GET['itemID']:@$_POST['itemID']);

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Item Stock Status');
$pg->head('Item Stock Status');

$sto = new stores($pg);
$error = new error('All-Store Quantities');

if (!strlen($itemID)) { $pg->error('No itemID provided'); }
else
{
	$itm = new items($pg);
	$itm->set_itemID($itemID);
	
	if (!strlen(@$itm->info['title'])) { $pg->error("Invalid itemID: $itemID"); }
	else
	{
		?>
		<font size="2"><b><?="{$itm->info['title']} - {$itm->info['name']}";?></b></font>
		<p />
		<font size="1"><b>Note:</b> All stores that have the item in stock have been highlighted.</font>
		<p />
		<?php
		// get all stores
		$sto = new stores($pg);
		$sto->setStores();
		$stores = $sto->getStores();
		
		$storeIDs = array();
		$store_names = array();
		while (list($a,$arr) = each($stores))
		{
			$storeIDs[] = $arr['sto_storeID'];
			$store_names[$arr['sto_storeID']] = $arr['sto_name'];
		}
		
		// get the quantities for each store
		$qtys = array(); // format: $qtys[storeID] = array(database_row)
		$sql = "SELECT * FROM quantity WHERE qty_storeID IN (".implode(',',$storeIDs).") AND qty_itemID=$itemID";
		$result = mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result)) { $qtys[$row['qty_storeID']] = $row; }
		
		// output the list
		$pg->outlineTableHead();
		?>
		<tr bgcolor="<?=$pg->color('table-head');?>">
			<td align="center"><b>Store</b></td>
			<td align="center"><b>New</b></td>
			<td align="center"><b>Used</b></td>
		</tr>
		<?php
		$shown = -1;
		while (list($storeID,$name) = each($store_names))
		{
			$shown++;
			$bg = ((($shown%2))?$pg->color('table-cell'):$pg->color('table-cell2'));

			$new = @$qtys[$storeID]['qty_new'];
			$used = @$qtys[$storeID]['qty_used'];
			
			if ($new || $used) { $b1 = '<font color="red"><b>'; $b2 = '</b></font>'; } else { $b1 = ''; $b2 = ''; }

			?>
			<tr bgcolor="<?=$bg;?>">
				<td><?=$b1.$name.$b2;?></td>
				<td align="right"><?=$b1.$new.$b2;?></td>
				<td align="right"><?=$b1.$used.$b2;?></td>
			</tr>
			<?php
		}
		$pg->outlineTableFoot();
		?>
		<p />
		<input type="button" value="&lt; Go Back" onclick="history.go(-1)" class="btn" />
		<input type="button" value="Close Window &gt;" onclick="window.close()" class="btn" />
		<?php
	}
}
?>