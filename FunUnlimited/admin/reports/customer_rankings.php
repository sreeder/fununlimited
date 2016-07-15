<?php
include('../../include/include.inc');

$cl = new check_login(STORE);
$error = new error('Customer Rankings');
$pg = new admin_page();

$pg->setTitle('Customer Rankings');
$pg->head('Customer Rankings',(@$_SESSION['customerID']?YES:NO));

$per_page = 750;

$type = getGP('type','sales',array('sales','cashtrades','credittrades','returns'));
$page = getGP('page',1);
$ytd = getGP('ytd',NO);

$rnk = new rankings($pg);
$rnk->setRankings($type,$ytd);
$rankings = $rnk->getRankings();

$chunks = array_chunk($rankings,$per_page,true);
if (!isset($chunks[($page - 1)])) { $page = 1; }
$show = @$chunks[($page - 1)];
if (!is_array($show))
{
	// this will only happen on 3/6 before an invoice is done...an unlikely event...
	$show = array();
}
$tot_pages = count($chunks);

?>
<font size="1">
	<b>Viewing:</b>
	<?=($type=='sales' ? '<b><u>' : '<a href="/admin/reports/customer_rankings.php?type=sales&ytd=' . $ytd . '">');?>Sales<?=($type=='sales' ? '</u></b>' : '</a>');?>
	|
	<?=($type=='cashtrades' ? '<b><u>' : '<a href="/admin/reports/customer_rankings.php?type=cashtrades&ytd=' . $ytd . '">');?>Cash Trades<?=($type=='cashtrades' ? '</u></b>' : '</a>');?>
	|
	<?=($type=='credittrades' ? '<b><u>' : '<a href="/admin/reports/customer_rankings.php?type=credittrades&ytd=' . $ytd . '">');?>Credit Trades<?=($type=='credittrades' ? '</u></b>' : '</a>');?>
	|
	<?=($type=='returns' ? '<b><u>' : '<a href="/admin/reports/customer_rankings.php?type=returns&ytd=' . $ytd . '">');?>Returns<?=($type=='returns' ? '</u></b>' : '</a>');?>
	<br />
	<b>Range:</b>
	<?=(!$ytd ? '<b><u>' : '<a href="/admin/reports/customer_rankings.php?type=' . $type . '&ytd=' . NO . '">');?>All Time<?=(!$ytd ? '</u></b>' : '</a>');?>
	|
	<?=($ytd ? '<b><u>' : '<a href="/admin/reports/customer_rankings.php?type=' . $type . '&ytd=' . YES . '">');?>Year to Date<?=($ytd ? '</u></b>' : '</a>');?>
</font>

<p />

<b>Page:</b>
<a href="/admin/reports/customer_rankings.php?type=<?php echo $type;?>&ytd=<?php echo $ytd;?>&page=<?php echo ($page>1 ? ($page - 1) : $page);?>">&lt;&lt;</a>
&nbsp;
<?php
for ($i=1; $i<=count($chunks); $i++)
{
	echo ($i==$page ? '<b><u>' : '<a href="/admin/reports/customer_rankings.php?type=' . $type . '&ytd=' . $ytd . '&page=' . $i . '">');
	echo $i;
	echo ($i==$page ? '</u></b>' : '</a>');
	echo ' &nbsp; ';
}
?>
<a href="/admin/reports/customer_rankings.php?type=<?php echo $type;?>&ytd=<?php echo $ytd;?>&page=<?php echo ($page<count($chunks) ? ($page + 1) : count($chunks));?>">&gt;&gt;</a>

<p />

<?php
$customer_page = 0;
if (@$_SESSION['customerID'])
{
	// get the page the customer is on
	while (list($a,$chunk) = each($chunks))
	{
		if (isset($chunk[$_SESSION['customerID']]))
		{
			$customer_page = ($a + 1);
			break;
		}
	}

	?>
	<?=$pg->outlineTableHead();?>
		<tr><td align="center" bgcolor="<?=$pg->color('table-head');?>"><b>Current Customer</b></td></tr>
		<tr><td bgcolor="<?=$pg->color('table-cell');?>"><span id="custrank">Unknown</span></td></tr>
	<?=$pg->outlineTableFoot();?>
	<p />
	<input type="button" value="&lt; Return to Customer Information" onclick="document.location='/admin/pos/pos.php'" class="btn" />
	<p />
	<?php
}
?>

