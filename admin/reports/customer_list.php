<?php
include('../../include/include.inc');

$cl = new check_login();

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$popup = (isset($_GET['popup'])?$_GET['popup']:@$_POST['popup']);
$letter = (isset($_GET['letter'])?$_GET['letter']:@$_POST['letter']);
$narrow = (isset($_GET['narrow'])?$_GET['narrow']:@$_POST['narrow']);
$uenarrow = urlencode($narrow);

$pg = new admin_page();
$pg->setFull(($popup ? NO : YES));
$pg->setTitle('Customer List');
$pg->head('Customer List');

$error = new error('Customer List');

// !!! THIS SEARCH CODE NEEDS TO BE COMBINED INTO CUSTOMERS CLASS !!!

/*

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!! OPTION TO LIMIT TO THIS-STORE ONLY !!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

*/

// pull in the total customer count
$sql = "SELECT COUNT(*) AS count FROM customers";
$result = mysql_query($sql,$db);
$error->mysql(__FILE__,__LINE__);
$row = mysql_fetch_assoc($result);
$totcustomers = $row['count'];

// pull in all valid first-letters
$letters = array(); // array of valid first-letters
$sql = "SELECT DISTINCT UPPER(SUBSTRING(cus_lname,1,1)) AS letter FROM customers WHERE cus_lname!='' ORDER BY cus_lname";
$result = mysql_query($sql,$db);
$error->mysql(__FILE__,__LINE__);
while ($row = mysql_fetch_assoc($result)) { $letters[] = $row['letter']; }

// if no letter was passed, use the first letter that contains customers
if (!strlen($letter)) { $letter = $letters[0]; }
$letter = mysql_escape_string($letter);

// pull in the matching customers
$customers = array();
$sql = "SELECT * FROM customers WHERE cus_lname LIKE '{$letter}%'".(strlen($narrow)?" AND (cus_fname LIKE '%$narrow%' OR cus_lname LIKE '%$narrow%')":'')." ORDER BY cus_lname,cus_fname";
$result = mysql_query($sql,$db);
$error->mysql(__FILE__,__LINE__);
while ($row = mysql_fetch_assoc($result)) { $customers[] = $row; }

// output the letter selection/narrow by entry
$show = array();
while (list($a,$let) = each($letters))
{
	if ($let == $letter) { $show[] = "<u>$let</u>"; }
	elseif (in_array($let,$letters)) { $show[] = '<a href="javascript:letter(\''.mysql_escape_string($let).'\')">'.$let.'</a>'; }
	else { $show[] = $let; }
}

?>
<script type="text/javascript">
	function letter(let)
	{
		var obj = document.getElementById('cusfrm');
		obj.letter.value = let;
		obj.submit();
	}
	function narrow(clr)
	{
		var obj = document.getElementById('cusfrm');
		obj.narrow.value = (!clr?document.getElementById('narrowby').value:'');
		obj.submit();
	}
</script>

<form method="get" action="/admin/reports/customer_list.php" id="cusfrm">
	<input type="hidden" name="popup" value="<?=$popup;?>" />
	<input type="hidden" name="letter" value="<?=$letter;?>" />
	<input type="hidden" name="narrow" value="<?=htmlspecialchars($narrow);?>" />
</form>

<table border="0" cellspacing="0" cellpadding="5">
	<tr><td><b>Last Name Begins With: &nbsp; <?=implode(' &nbsp; ',$show);?></b></td></tr>
	<tr>
		<td>
			<form onsubmit="narrow();return false">
				<b>Narrow By:</b> <input type="text" id="narrowby" size="15" value="<?=htmlspecialchars($narrow);?>" />
				<input type="submit" value="Narrow &gt;" onclick="narrow(false)" class="btn" />
				<font size="1">(IE: &quot;scott&quot;, &quot;allen&quot;)</font>
			</form>
		</td>
	</tr>
	<?php
	if (strlen($narrow)) { ?><tr><td><b>Current Narrow By:</b> <?=htmlspecialchars($narrow);?> (<a href="javascript:narrow(true)">Clear</a>)</td></tr><?php }
	?>
</table>

<p />
<font size="1"><b>Note:</b> The <b>Narrow By</b> value only searches within the currently selected letter.</font>
<p />
<hr width="75%" size="-1" color="#000000" noshade="noshade" />
<p />
<b>Matching Customers:</b> <?=count($customers);?> / <b>Total Customers:</b> <?=$totcustomers;?>
<p />

<?=$pg->outlineTableHead(600);?>
<tr>
	<td bgcolor="<?=$pg->color('table-head');?>"><b>Name</b></td>
	<td bgcolor="<?=$pg->color('table-head');?>"><b>Address</b></td>
	<td bgcolor="<?=$pg->color('table-head');?>"><b>Phone</b></td>
	<td bgcolor="<?=$pg->color('table-head');?>"><b>Cust. #<br />Cust. Since</b></td>
</tr>
<?php
while (list($a,$arr) = each($customers))
{
	$bg = (($a%2)?$pg->color('table-cell'):$pg->color('table-cell2'));

	?>
	<tr><!-- height="45" -->
		<td width="100%" valign="top" bgcolor="<?=$bg;?>"><?="{$arr['cus_lname']}, {$arr['cus_fname']}";?></td>
		<td valign="top" bgcolor="<?=$bg;?>">
			<?=$arr['cus_address'];?><br />
			<?=trim("{$arr['cus_city']}, {$arr['cus_state']} {$arr['cus_zip']}",',');?>
			<?php
			if (strlen($arr['cus_email'])) { ?><br /><a href="mailto:<?=$arr['cus_email'];?>"><?=$arr['cus_email'];?></a><?php }
			?>
		</td>
		<td valign="top" bgcolor="<?=$bg;?>">
			<?=(strlen($arr['cus_phone'])?$pg->format('phone',$arr['cus_phone']):'');?>
			<?=(strlen($arr['cus_cellphone'])?'<br />Cell: '.$pg->format('phone',$arr['cus_cellphone']):'');?>
		</td>
		<td valign="top" align="right" bgcolor="<?=$bg;?>">#<?=$arr['cus_customerID'];?><br /><?=date('m/d/Y',$arr['cus_timeadded']);?></td>
	</tr>
	<?php
}
if (!count($customers)) { ?><tr><td align="center" colspan="4" bgcolor="<?=$pg->color('table-cell');?>">--- No Customers Found ---</td></tr><?php }

$pg->outlineTableFoot();

$pg->foot();
?>