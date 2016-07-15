<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$requestID = (isset($_GET['requestID'])?$_GET['requestID']:@$_POST['requestID']);
$current_requestID = @$_SESSION['requestID'];

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Store to Store Inventory Movement - Print List');
$pg->head('Store to Store Inventory Movement - Print List');

$error = new error('Store to Store Inventory Movement - Print List');
$mov = new inventory_movement($pg);

// output list of items from the given request
if ($requestID == 'incomplete')
{
	$getrequestID = $current_requestID;
	$tmpitems = $_SESSION['request_info']['items'];
}
else { $getrequestID = $requestID; }
$mov->setRequestID($getrequestID);
$info = $mov->getRequestInfo();

if (!count($info)) { echo "Invalid requestID: $getrequestID"; }
else
{
	$status = $info['req_status'];
	if ($status == MOV_INCOMPLETE) { $_SESSION['request_info']['items'] = $tmpitems; }
	$items = $mov->getRequestItems();

	$max_title_length = 38;
	$cols = 2;
	$width = floor(100/$cols);

	if ($status == MOV_INCOMPLETE) { $fields = array('itm_itemID','pla_platformID','pla_name','itm_title','newused','price'); }
	else {  $fields = array('rqi_itemID','rqi_platformID','rqi_platformname','rqi_title','rqi_newused','rqi_price'); }

	// break items out into platforms
	$show = array(); // format: $show[platformID] = array(platform_name,items)
	while (list($a,$arr) = each($items))
	{
		if (!isset($show[$arr[$fields[1]]]))
		{
			$show[$arr[$fields[1]]] = array($arr[$fields[2]],array());
		}

		if ($act == "printitems" || ($act == "printitemsnr" && !$arr['rqi_received']))
		{
			$show[$arr[$fields[1]]][1][] = $arr; // add to the items element
			$item_platformIDs[] = $arr[$fields[1]];
		}
	}
	reset($items);

	// remove platforms that don't contain items
	while (list($platformID,list($platform_name,$items)) = each($show))
	{
		if (!count($items)) { unset($show[$platformID]); }
	}
	reset($show);
	$platformIDs = array_keys($show);

	// if completed, this time will be used for completed date/time display
	$usetime = ($info['req_timecompleted']?$info['req_timecompleted']:$info['req_timeadded']);

	?>
	<style type="text/css">
		.p { font-size:8;line-height:11px; }
		.c { font-size:8;line-height:11px; font-family:Courier New; }
	</style>

	<table border="0" cellspacing="0" cellpadding="5">
		<tr>
			<td valign="top">
				<font size="2">
					<b><u>Stores:</u></b><br />
					<?=$mov->showStores(NO,NO);?>
				</font>
			</td>
			<td>&nbsp; &nbsp; &nbsp;</td>
			<td valign="top">
				<font size="2">
					<b><u>Request Status:</u></b><br />
					<?=$mov->getStatusWords($status,$info['req_tostoreID']);?>
					<?=($status==MOV_COMPLETE?'<br />Completed on '.date('m/d/Y',$usetime).' at '.date('h:ia',$usetime):'');?>
				</font>
			</td>
		</tr>
	</table>
	<?php

	if ($status == MOV_INCOMPLETE)
	{
		?>Check the items you would like to send as well as whether or not they have a box, instructions, and their condition.<p /><?php
	}
	?>

	<font size="1"><b>Key:</b> B=Box, NB=No Box, I=Instructions, NI=No Instructions, G=Good, F=Fair, P=Poor</font>
	<p />
	<input type="button" value="Print &gt;" onclick="window.print()" class="btn" />
	<input type="button" value="Close Window &gt;" onclick="window.close()" class="btn" />
	<?php

	if ($act == "printitemsnr") { ?><p /><font size="1"><b>Note:</b> The following items were NOT received by the <b>To</b> store!</font><?php }
	?>
	<br />
	<table border="0" cellspacing="0" cellpadding="1">
		<?php
		$total_count = 0;
		$total_price = 0;

		while (list($a,$platformID) = each($platformIDs))
		{
			list($platform_name,$items) = $show[$platformID];
			$percol = ceil(count($items)/$cols);
			$count = 0;
			$total = 0;

			?>
			<tr><td colspan="<?=($cols*5);?>"><font size="2"><b><i>� <u><?=$platform_name;?></u></i></b></font><br />&nbsp;</td></tr>
			<tr>
				<?php
				for ($i=0; $i<$cols; $i++)
				{
					$start = ($i*$percol);
					$end = (($i+1)*$percol);
					if (count($items) > 3 && $end > count($items)) { $end = count($items); }

					$titles = array();
					$newused = array();
					$configs = array();
					$prices = array();

					for ($j=$start; $j<$end; $j++)
					{
						$arr = @$items[$j];

						if (is_array($arr))
						{
							$config = array();
							if ($status == MOV_INCOMPLETE)
							{
								$config = ' B NB / I NI / G F P ';
							}
							else
							{
								if ($arr['rqi_box']) { $config[] = '<u>B</u> NB'; } else { $config[] = 'B <u>NB</u>'; }
								if ($arr['rqi_instructions']) { $config[] = '<u>I</u> NI'; } else { $config[] = 'I <u>NI</u>'; }
								$config[] = ($arr['rqi_condition']==GOOD?'<u>G</u> F P':($arr['rqi_condition']==FAIR?'G <u>F</u> P':'G F <u>P</u>'));
								$config = implode(' / ',$config);
							}

							$titles[] = '() '.substr(stripslashes($arr[$fields[3]]),0,$max_title_length);
							$newused[] = ($arr[$fields[4]]==ITEM_NEW?'New':'Used');
							$configs[] = "[$config]";
							$prices[] = $arr[$fields[5]];
							$count++;
							$total += $arr[$fields[5]];
						}
						else
						{
							$titles[] = '&nbsp;';
							$newused[] = '&nbsp;';
							$configs[] = '&nbsp;';
							$prices[] = '&nbsp;';
						}
					}

					?>
					<td class="p" valign="top"><nobr><?=implode('<br />',$titles);?></nobr></td>
					<td class="p" align="right" valign="top"><?=implode('<br />',$prices);?></td>
					<td class="p" valign="top"><?=implode('<br />',$newused);?></td>
					<td class="c" valign="top"><?=implode('<br />',$configs);?></td>
					<td>&nbsp;</td>
					<?php
				}
				?>
			</tr>
			<tr><td class="p" colspan="<?=($cols*5);?>">&nbsp; &nbsp; &nbsp; <font size="1"><b><?=$platform_name;?> Total:</b> <?=number_format($count,0);?> item<?=($count!=1?'s':'');?> / $<?=number_format($total,2);?></font></td></tr>
			<tr><td colspan="<?=($cols*5);?>">&nbsp;</td></tr>
			<?php

			$total_count += $count;
			$total_price += $total;
		}
		?>
	</table>
	<p />
	<b>Grand Total:</b> <?=number_format($total_count,0);?> item<?=($total_count!=1?'s':'');?> / $<?=number_format($total_price,2);?>
	<?php
}

if (!$current_requestID) { $mov->clearSession(); }

//$pg->addOnload('window.print()');
$pg->foot();
?>