<?php
include('../../include/include.inc');

$cl = new check_login(STORE);
$error = new error('Summary Report');

$pg = new admin_page();
$pg->setTitle('Summary Report');
$pg->head('Summary Report');

// output the criteria form
$inv = new invoice($pg);
list($min, $max) = $inv->getInvoiceRange();
unset($inv);

// get the platforms
$pla = new platforms($pg, 0);
$pla->set_item('platforms');

// get the employees
$empl = new employees($pg);
$empl->get_employees(BOTH);

// output the criteria form
$date_low = mktime(0, 0, 0, date('m'), 1, date('Y'));
$date_high = time();

// temporary
$date_low = strtotime('1/1/2014');
$date_high = strtotime($max);

?>
<p>
	Please select from the following criteria to view a summary report:
</p>

<p>
	<b>Valid date range:</b> <?php echo "$min - $max"?>
</p>

<script type="text/javascript">
	function verify(frm)
	{
		if (frm.fromdate.value == '')
		{
			alert('Please select a from date');
			fromcal.popup();
			return false;
		}
		else if (frm.todate.value == '')
		{
			alert('Please select a to date');
			tocal.popup();
			return false;
		}
		else if (!anyChecked('view_type'))
		{
			alert('Please select at least one type');
			return false;
		}
		else { return true; }
	}
</script>
<script language="javascript" src="/scripts/calendar.js"></script>

<?php echo $pg->outlineTableHead();?>
<form method="post" action="summary_print.php" name="frmReport" onsubmit="return verify(this)">
<input type="hidden" name="act" value="print" />
<tr>
	<td bgcolor="<?php echo $pg->color('table-label');?>"><b>From Date:</b></td>
	<td bgcolor="<?php echo $pg->color('table-cell');?>">
		<input type="text" size="12" name="fromdate" id="fromdate" value="<?php echo date('m/d/Y', $date_low);?>" onclick="fromcal.popup()">
		<a href="javascript:fromcal.popup();"><img src="/images/calendar/cal.gif" width="16" height="16" border="0" align="bottom" alt="Click to select a date" /></a>
		<input type="button" value="Today" onclick="$('fromdate').value='<?php echo date('m/d/Y');?>'" />
		<input type="button" value="Min" style="width:40px" onclick="$('fromdate').value='<?php echo $min;?>'" />
		<input type="button" value="Clear Date" onclick="$('fromdate').value=''" />
		<script type="text/javascript">var fromcal = new calendar2($('fromdate'));</script>
	</td>
</tr>
<tr>
	<td bgcolor="<?php echo $pg->color('table-label');?>"><b>To Date:</b></td>
	<td bgcolor="<?php echo $pg->color('table-cell');?>">
		<input type="text" size="12" name="todate" id="todate" value="<?php echo date('m/d/Y', $date_high);?>" onclick="tocal.popup()">
		<a href="javascript:tocal.popup();"><img src="/images/calendar/cal.gif" width="16" height="16" border="0" align="bottom" alt="Click to select a date" /></a>
		<input type="button" value="Today" onclick="$('todate').value='<?php echo date('m/d/Y');?>'" />
		<input type="button" value="Max" style="width:40px" onclick="$('todate').value='<?php echo $max;?>'" />
		<input type="button" value="Clear Date" onclick="$('todate').value=''" />
		<script type="text/javascript">var tocal = new calendar2($('todate'));</script>
	</td>
</tr>
<tr>
	<td bgcolor="<?php echo $pg->color('table-label');?>"><b>Platform:</b></td>
	<td bgcolor="<?php echo $pg->color('table-cell');?>">
		<select name="platformID" size="1">
			<option value="0">- All Platforms -</option>
			<?php
			foreach ($pla->values as $arr)
			{
				?><option value="<?php echo $arr[0];?>"><?php echo $arr[1];?></option><?php
			}
			?>
		</select>
	</td>
</tr>
<tr>
	<td bgcolor="<?php echo $pg->color('table-label');?>"><b>Employee:</b></td>
	<td bgcolor="<?php echo $pg->color('table-cell');?>">
		<select name="employeeID" style="vertical-align:middle">
			<option value="0">- All Employees -</option>
			<option value="active">- All Active Employees -</option>
			<option value="inactive">- All Inactive Employees -</option>
			<optgroup label="Active Employees">
				<?php
				foreach ($empl->employees as $employeeID => $arr)
				{
					if (!$arr['emp_active']) { continue; }
					?>
					<option value="<?php echo $employeeID;?>"><?php echo $arr['emp_lname'];?>, <?php echo $arr['emp_fname'];?></option>
					<?php
				}
				?>
			</optgroup>
			<optgroup label="Inactive Employees">
				<?php
				foreach ($empl->employees as $employeeID => $arr)
				{
					if ($arr['emp_active']) { continue; }
					?>
					<option value="<?php echo $employeeID;?>"><?php echo $arr['emp_lname'];?>, <?php echo $arr['emp_fname'];?></option>
					<?php
				}
				?>
			</optgroup>
		</select>
	</td>
</tr>
<tr>
	<td bgcolor="<?php echo $pg->color('table-label');?>"><b>Show:</b></td>
	<td bgcolor="<?php echo $pg->color('table-cell');?>">
		<input type="radio" name="show" id="show_total" value="total" class="nb"<?php echo getChecked();?> />
		<label for="show_total">Totals</label>

		<input type="radio" name="show" id="show_avg" value="average" class="nb" />
		<label for="show_avg">Averages</label>
	</td>
</tr>
<tr>
	<td bgcolor="<?php echo $pg->color('table-label');?>"><b>Group By:</b></td>
	<td bgcolor="<?php echo $pg->color('table-cell');?>">
		<input type="radio" name="groupby" id="gb_item" value="item" class="nb" />
		<label for="gb_item">Item</label>

		<input type="radio" name="groupby" id="gb_platform" value="platform" class="nb"<?php echo getChecked();?> />
		<label for="gb_platform">Platform</label>

		<input type="radio" name="groupby" id="gb_employee" value="employee" class="nb" />
		<label for="gb_employee">Employee</label>
	</td>
</tr>
<?php echo $pg->outlineTableFoot();?>

<p />

<input type="submit" value="View Report &gt;" class="btn">
</form>
<?php
$pg->foot();
?>
