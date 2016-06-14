<?php
include('../../include/include.inc');

$cl = new check_login();

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

$pg = new admin_page();
$pg->setTitle('Whole Platform Item Info');
$pg->head('Whole Platform Item Info');

$error = new error('Whole Platform Item Info');

// show the platform/info element selection form
$pla = new platforms($pg,0);
$pla->set_item('platforms');

// set/pull the element variables
include('platform_items_elements.php');
?>
Select a platform and item information elements you would like to display.
<p />

<script type="text/javascript">
	function verify(frm)
	{
		if (frm.platformID.selectedIndex == 0) { alert('You must select a platform.'); frm.platformID.focus(); return false; }
		else if (frm.elems.length == 0) { alert('You must select at least 1 item info element.'); return false; }
		else { selectAllOptions(frm.elems); return true; }
	}

	function add_preset(num)
	{
		var elems = eval('pre'+num);
		var objfrom = document.getElementById('not_used');
		var objto = document.getElementById('elems');
		var moveelems = new Array();

		for (var i=0; i<objfrom.options.length; i++)
		{
			if (in_array(objfrom.options[i].value,elems))
			{
				objfrom.selectedIndex = i;
				move(objfrom,objto,true);
				i--;
			}
		}
	}
</script>
<form method="post" action="/admin/setup_items/platform_itemsUpdate.php" id="crtlst" onsubmit="return verify(this)">
<input type="hidden" name="act" value="setcriteria" />
<input type="hidden" name="return" value="/admin/setup_items/platform_items_form.php" />
<b>Platform:</b> <select name="platformID" size="1" style="vertical-align:middle"><option value=""></option><?php
	while (list($a,$arr) = each($pla->values)) { ?><option value="<?=$arr[0];?>"><?=$arr[1];?></option><?php }
?></select>
<p />
<script language="javascript" src="/scripts/listbox.js"></script>

<table cellpadding="3" cellspacing="0" border="0">
	<tr>
		<td align="center"><b>Not Selected</b><br>
			<select name="not_used" id="not_used" multiple="multiple" size="<?=$element_count;?>" ondblClick="move(this.form.not_used,this.form.elements)" style="width:200px"><?php
				while (list($element,$arr) = each($elements))
				{
					if (!isset($arr[4]) || (is_array($arr[4]) && (!isset($arr[5]) || (isset($arr[5]) && $arr[5])))) { ?><option value="<?=$element;?>"><?=$arr[0];?></option><?php }
				}
				reset($elements);
			?></select>
		</td>
		<td valign="middle" align="center" class="eight">
			<input type="button" name="right_move" value="&gt;&gt;" onclick="move(this.form.not_used,this.form.elems,true);" class="btn" /><br>
			<input type="button" name="right_move_all" value="All &gt;&gt;" onclick="moveAll(this.form.not_used,this.form.elems,true);" class="btn" /><br>
			<input type="button" name="left_move" value="&lt;&lt;" onclick="move(this.form.elems,this.form.not_used,true);" class="btn" /><br>
			<input type="button" name="left_move_all" value="All &lt;&lt;" onclick="moveAll(this.form.elems,this.form.not_used,true);" class="btn" />
		</td>
		<td align="center"><b>Selected</b><br>
			<select name="elements[]" id="elems" multiple="multiple" size="<?=$element_count;?>" ondblClick="move(this.form.elements,this.form.not_used)" style="width:200px"></select>
		</td>
	</tr>
</table>
<p />
<input type="submit" value="Show Items &gt;" class="btn">
<p />
<?=$pg->outlineTableHead();?>
<tr><td align="center" bgcolor="<?=$pg->color('table-head');?>"><b>Presets</b></td></tr>
<tr>
	<td align="center" bgcolor="<?=$pg->color('table-cell');?>">
		Click a button to add elements to the selected list.
		<p />
		<?php
		while (list($a,$arr) = each($presets))
		{
			echo "\n";
			$name = $arr[0];
			$elems = $arr[1];
			$show = @$arr[2];
			?><script type="text/javascript">var pre<?=$a;?>=['<?=implode("','",$elems);?>'];</script><?php
			?><input type="button" value="<?=$name;?>" onclick="add_preset(<?=$a;?>)" class="btn" /> <?php
		}
		?>
		<input type="button" value="None" onclick="moveAll(document.getElementById('elems'),document.getElementById('not_used'),true)" class="btn" />
	</td>
</tr>
<?=$pg->outlineTableFoot();?>
</form>
<p />
<font size="1"><b>Note:</b> <b>Title</b> will always be included in the list. All selected elements may not be shown.</font>
<?php

$pg->foot();
?>