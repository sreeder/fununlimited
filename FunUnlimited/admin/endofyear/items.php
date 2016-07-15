<?php
include('../../include/include.inc');

$cl = new check_login();

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

$pg = new admin_page();
$pg->setTitle('End of Year Item Quantities');
$pg->head('End of Year Item Quantities');

$error = new error('End of Year Item Quantities');

// show the platform selection form
$pla = new platforms($pg,0);
$pla->set_item('platforms');

// set/pull the global variables
include('items_elements.php');

// output the 'completed' status (if applicable)
if (isset($_GET['completed']) && isset($_GET['platformID']) && isset($_GET['yearID']) && isset($_GET['affected']))
{
	$platformID = $_GET['platformID'];
	$yearID = $_GET['yearID'];
	$year = @$years[$yearID];
	$affected = $_GET['affected'];

	// get the platform name
	$npla = new platforms($pg,$platformID);
	$platform_name = $npla->platform_name();

	$pg->status("Completed platform <b>$platform_name</b> for year <b>$year</b><br /><b>$affected</b> items had their quantities changed");
}

// decide which year the auto-select
$this_year = date('Y');
$month = date('m');
if ($month < 11) { $this_year--; }

if (!isset($_GET['year'])) { $year = $this_year; }
else { $year = @$_GET['year']; }
$yearID = array_search($year,$years);
if ($yearID === false) { $pg->error("Invalid year: $year"); $pg->foot(); die(); }

$select_years = array($this_year,($this_year+1)); // possible years in select box

// pull the completion status for each platform
$completed = array();
$completed_times = array();
$sql = "SELECT * FROM endofyear_platforms WHERE eyp_yearID=$yearID";
$result = mysql_query($sql,$db);
$error->mysql(__FILE__,__LINE__);
while ($row = mysql_fetch_assoc($result))
{
	$completed[$row['eyp_platformID']] = $row['eyp_completed'];
	$completed_times[$row['eyp_platformID']] = $row['eyp_completedtime'];
}

// output the year and platform selection form
?>
<script type="text/javascript">
	function openPlatform(platformID,complete)
	{
		if (!complete || confirm('You have decided to enter quantities for a platform that is already marked as completed.\nIf you do this, it will be marked as incomplete.\nThe quantities entered when this platform was marked as complete will be shown as the on-hand quantities.'))
		{
			document.location='/admin/endofyear/itemsUpdate.php?act=setcriteria&yearID=<?=$yearID;?>&platformID='+platformID;
		}
	}
</script>

Please select the platform you would like to enter the end-of-year quantities for.
<p />
Please remember that any changes you make using this utility have a profound impact on many aspects of the software!
<p />

<form method="get" action="/admin/endofyear/items.php">
	<b>Enter Quantities for Year:</b>
	<select name="year" size="1" onchange="this.form.submit()"><?php
		while (list($a,$y) = each($select_years))
		{
			if ($y == $year) { $s = ' selected="selected"'; } else { $s = ''; }
			?><option value="<?=$y;?>"<?=$s;?>><?=$y;?></option><?php
		}
	?></select>
	<input type="submit" value="Change &gt;" class="btn" />
</form>
<p />

<?php
$pg->outlineTableHead();
?>
<tr bgcolor="<?=$pg->color('table-head');?>">
	<td><b>Platform</b></td>
	<td><b>Completed?</b></td>
	<td><b>Functions</b></td>
</tr>
<?php
while (list($a,list($platformID,$name,$abb)) = each($pla->values))
{
	$bg = (($a%2)?$pg->color('table-cell'):$pg->color('table-cell2'));
	?>
	<tr bgcolor="<?=$bg;?>">
		<td><?=$name;?></td>
		<td><?=(@$completed[$platformID]?date('m/d/Y',$completed_times[$platformID]):'No');?></td>
		<td><a href="javascript:void(0)" onclick="openPlatform(<?=$platformID;?>,<?=(@$completed[$platformID]?YES:NO);?>);return false">Enter Qtys &gt;</a></td>
	</tr>
	<?php
}
$pg->outlineTableFoot();

$pg->foot();
?>