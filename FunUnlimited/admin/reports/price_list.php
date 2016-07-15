<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Price List');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);


$pg = new admin_page();
$pg->setTitle('Price List');
$pg->head('Price List');

// output the platform selection boxes
$platforms = array(); // format: $platforms[platformID] = name
$sql = "SELECT pla_platformID,pla_name FROM platforms,items WHERE pla_platformID=itm_platformID GROUP BY itm_platformID ORDER BY pla_name";
$result = mysql_query($sql,$db);
$error->mysql(__FILE__,__LINE__);
while ($row = mysql_fetch_row($result)) { $platforms[$row[0]] = $row[1]; }

$select = array(1,2,3,11,12,13,14,15,18,20,42); // platforms to default as selected (game-related platforms)
?>
Please select the platforms you would like to include in the price list.
<p />
<script language="javascript" src="/scripts/listbox.js"></script>
<form method="post" action="/admin/reports/price_list_print.php" target="_blank" name="prclst" onsubmit="selectAllOptions(this.elements['platforms[]'])">
<input type="hidden" name="act" value="print">
<table cellpadding="3" cellspacing="0" border="0">
	<tr>
		<td align="center"><b>Not Selected</b><br>
			<select name="not_used" id="not_used" multiple="multiple" size="20" ondblClick="move(document.prclst.not_used,document.prclst.used)" style="width:200px"><?php
				while (list($id,$name) = each($platforms))
				{
					if (!in_array($id,$select)) { ?><option value="<?php echo $id;?>"><?php echo $name;?></option><?php }
				}
				reset($platforms);
			?></select>
		</td>
		<td valign="middle" align="center" class="eight">
			<input type="button" name="right_move" value="&gt;&gt;" onclick="move(document.prclst.not_used,document.prclst.used);" class="btn" /><br>
			<input type="button" name="right_move_all" value="All &gt;&gt;" onclick="moveAll(document.prclst.not_used,document.prclst.used);" class="btn" /><br>
			<input type="button" name="left_move" value="&lt;&lt;" onclick="move(document.prclst.used,document.prclst.not_used);" class="btn" /><br>
			<input type="button" name="left_move_all" value="All &lt;&lt;" onclick="moveAll(document.prclst.used,document.prclst.not_used);" class="btn" />
		</td>
		<td align="center"><b>Selected</b><br>
			<select name="platforms[]" id="used" multiple="multiple" size="20" ondblClick="move(document.prclst.used,document.prclst.not_used)" style="width:200px"><?php
				while (list($id,$name) = each($platforms))
				{
					if (in_array($id,$select)) { ?><option value="<?php echo $id;?>"><?php echo $name;?></option><?php }
				}
				reset($platforms);
			?></select>
		</td>
	</tr>
</table>

<p>
	<input type="checkbox" name="instock" value="<?php echo YES;?>" id="is" class="nb" />
	<label for="is">Only show in-stock items (new or used)</label><br />

	<input type="checkbox" name="show0" value="<?php echo YES;?>" id="s0" class="nb" checked="checked" />
	<label for="s0">Show items with $0.00 price</label><br />
</p>

<input type="submit" value="Print Price List &gt;" class="btn">
<input type="button" value="Reset Selected Platforms &gt;" onclick="document.location='/admin/reports/price_list.php'" class="btn">
</form>

<p>
	<b>Note:</b> It may take a few minutes to generate and display the price list - please be patient!
</p>
<?php

$pg->foot();
?>
