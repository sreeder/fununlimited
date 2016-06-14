<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

if (@$_SESSION['requestID']) { header('Location: /admin/utilities/invmove_picklist.php'); }

$pg = new admin_page();
$pg->setTitle('Store to Store Inventory Movement - Select Stores');
$pg->head('Store to Store Inventory Movement - Select Stores');

$error = new error('Store to Store Inventory Movement - Select Stores');
$mov = new inventory_movement($pg);

// output to/from store selection
$sto = new stores($pg);
$sto->setStores();
$stores = $sto->getStores();

?>
<script type="text/javascript">
	function verify(frm)
	{
		if (!frm.elements['stores[from]'].selectedIndex) { alert('Please select a from store.'); frm.elements['stores[from]'].focus(); return false; }
		else if (!frm.elements['stores[to]'].selectedIndex) { alert('Please select a to store.'); frm.elements['stores[to]'].focus(); return false; }
		else if (frm.elements['stores[from]'].options[frm.elements['stores[from]'].selectedIndex].value == frm.elements['stores[to]'].options[frm.elements['stores[to]'].selectedIndex].value) { alert('Please select different stores.'); frm.elements['stores[from]'].focus(); return false; }
		else if (!frm.elements['qtys[from]'].value.length) { alert('Please enter a minimum quantity.'); frm.elements['qtys[from]'].focus(); return false; }
		else if (!frm.elements['qtys[to]'].value.length) { alert('Please enter a maximum quantity.'); frm.elements['qtys[to]'].focus(); return false; }
		else { return true; }
	}
</script>

Please select the stores you would like to transfer the inventory <b>From</b> and <b>To</b>,<br />
as well as the minimum/maximum on hand quantities for each:
<p />
<?=$pg->outlineTableHead();?>
	<form method="post" action="/admin/utilities/invmoveUpdate.php" onsubmit="return verify(this)">
	<input type="hidden" name="act" value="setstores" />
	<tr>
		<td bgcolor="<?=$pg->color('table-label');?>"><b>From:</b></td>
		<td bgcolor="<?=$pg->color('table-cell');?>">
			<select name="stores[from]" size="1"><option value="">- Select Store -</option><?php
				while (list($a,$arr) = each($stores))
				{
					if ($arr['sto_storeID'] == $_SESSION['storeID'])
					{
						if ($arr['sto_storeID'] == @$_SESSION['request_info']['stores']['from']) { $s = ' selected="selected"'; } else { $s = ''; }
						?><option value="<?=$arr['sto_storeID'];?>"<?=$s;?>><?=$arr['sto_name'];?></option><?php
					}
				}
				reset($stores);
			?></select>
		</td>
		<td bgcolor="<?=$pg->color('table-cell');?>" align="right">
			<b>Min Qty:</b> &gt;= <input type="text" name="qtys[from]" size="3" onkeypress="return onlynumbers(this.value,event,true)" value="<?=(isset($_SESSION['request_info']['qtys']['from'])?$_SESSION['request_info']['qtys']['from']:2);?>" />
		</td>
	</tr>
	<tr>
		<td bgcolor="<?=$pg->color('table-label');?>"><b>To:</b></td>
		<td bgcolor="<?=$pg->color('table-cell');?>">
			<select name="stores[to]" size="1"><option value="">- Select Store -</option><?php
				while (list($a,$arr) = each($stores))
				{
					if ($arr['sto_storeID'] != $_SESSION['storeID'])
					{
						if ($arr['sto_storeID'] == @$_SESSION['request_info']['stores']['to']) { $s = ' selected="selected"'; } else { $s = ''; }
						?><option value="<?=$arr['sto_storeID'];?>"<?=$s;?>><?=$arr['sto_name'];?></option><?php
					}
				}
				reset($stores);
			?></select>
		</td>
		<td bgcolor="<?=$pg->color('table-cell');?>" align="right">
			<b>Max Qty:</b> &lt;= <input type="text" name="qtys[to]" size="3" onkeypress="return onlynumbers(this.value,event,true)" value="<?=(isset($_SESSION['request_info']['qtys']['to'])?$_SESSION['request_info']['qtys']['to']:0);?>" />
		</td>
	</tr>
<?=$pg->outlineTableFoot();?>
<p />
<input type="reset" value="&lt; Reset Selections" class="btn" />
<input type="submit" value="Select Stores &gt;" class="btn" />
</form>
<p />
<font size="1"><b>Note:</b> The next page may take a minute or two to load - please be patient!</font>
<?php

$pg->foot();
?>