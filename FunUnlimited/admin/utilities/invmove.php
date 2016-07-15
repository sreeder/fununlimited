<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$previous = (isset($_GET['previous'])?$_GET['previous']:@$_POST['previous']);

if (isset($_GET['storeID'])) { $_SESSION['storeID'] = $_GET['storeID']; }
if (@$_GET['reset']) { unset($_SESSION['requestID']); unset($_SESSION['request_info']); }
if (@$_SESSION['requestID']) { header('Location: /admin/utilities/invmove_picklist.php'); }

$pg = new admin_page();
$pg->setTitle('Store to Store Inventory Movement');
$pg->head('Store to Store Inventory Movement');

$error = new error('Store to Store Inventory Movement');
$mov = new inventory_movement($pg);

// output list of requests and nav buttons
if (strlen(@$_GET['status'])) { $pg->status($_GET['status']); }
$mov->setRequests($previous);
$requests = $mov->getRequests();

?>
<script type="text/javascript">
	function reloadPage() { document.location = document.location; }
	setTimeout('reloadPage()',60000); // refresh every 60 seconds
</script>

<b>View:</b>
<?=($previous?'<a href="/admin/utilities/invmove.php?previous='.NO.'">':'<u>');?>Active<?=($previous?'</a>':'</u>');?>
 |
<?=(!$previous?'<a href="/admin/utilities/invmove.php?previous='.YES.'">':'<u>');?>Previous<?=(!$previous?'</a>':'</u>');?>
<p />

<input type="button" value="Create New Request &gt;" onclick="document.location='invmove_pickstores.php'" class="btn" />
<p />
<?php
$word = ($previous?'previous':'active');

