<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Best Selling Items');

$pg = new admin_page();
$pg->setTitle('Best Selling Items');
$pg->head('Best Selling Items');

// output the criteria form
$inv = new invoice($pg);
list($min,$max) = $inv->getInvoiceRange();

// get the platforms
$pla = new platforms($pg,0);
$pla->set_item('platforms');

// output the criteria form
?>
Please select a platform and a date range to view the best selling items.
<p />
<b>Valid date range:</b> <?="$min - $max"?>
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

<?=$pg->outlineTableHead();?>
<form method="get" action="best_sellers_print.php" name="bsfrm" onsubmit="return verify(this)">
<input type="hidden" name="act" value="print">
<tr>
	<td bgcolor="<?=$pg->color('table-label');?>"><b>Platform:</b></td>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		<select name="platformID" size="1">
			<option value="0">All</option>
			<?php
			while (list($a,$arr) = each($pla->values))
			{
				?><option value="<?=$arr[0];?>"><?=$arr[1];?></option><?php
			}
			?>
		</select>
	</td>
</tr>
<tr>
	<td bgcolor="<?=$pg->color('table-label');?>"><b>From Date:</b></td>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		<input type="text" size="12" name="fromdate" value="<?=date('m/d/Y',mktime(0,0,0,date('m'),1,date('Y')));?>" readonly="readonly" onclick="fromcal.popup()">
		<a href="javascript:fromcal.popup();"><img src="/images/calendar/cal.gif" width="16" height="16" border="0" align="bottom" alt="Click to select a date" /></a>
		<input type="button" value="Today" onclick="document.getElementById('bsfrm').elements['fromdate'].value='<?=date('m/d/Y');?>'" />
		<input type="button" value="Min" style="width:40px" onclick="document.getElementById('bsfrm').elements['fromdate'].value='<?=$min;?>'" />
		<input type="button" value="Clear Date" onclick="document.getElementById('bsfrm').elements['fromdate'].value=''" />
		<script type="text/javascript">var fromcal = new calendar2(document.getElementById('bsfrm').elements['fromdate']);</script>
	</td>
</tr>
<tr>
	<td bgcolor="<?=$pg->color('table-label');?>"><b>To Date:</b></td>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		<input type="text" size="12" name="todate" value="<?=date('m/d/Y');?>" readonly="readonly" onclick="tocal.popup()">
		<a href="javascript:tocal.popup();"><img src="/images/calendar/cal.gif" width="16" height="16" border="0" align="bottom" alt="Click to select a date" /></a>
		<input type="button" value="Today" onclick="document.getElementById('bsfrm').elements['todate'].value='<?=date('m/d/Y');?>'" />
		<input type="button" value="Max" style="width:40px" onclick="document.getElementById('bsfrm').elements['todate'].value='<?=$max;?>'" />
		<input type="button" value="Clear Date" onclick="document.getElementById('bsfrm').elements['todate'].value=''" />
		<script type="text/javascript">var tocal = new calendar2(document.getElementById('bsfrm').elements['todate']);</script>
	</td>
</tr>
<tr>
	<td bgcolor="<?=$pg->color('table-label');?>"><b>View Top:</b></td>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		<select name="num" size="1">
			<?php
			for ($i=10; $i<=200; $i+=10)
			{
				?><option value="<?=$i;?>"<?=($i==100 ? ' selected="selected"' : '');?>><?=$i;?></option><?php
			}
			?>
			<option value="1000">1,000</option>
		</select>
		items
	</td>
</tr>
<tr>
	<td bgcolor="<?=$pg->color('table-label');?>"><b>Calculate On:</b></td>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		<input type="radio" name="order" value="qty" id="vq" class="nb" checked="checked" /> <label for="vq">Units Sold</label>
		<input type="radio" name="order" value="price" id="vp" class="nb" /> <label for="vp">Total Sales</label>
	</td>
</tr>
<tr>
	<td bgcolor="<?=$pg->color('table-label');?>"><b>Show:</b></td>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		<input type="radio" name="stock" value="in" id="si" class="nb" /> <label for="si">In Stock</label>
		<input type="radio" name="stock" value="out" id="so" class="nb" /> <label for="so">Out of Stock</label>
		<input type="radio" name="stock" value="either" id="se" class="nb" checked="checked" /> <label for="se">Either</label>
	</td>
</tr>
<?=$pg->outlineTableFoot();?>

<p />

<input type="submit" value="View Best Sellers &gt;" class="btn">
</form>
<?php
$pg->foot();
?>
