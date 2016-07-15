<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Age/Gender Reports');

$pg = new admin_page();
$pg->setTitle('Age/Gender Reports');
$pg->head('Age/Gender Reports');

// output the criteria form
$inv = new invoice($pg);
list($min, $max) = $inv->getInvoiceRange();

// get the platforms
$pla = new platforms($pg,0);
$pla->set_item('platforms');

// output the criteria form
?>
Please select a platform and a date range to view sales/trades by age/gender.
<p />
<b>Valid date range:</b> <?php echo "$min - $max"?>
<p />
<script type="text/javascript">
	function verify(frm)
	{
		if (frm.fromdate.value == '') { alert('Please select a from date'); fromcal.popup(); return false; }
		else if (frm.todate.value == '') { alert('Please select a to date'); tocal.popup(); return false; }
		else { return true; }
	}
</script>
<script language="javascript" src="/scripts/calendar.js"></script>

<?php echo $pg->outlineTableHead();?>
<form method="get" action="by_demo_print.php" name="bsfrm" onsubmit="return verify(this)">
<input type="hidden" name="act" value="print">
<tr>
	<td bgcolor="<?php echo $pg->color('table-label');?>"><b>Platform:</b></td>
	<td bgcolor="<?php echo $pg->color('table-cell');?>">
		<select name="platformID" size="1">
			<option value="0">All</option>
			<?php
			while (list($a,$arr) = each($pla->values))
			{
				?><option value="<?php echo $arr[0];?>"><?php echo $arr[1];?></option><?php
			}
			?>
		</select>
	</td>
</tr>
<tr>
	<td bgcolor="<?php echo $pg->color('table-label');?>"><b>From Date:</b></td>
	<td bgcolor="<?php echo $pg->color('table-cell');?>">
		<input type="text" size="12" name="fromdate" value="<?php echo date('m/d/Y',mktime(0,0,0,date('m'),1,date('Y')));?>" onclick="fromcal.popup()">
		<a href="javascript:fromcal.popup();"><img src="/images/calendar/cal.gif" width="16" height="16" border="0" align="bottom" alt="Click to select a date" /></a>
		<input type="button" value="Today" onclick="document.getElementById('bsfrm').elements['fromdate'].value='<?php echo date('m/d/Y');?>'" />
		<input type="button" value="Min" style="width:40px" onclick="document.getElementById('bsfrm').elements['fromdate'].value='<?php echo $min;?>'" />
		<input type="button" value="Clear Date" onclick="document.getElementById('bsfrm').elements['fromdate'].value=''" />
		<script type="text/javascript">var fromcal = new calendar2(document.getElementById('bsfrm').elements['fromdate']);</script>
	</td>
</tr>
<tr>
	<td bgcolor="<?php echo $pg->color('table-label');?>"><b>To Date:</b></td>
	<td bgcolor="<?php echo $pg->color('table-cell');?>">
		<input type="text" size="12" name="todate" value="<?php echo date('m/d/Y');?>" onclick="tocal.popup()">
		<a href="javascript:tocal.popup();"><img src="/images/calendar/cal.gif" width="16" height="16" border="0" align="bottom" alt="Click to select a date" /></a>
		<input type="button" value="Today" onclick="document.getElementById('bsfrm').elements['todate'].value='<?php echo date('m/d/Y');?>'" />
		<input type="button" value="Max" style="width:40px" onclick="document.getElementById('bsfrm').elements['todate'].value='<?php echo $max;?>'" />
		<input type="button" value="Clear Date" onclick="document.getElementById('bsfrm').elements['todate'].value=''" />
		<script type="text/javascript">var tocal = new calendar2(document.getElementById('bsfrm').elements['todate']);</script>
	</td>
</tr>
<tr>
	<td bgcolor="<?php echo $pg->color('table-label');?>"><b>View Type:</b></td>
	<td bgcolor="<?php echo $pg->color('table-cell');?>">
		<input type="radio" name="type" id="ts" value="<?php echo SALE;?>" checked="checked" class="nb" />
		<label for="ts">Sales</label>

		<input type="radio" name="type" id="tt" value="<?php echo TRADE;?>" class="nb" />
		<label for="tt">Trades</label>

		<input type="radio" name="type" id="tr" value="<?php echo RETURNS;?>" class="nb" />
		<label for="tr">Returns</label>
	</td>
</tr>
<?php echo $pg->outlineTableFoot();?>

<p>
	<input type="submit" value="View Report &gt;" class="btn">
</p>

</form>
<?php
$pg->foot();
?>