<script type="text/javascript">
	function customer_rank(rank,total,value,tonext)
	{
		var obj = document.getElementById('custrank');

		if (rank == -1) { obj.innerHTML = 'No Ranking'; }
		else
		{
			var html = '';
			html += '<b>Rank:</b> #'+rank+' of '+total+'<br />';
			html += '<b>Value:</b> $'+format_price(value)+'<br />';
			if (rank > 1)
			{
				html += '<b>Needed for Next Rank:</b> $'+format_price(tonext);
			}
			if (<?php echo $customer_page;?> != 0)
			{
				html += '<br /><center><a href="/admin/reports/customer_rankings.php?type=<?php echo $type;?>&ytd=<?php echo $ytd;?>&page=<?php echo $customer_page;?>#id<?php echo @$_SESSION['customerID'];?>">Go to Customer</a></center>';
			}

			obj.innerHTML = html;
		}
	}
</script>
<?php

$custrank = -1;
$custrankval = -1;
$custranktonext = -1;
$rankvals = array();

$field_rank = 'rnk_' . ($ytd ? 'ytd_' : '') . $type;
$field_value = 'rnk_' . ($ytd ? 'ytd_' : '') . $type . '_value';

while (list($a,$arr) = each($rankings))
{
	$rank = $arr[$field_rank];
	$rankval = $arr[$field_value];

	if ($arr['cus_customerID'] == @$_SESSION['customerID'])
	{
		$p = '<font color="red"><b>'; $s = '</b></font>';
		$custrank = $rank;
		$custrankval = $rankval;
		if ($rank > 1)
		{
			for ($i=($rank-1); $i>0; $i--)
			{
				if (isset($rankvals[$i]))
				{
					$custranktonext = ($rankvals[$i] - $rankval);
					break;
				}
			}
		}
	}

	$rankvals[$rank] = $rankval;
}

$pg->outlineTableHead();
?>
<tr bgcolor="<?=$pg->color('table-head');?>">
	<td><b>Rank</b></td>
	<td><b>Value ($)</b></td>
	<td><b>Customer</b></td>
</tr>
<?php
while (list($customerID,$arr) = each($show))
{
	$bg = (($a%2)?$pg->color('table-cell'):$pg->color('table-cell2'));

	if ($customerID == @$_SESSION['customerID'])
	{
		$p = '<font color="red"><b>';
		$s = '</b></font>';
	}
	elseif ($arr[$field_rank] <= 10)
	{
		$p = '<b>';
		$s = '</b>';
	}
	else
	{
		$p = '';
		$s = '';
	}

	?>
	<tr bgcolor="<?=$bg;?>">
		<td align="right"><a name="id<?=$customerID;?>" title="<?php echo $customerID;?>" style="color:#000000;text-decoration:none"><?=$p . $arr[$field_rank] . $s;?></a></td>
		<td align="right"><?=$p . $arr[$field_value] . $s;?></td>
		<td><?=$p . "{$arr['cus_lname']}, {$arr['cus_fname']}" . $s;?></td>
	</tr>
	<?php
}
if (!count($rankings))
{
	?>
	<tr>
		<td colspan="3" bgcolor="<?=$pg->color('table-cell');?>" align="center">--- No Rankings ---</td>
	</tr>
	<?php
}
$pg->outlineTableFoot();

if (@$_SESSION['customerID'])
{
	$allranks = array_keys($rankvals);
	$totalranks = (count($allranks) ? $allranks[(count($allranks)-1)] : 0);
	$pg->addOnload("customer_rank($custrank,$totalranks,$custrankval,$custranktonext)");
}

$pg->foot();
?>