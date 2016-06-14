<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Price Changes List');

$pg = new admin_page();
$pg->setTitle('Price Changes List');
$pg->head('Price Changes List');

// output the criteria form
$prc = new prices($pg);
list($min,$max) = $prc->getPriceChangeRange();

?>
Please select either new/used/both and a date range to view changed prices.
<p />
<b>Valid date range:</b> <?="$min - $max"?>
<p />
<script type="text/javascript">
	function verify(frm)
	{
		if (frm.fromdate.value == '') { alert('Please select a from date'); fromcal.popup(); return false; }
		else { return true; }
	}
</script>
<script language="javascript" src="/scripts/calendar.js"></script>
<?php

if (@$_GET['newused'])
{
	$nu = $_GET['newused'];
	$c1 = ($nu==ITEM_NEW?' checked="checked"':'');
	$c2 = ($nu==ITEM_USED?' checked="checked"':'');
	$c3 = ($nu==BOTH?' checked="checked"':'');
}
else
{
	$c1 = '';
	$c2 = ' checked="checked"';
	$c3 = '';
}

$pg->outlineTableHead();
?>

<form method="get" action="price_changes_print.php" name="pcfrm" onsubmit="return verify(this)">
<input type="hidden" name="act" value="print">
<tr>
	<td bgcolor="<?=$pg->color('table-label');?>"><b>Platform:</b></td>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		<select name="platformID" size="1">
			<option value="0">All</option>
			<?php
			$pla = new platforms($this->pg,0);
			$pla->set_item('platforms');
			while (list($a,$arr) = each($pla->values))
			{
				?><option value="<?=$arr[0];?>"><?=$arr[1];?></option><?php
			}
			?>
		</select>
	</td>
</tr>
<tr>
	<td bgcolor="<?=$pg->color('table-label');?>"><b>New/Used:</b></td>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		<input type="radio" name="newused" value="<?=ITEM_NEW;?>" id="n" class="nb"<?=$c1;?> /> <label for="n">New</label>
		<input type="radio" name="newused" value="<?=ITEM_USED;?>" id="u" class="nb"<?=$c2;?> /> <label for="u">Used</label>
		<input type="radio" name="newused" value="<?=BOTH;?>" id="b" class="nb"<?=$c3;?> /> <label for="b">Both</label>
	</td>
</tr>
<tr>
	<td bgcolor="<?=$pg->color('table-label');?>"><b>From Date:</b></td>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		<input type="text" size="12" name="fromdate" readonly="readonly" onclick="fromcal.popup()">
		<a href="javascript:fromcal.popup();"><img src="/images/calendar/cal.gif" width="16" height="16" border="0" align="bottom" alt="Click to select a date" /></a>
		<input type="button" value="Today" onclick="document.getElementById('pcfrm').elements['fromdate'].value='<?=date('m/d/Y');?>'" />
		<input type="button" value="Min" style="width:40px" onclick="document.getElementById('pcfrm').elements['fromdate'].value='<?=$min;?>'" />
		<input type="button" value="Clear Date" onclick="document.getElementById('pcfrm').elements['fromdate'].value=''" />

		<script type="text/javascript">var fromcal = new calendar2(document.getElementById('pcfrm').elements['fromdate']);</script>
	</td>
</tr>
<tr>
	<td bgcolor="<?=$pg->color('table-label');?>"><b>To Date:</b></td>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		<input type="text" size="12" name="todate" readonly="readonly" onclick="tocal.popup()">
		<a href="javascript:tocal.popup();"><img src="/images/calendar/cal.gif" width="16" height="16" border="0" align="bottom" alt="Click to select a date" /></a>
		<input type="button" value="Today" onclick="document.getElementById('pcfrm').elements['todate'].value='<?=date('m/d/Y');?>'" />
		<input type="button" value="Max" style="width:40px" onclick="document.getElementById('pcfrm').elements['todate'].value='<?=$max;?>'" />
		<input type="button" value="Clear Date" onclick="document.getElementById('pcfrm').elements['todate'].value=''" />

		<script type="text/javascript">var tocal = new calendar2(document.getElementById('pcfrm').elements['todate']);</script>
	</td>
</tr>
<tr>
	<td bgcolor="<?=$pg->color('table-label');?>"><b>Limit:</b></td>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		<input type="checkbox" name="limit" id="limit" value="<?php echo YES;?>" checked="checked" class="nb" />
		<label for="limit">Only show the latest price change for each item</label>
	</td>
</tr>
<?php
$pg->outlineTableFoot();

?>
<p />
<input type="submit" value="View Price Changes &gt;" class="btn">
</form>
<p />
<font size="1"><b>Note:</b> If the <b>To Date</b> is left blank, today's date will be used</font>
<?php

$pg->foot();
?>
