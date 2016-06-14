<?php
/**
 * Utah Pawn file download - criteria
 * Created: 10/05/2012
 * Revised: 10/05/2012
 */

include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Utah Pawn File');

$pg = new admin_page();
$pg->setTitle('Utah Pawn File Download');
$pg->head('Utah Pawn File Download');

// output the criteria form
$inv = new invoice($pg);
list($min, $max) = $inv->getInvoiceRange();

// output the criteria form

?>
<p>
	Please select a date range to generate and download the Utah Pawn upload file.
</p>

<p>
	<b>Valid date range:</b> <?php echo "$min - $max";?>
</p>

<p style="color:red;font-weight:bold">
	DO NOT generate a file for more than a few months at a time! The file will be too big for the uploader!
</p>

<script type="text/javascript">
	function verify(frm)
	{
		if (frm.fromdate.value == '') { alert('Please select a from date'); fromcal.popup(); return false; }
		else if (frm.todate.value == '') { alert('Please select a to date'); tocal.popup(); return false; }
		else { return true; }
	}
</script>
<script type="text/javascript" src="/scripts/calendar.js"></script>

<form method="get" action="pawn_download_generate.php" name="pdfrm" id="pdfrm" onsubmit="return verify(this)">

<?php echo $pg->outlineTableHead();?>
<tr>
	<td bgcolor="<?php echo $pg->color('table-label');?>"><b>From Date:</b></td>
	<td bgcolor="<?php echo $pg->color('table-cell');?>">
                <input type="text" size="12" name="fromdate" value="<?php echo date('m/d/Y');?>" onclick="fromcal.popup()">
		<a href="javascript:fromcal.popup();"><img src="/images/calendar/cal.gif" width="16" height="16" border="0" align="bottom" alt="Click to select a date" /></a>
		<input type="button" value="Today" onclick="document.getElementById('pdfrm').elements['fromdate'].value='<?php echo date('m/d/Y');?>'" />
		<input type="button" value="Min" style="width:40px" onclick="document.getElementById('pdfrm').elements['fromdate'].value='<?php echo $min;?>'" />
		<input type="button" value="Clear Date" onclick="document.getElementById('pdfrm').elements['fromdate'].value=''" />
		<script type="text/javascript">var fromcal = new calendar2(document.getElementById('pdfrm').elements['fromdate']);</script>
	</td>
</tr>
<tr>
	<td bgcolor="<?php echo $pg->color('table-label');?>"><b>To Date:</b></td>
	<td bgcolor="<?php echo $pg->color('table-cell');?>">
		<input type="text" size="12" name="todate" value="<?php echo date('m/d/Y');?>" onclick="tocal.popup()">
		<a href="javascript:tocal.popup();"><img src="/images/calendar/cal.gif" width="16" height="16" border="0" align="bottom" alt="Click to select a date" /></a>
		<input type="button" value="Today" onclick="document.getElementById('pdfrm').elements['todate'].value='<?php echo date('m/d/Y');?>'" />
		<input type="button" value="Max" style="width:40px" onclick="document.getElementById('pdfrm').elements['todate'].value='<?php echo $max;?>'" />
		<input type="button" value="Clear Date" onclick="document.getElementById('pdfrm').elements['todate'].value=''" />
		<script type="text/javascript">var tocal = new calendar2(document.getElementById('pdfrm').elements['todate']);</script>
	</td>
</tr>
<?php echo $pg->outlineTableFoot();?>

<p>
	<input type="submit" value="Generate File &gt;" class="btn">
</p>

</form>
<?php

$pg->foot();

/* END OF FILE */
/* Location: ./admin/reports/pawn_download.php */
