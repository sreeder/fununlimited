<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Print Inventory Status');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle("Inventory Status - {$_SESSION['store_info']['sto_name']}");
$pg->head("Inventory Status - {$_SESSION['store_info']['sto_name']}");

if ($act == "print")
{
	// output a printable inventory list
	$platformIDs = @$_POST['platforms'];
	$instock = @$_POST['instock'];

	if (!count($platformIDs)) { $pg->error('No platforms selected!'); }
	else
	{
		// output the inventory list
		if ($instock) { $qty_where = '(qty_new>0 OR qty_used>0)'; }
		else { $qty_where = '(qty_new=0 AND qty_used=0)'; }

		$allitems = array(); // format: $allitems[platformID][#] = array(info)
		$platforms = array(); // format: $platforms[platformID] = name
		$sql = "SELECT * FROM platforms,items,quantity WHERE pla_platformID IN (".implode(',',$platformIDs).") AND pla_platformID=itm_platformID AND itm_itemID=qty_itemID AND qty_storeID={$_SESSION['storeID']} AND {$qty_where} ORDER BY pla_name,itm_title";
		$result = mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result))
		{
			if (!isset($allitems[$row['pla_platformID']]))
			{
				$allitems[$row['pla_platformID']] = array();
				$platforms[$row['pla_platformID']] = $row['pla_name'];
			}
			$allitems[$row['pla_platformID']][] = $row;
		}

		$cols = 3;
		$colwidth = floor(100/$cols);
		$max_title_length = 25;

		?>
		<b>Showing:</b> <?=($instock?'In-Stock Items':'Out-of-Stock Items');?>
		<p />
		<font size="2"><b>Format:</b> [title] [new qty]/[used qty]</font>
		<p />
		<input type="button" value="Print &gt;" onclick="window.print()" class="btn" />
		<input type="button" value="Close &gt;" onclick="window.close()" class="btn" />
		<p />
		<table border="0" cellspacing="0" cellpadding="2" width="100%">
			<?php
			while (list($platformID,$items) = each($allitems))
			{
				$numeach = ceil(count($items)/$cols);

				?>
				<tr><td colspan="<?=$cols;?>"><hr width="100%" size="-1" color="#000000" noshade="noshade" /></td></tr>
				<tr><td align="center" colspan="<?=$cols;?>"><b><?=$platforms[$platformID];?></b></td></tr>
				<tr>
					<?php
					for ($i=0; $i<$cols; $i++)
					{
						$start = ($i*$numeach);
						$end = ((($i+1)*$numeach)-1);
						if ($end > (count($items)-1)) { $end = (count($items)-1); }

						?><td width="<?=$colwidth;?>%" valign="top"><table border="0" cellspacing="0" cellpadding="0" width="100%"><?php
						for ($j=$start; $j<=$end; $j++)
						{
							$thisitem = $items[$j];
							$title = substr($thisitem['itm_title'],0,$max_title_length).(strlen($thisitem['itm_title'])>$max_title_length?'':'');
							$newqty = @$thisitem['qty_new'];
							$usedqty = @$thisitem['qty_used'];
							?><tr><td width="100%"><?=$title;?></td><td align="right"><?=$newqty;?></td><td>/</td><td align="right"><?=$usedqty;?></td>
							</tr><?php
						}
						?></table></td><?php
					}
					?>
				</tr>
				<?php
			}
			?>
			<tr><td colspan="<?=$cols;?>"><hr width="100%" size="-1" color="#000000" noshade="noshade" /></td></tr>
		</table>
		<?php

		//$pg->addOnload('window.print()');
	}
}

$pg->foot();
?>
