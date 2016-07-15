<?php
include('../../include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$upc = @$_GET['upc'];
$platformID = @$_GET['platformID'];
$newused = @$_GET['newused'];

$pg = new admin_page();
$pg->setFull(NO);
$pg->head();

$error = new error('Whole Platform Quantities UPC Check');

if (strlen($upc))
{
	// check that the UPC doesn't exist in any other platforms
	$sql = "SELECT pla_platformID,pla_name,itm_itemID,itm_title FROM items,platforms WHERE itm_upc='".mysql_real_escape_string($upc)."' AND itm_platformID=pla_platformID";
	$result = mysql_query($sql,$db);
	if (mysql_errno()) { ?><script type="text/javascript">alert('MySQL error in begqty UPC check - tell Scott!\n\n<?=mysql_real_escape_string($sql);?>\n<?=mysql_real_escape_string(mysql_error());?>')</script><?php }
	$error->mysql(__FILE__,__LINE__);

	if (mysql_num_rows($result))
	{
		// it does exist in another platform - alert
		$row = mysql_fetch_assoc($result);
		$name = $row['pla_name'];
		$title = $row['itm_title'];
		?>
		<script type="text/javascript">
			parent.unlock_upc();
			<?php
			if ($row['pla_platformID'] == $platformID)
			{
				$itemID = $row['itm_itemID'];
				$nu = ($newused==ITEM_NEW?'new':'used');
				$sql = "UPDATE quantity SET qty_$nu=(qty_$nu+1) WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID=$itemID";
				mysql_query($sql,$db);
				$error->mysql(__FILE__,__LINE__);

				$sql = "SELECT qty_$nu FROM quantity WHERE qty_storeID={$_SESSION['storeID']} AND qty_itemID=$itemID";
				$result = mysql_query($sql,$db);
				$error->mysql(__FILE__,__LINE__);
				$row = mysql_fetch_assoc($result);
				$updqty = $row["qty_$nu"];

				?>
				alert('UPC number <?=$upc;?> exists in the current platform, but is on a different page.\n\nThe <?=($newused==ITEM_NEW?'new':'used');?> quantity has been increased to <?=$updqty;?> for item: <?=mysql_real_escape_string($title);?>');
				parent.lastval = '';
				<?php
			}
			else { ?>parent.upc_exists('<?=$upc;?>','<?=mysql_real_escape_string($title);?>','<?=mysql_real_escape_string($name);?>');<?php }
			/*else { ?>alert('UPC number <?=$upc;?> found in different platform<?=$name;?>)');<?php }*/
			?>
		</script>
		<?php
	}
	else
	{
		// it doesn't exist in another platform - unlock the UPC text box
		$_SESSION['begqty_newused'] = $newused;
		?>
		<script type="text/javascript">
			parent.ask_addnew('<?=$upc;?>');
		</script>
		<?php
	}
}

$pg->foot();
?>