if (!count($requests)) { ?>No <?=$word;?> inventory requests found<?php }
else
{
	?>
	<script type="text/javascript">
		function go(where,reqID)
		{
			if (where == 'printitems' || where == 'printitemsnr')
			{
				open_window('/admin/utilities/invmove_print.php?act='+where+'&requestID='+reqID,'printitems',725,500,'YES',true);
			}
			else if (where != 'delete' || (where == 'delete' && confirm('Are you sure you want to delete this inventory request?')))
			{
				var frm = document.reqs;
				frm.act.value = where;
				frm.requestID.value = reqID;
				frm.submit();
			}
		}
	</script>

	<form method="post" action="/admin/utilities/invmoveUpdate.php" name="reqs">
	<input type="hidden" name="act" value="">
	<input type="hidden" name="requestID" value="">
	<?=$pg->outlineTableHead();?>
	<tr bgcolor="<?=$pg->color('table-head');?>">
		<td><b>Stores</b></td>
		<td><b>Details</b></td>
		<td><b>Status</b></td>
		<td><b>Functions</b></td>
	</tr>

	<?php
	while (list($a,$arr) = each($requests))
	{
		$bg = (($a%2)?$pg->color('table-cell'):$pg->color('table-cell2'));

		$totitems = count($arr['items']);
		$totprice = 0;
		while (list($b,$iarr) = each($arr['items'])) { $totprice += $iarr['rqi_price']; }

		$fbo=''; $fbc=''; $tbo=''; $tbc='';
		if ($_SESSION['storeID'] == $arr['req_fromstoreID']) { $fbo = '<font color="red"><b>'; $fbc = '</b></font>'; }
		if ($_SESSION['storeID'] == $arr['req_tostoreID']) { $tbo = '<font color="red"><b>'; $tbc = '</b></font>'; }

		$arr['req_status'] = $mov->fixStatus($arr['req_status'],$arr['req_tostoreID']);

		?>
		<tr bgcolor="<?=$bg;?>">
			<td valign="top">

					<?=$fbo;?>From: <?=$arr['sto_fromname'];?><?=$fbc;?><br />
					<?=$tbo;?>To: <?=$arr['sto_toname'];?><?=$tbc;?>

			</td>
			<td valign="top">
				Date: <?=date('m/d/y',$arr['req_timeadded']);?><br />
				Time: <?=date('h:ia',$arr['req_timeadded']);?><br />
				Total Items: <?=number_format($totitems,0);?><br />
				Total Price: $<?=number_format($totprice,2);?>
			</td>
			<td valign="top"><?=$mov->getStatusWords($arr['req_status'],$arr['req_tostoreID']);?></td>
			<td>
				<?php
				if ($arr['req_status'] == MOV_INCOMPLETE)
				{
					?>
					<input type="button" value="Delete Request &gt;" onclick="go('delete',<?=$arr['req_requestID'];?>)" class="btn">
					<?php
				}
				elseif ($arr['req_status'] == MOV_REQUEST_SENT.MOV_FROM_STORE)
				{
					?>
					<input type="button" value="Print Item List &gt;" onclick="go('printitems',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<img src="/images/blank.gif" width="1" height="2"><br />
					<input type="button" value="Cancel Request &gt;" onclick="go('delete',<?=$arr['req_requestID'];?>)" class="btn">
					<?php
				}
				elseif ($arr['req_status'] == MOV_REQUEST_SENT.MOV_TO_STORE)
				{
					?>
					<input type="button" value="Print Item List &gt;" onclick="go('printitems',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<img src="/images/blank.gif" width="1" height="2"><br />
					<input type="button" value="Select Items to Receive &gt;" onclick="go('selrequested',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<img src="/images/blank.gif" width="1" height="2"><br />
					<input type="button" value="Deny Request &gt;" onclick="if (confirm('Are you sure you want to deny this inventory request?')) { go('deny',<?=$arr['req_requestID'];?>); }" class="btn">
					<?php
				}
				elseif ($arr['req_status'] == MOV_ITEMS_SELECTED.MOV_FROM_STORE)
				{
					?>
					<input type="button" value="Print Item List &gt;" onclick="go('printitems',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<img src="/images/blank.gif" width="1" height="2"><br />
					<input type="button" value="Select Shipped Items &gt;" onclick="go('selshipped',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<img src="/images/blank.gif" width="1" height="2"><br />
					<input type="button" value="Cancel Request &gt;" onclick="go('delete',<?=$arr['req_requestID'];?>)" class="btn">
					<?php
				}
				elseif ($arr['req_status'] == MOV_ITEMS_SELECTED.MOV_TO_STORE)
				{
					?>
					<input type="button" value="Print Item List &gt;" onclick="go('printitems',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<?php
				}
				elseif ($arr['req_status'] == MOV_DENIED)
				{
					?>
					<input type="button" value="Print Item List &gt;" onclick="go('printitems',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<img src="/images/blank.gif" width="1" height="2"><br />
					<input type="button" value="Resend Request &gt;" onclick="if (confirm('Are you sure you want to resend this inventory request?')) { go('resend',<?=$arr['req_requestID'];?>); }" class="btn"><br />
					<img src="/images/blank.gif" width="1" height="2"><br />
					<input type="button" value="Delete Request &gt;" onclick="go('delete',<?=$arr['req_requestID'];?>)" class="btn">
					<?php
				}
				elseif ($arr['req_status'] == MOV_IN_TRANSIT && $_SESSION['storeID'] == $arr['req_fromstoreID'])
				{
					?>
					<input type="button" value="Print Item List &gt;" onclick="go('printitems',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<?php
				}
				elseif ($arr['req_status'] == MOV_IN_TRANSIT && $_SESSION['storeID'] == $arr['req_tostoreID'])
				{
					?>
					<input type="button" value="Print Item List &gt;" onclick="go('printitems',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<img src="/images/blank.gif" width="1" height="2"><br />
					<input type="button" value="Select Received Items &gt;" onclick="go('selreceived',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<?php
				}
				elseif ($arr['req_status'] == MOV_ITEMS_RECEIVED.MOV_FROM_STORE)
				{
					$allreceived = $mov->wereAllReceived($arr['req_requestID']);
					if (!$allreceived)
					{
						?>
						<font color="red"><b>Not all items were received!</b></font><br />
						<img src="/images/blank.gif" width="1" height="2"><br />
						<input type="button" value="Print Non-Received Item List &gt;" onclick="go('printitemsnr',<?=$arr['req_requestID'];?>)" class="btn"><br />
						<img src="/images/blank.gif" width="1" height="2"><br />
						<?php
					}

					?>
					<input type="button" value="Print Item List &gt;" onclick="go('printitems',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<img src="/images/blank.gif" width="1" height="2"><br />
					<input type="button" value="Complete Request &gt;" onclick="go('complete',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<?php
				}
				elseif ($arr['req_status'] == MOV_ITEMS_RECEIVED.MOV_TO_STORE)
				{
					?>
					<input type="button" value="Print Item List &gt;" onclick="go('printitems',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<?php
				}
				elseif ($arr['req_status'] == MOV_COMPLETE)
				{
					?>
					<input type="button" value="Print Item List &gt;" onclick="go('printitems',<?=$arr['req_requestID'];?>)" class="btn"><br />
					<?php
				}
				?>
			</td>
		</tr>
		<?php
	}
	?>

	<?=$pg->outlineTableFoot();?>
	</form>
	<?php
}
?>
<p />
<font size="1"><b>Note:</b> This page automatically refreshes every 60 seconds.</font>
<?php

$pg->foot();
?>