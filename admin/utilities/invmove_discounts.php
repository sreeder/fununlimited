<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

if (@$_SESSION['requestID']) { header('Location: /admin/utilities/invmove_picklist.php'); }
if (!is_array(@$_SESSION['request_info']['stores'])) { header('Location: /admin/utilities/invmove_stores.php'); }

$pg = new admin_page();
$pg->setTitle('Store to Store Inventory Movement - Enter Discounts');
$pg->head('Store to Store Inventory Movement - Enter Discounts');

$error = new error('Store to Store Inventory Movement - Enter Discounts');
$mov = new inventory_movement($pg);

if (strlen(@$_GET['error'])) { $pg->error($_GET['error']); }

// output discounts form
$mov->showStores(YES);
$discounts = $mov->getDiscounts();
$selplatforms = $mov->getSelectedPlatforms();
if ($selplatforms == -1) { $selplatforms = array(); $firstload = YES; } else { $firstload = NO; }

// get the items that match the store information
$mov->setCriteriaItems();
$items = $mov->getCriteriaItems();
$_SESSION['request_info']['items'] = $items;

if (!count($items))
{
	?>
	No items matched your criteria. Please click <b>Changes Stores</b> and try again.
	<?php
}
else
{
	$platforms = array(0=>'titles');
	$platforms += $mov->getItemPlatforms($items); // array_merge() reindexes the keys since they're numeric, so use += here

	?>
	<?=number_format(count($items),0);?> item<?=(count($items)!=1?'s':'');?> matched your criteria.
	<p />
	Select the platforms to include, enter the new/used percents to sell the items to the store at,<br />
	and enter the discounts to be given for each of the following values:
	<p />
	<?php

	// format: $display['field'] = array('title','default_value')
	$display = array(
		'newpercent'=>array('New Percent',80),
		'usedpercent'=>array('Used Percent',70),
		'nobox'=>array('No Box',2),
		'noinstructions'=>array('No Instructions',0),
		'condfair'=>array('Fair Condition',2),
		'condpoor'=>array('Poor Condition',4)
	);
	$bg = $pg->color('table-cell2');

	?>
	<script type="text/javascript">
		function doCheckAll(frm,chk)
		{
			for (var i=0; i<frm.elements.length; i++)
			{
				var obj = frm.elements[i];
				if (obj.type == 'checkbox') { obj.checked = chk; }
			}
		}

		function verify(frm)
		{
			for (var i=0; i<frm.elements.length; i++)
			{
				var obj = frm.elements[i];
				if (obj.type == 'text' && !obj.value.length)
				{
					if (obj.name.indexOf('percent') > -1) { obj.value = 70; }
					else { obj.value = format_price(0); }
				}
			}

			return true;
		}
	</script>

	<font size="1"><b>Note:</b> Any percents left blank will be set to 70%, and discounts to $0.00</font>
	<p />
	<?php
	$pg->outlineTableHead();
	?>
	<form method="post" action="invmoveUpdate.php" onsubmit="return verify(this)">
	<input type="hidden" name="act" value="setdiscounts" />
	<?php
	while (list($platformID,$name) = each($platforms))
	{
		if (!$platformID)
		{
			?>
			<tr bgcolor="<?=$pg->color('table-head');?>">
				<td><input type="checkbox" onclick="doCheckAll(this.form,this.checked)" style="border-width:0px" title="Check/uncheck all platforms"<?=($firstload?' checked="checked"':'');?> /></td>
				<td><b>Platform</b></td>
				<?php
				while (list($field,$arr) = each($display)) { ?><td align="center" width="80"><b><?=$arr[0];?></b></td><?php }
				reset($display);
				?>
			</tr>
			<?php
		}
		else
		{
			$pchecked = ($firstload||in_array($platformID,$selplatforms)?' checked="checked"':'');

			?>
			<tr>
				<td bgcolor="<?=$pg->color('table-cell');?>"><input type="checkbox" name="selplatforms[]" value="<?=$platformID;?>" style="border-width:0px"<?=$pchecked;?> /></td>
				<td bgcolor="<?=$pg->color('table-head');?>"><b><?=$name;?></b></td>
				<?php
				while (list($field,$arr) = each($display))
				{
					$value = @$discounts[$platformID][$field];
					if (!strlen($value)) { $value = $arr[1]; } // default

					if (substr($field,-7) == "percent")
					{
						?><td align="center" bgcolor="<?=$bg;?>"><input type="text" name="discounts[<?=$field;?>][<?=$platformID;?>]" size="3" value="<?=$value;?>" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right" />%</td><?php
					}
					else
					{
						?><td align="center" bgcolor="<?=$bg;?>">$<input type="text" name="discounts[<?=$field;?>][<?=$platformID;?>]" size="5" value="<?=sprintf('%0.2f',$value);?>" onkeypress="return onlynumbers(this.value,event,true)" onblur="this.value=format_price(this.value,false)" style="text-align:right" /></td><?php
					}
				}
				reset($display);
				?>
			</tr>
			<?php
		}
	}
	$pg->outlineTableFoot();
	?>
	<p />
	<input type="reset" value="&lt; Reset Discounts" class="btn" />
	<input type="submit" value="Set Discounts &gt;" class="btn" />
	</form>
	<p />
	<font size="1"><b>Note:</b> The next page may be <b>very</b> large and browser-intensive. Please give it a minute or two to load!</font>
	<?php
}

$pg->foot();
?>