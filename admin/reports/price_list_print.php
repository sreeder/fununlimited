<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Print Price List');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle("Price List - {$_SESSION['store_info']['sto_name']}");
$pg->head("Price List - {$_SESSION['store_info']['sto_name']}");

if ($act == "print")
{
	// output a printable price list
	$platformIDs = getP('platforms');
	$instock = getP('instock');
	$show0 = getP('show0');

	if (!count($platformIDs))
	{
		$pg->error('No platforms selected!');
	}
	else
	{
		// output the price list
		$qty_table = '';
		$qty_where = '';
		if ($instock)
		{
			$qty_table = ',quantity';
			$qty_where = " AND qty_storeID={$_SESSION['storeID']} AND qty_itemID=itm_itemID AND (qty_new>0 OR qty_used>0)";
		}

		$allitems = array(); // format: $allitems[platformID][#] = array(info)
		$platforms = array(); // format: $platforms[platformID] = name
		$sql = "
			SELECT
				*
			FROM
				platforms,items,prices$qty_table
			WHERE
				pla_platformID IN " . getIn($platformIDs) . "
				AND pla_platformID=itm_platformID
				AND itm_active=" . YES . "
				AND itm_itemID=prc_itemID
				AND prc_used" . ($show0 ? '>=' : '>') . "0
				$qty_where
			ORDER BY
				pla_name,itm_title
		";
		$result = mysql_query($sql, $db);
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

		$cols = 4;
		$colwidth = floor(100/$cols);
		$max_title_length = 25;

		?>
		<font size="2"><b>Format:</b> [title] [used price]<?php echo ($instock?" [new qty] / [used qty]":'');?></font>
		<?php
		if ($instock) { ?><br /><font size="1"><b>Note:</b> Limited to items in stock as of <?php echo date('m/d/Y h:ia');?></font><?php }
		?>
		<p>
			<input type="button" value="Print &gt;" onclick="window.print()" class="btn" />
			<input type="button" value="Close &gt;" onclick="window.close()" class="btn" />
		</p>

		<table border="0" cellspacing="0" cellpadding="2" width="100%">
			<?php
			while (list($platformID, $items) = each($allitems))
			{
				$numeach = ceil(count($items)/$cols);

				?>
				<tr><td colspan="<?php echo $cols;?>"><hr width="100%" size="-1" color="#000000" noshade="noshade" /></td></tr>
				<tr><td align="center" colspan="<?php echo $cols;?>"><b><?php echo $platforms[$platformID];?></b></td></tr>
				<tr>
					<?php
					for ($i=0; $i<$cols; $i++)
					{
						$start = ($i*$numeach);
						$end = ((($i+1)*$numeach)-1);
						if ($end > (count($items)-1)) { $end = (count($items)-1); }

						?><td width="<?php echo $colwidth;?>%" valign="top"><table border="0" cellspacing="0" cellpadding="0" width="100%"><?php
						for ($j=$start; $j<=$end; $j++)
						{
							$thisitem = $items[$j];
							$title = substr($thisitem['itm_title'], 0, $max_title_length) . (strlen($thisitem['itm_title'])>$max_title_length ? '' : '');
							$newprc = $thisitem['prc_new'];
							$usedprc = $thisitem['prc_used'];
							$newqty = @$thisitem['qty_new'];
							$usedqty = @$thisitem['qty_used'];
							?><tr><td><?php echo $title;?></td><td align="right"><?php echo $usedprc;?></td><td>&nbsp;</td><?php
							if ($instock) { ?><td align="right"><?php echo $newqty;?></td><td>/</td><td align="right"><?php echo $usedqty;?></td><?php }
							?></tr><?php
						}
						?></table></td><?php
					}
					?>
				</tr>
				<?php
			}
			?>
			<tr><td colspan="<?php echo $cols;?>"><hr width="100%" size="-1" color="#000000" noshade="noshade" /></td></tr>
		</table>
		<?php

		//$pg->addOnload('window.print()');
	}
}

$pg->foot();
?>
