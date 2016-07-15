<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$show = 1000;
$perpage = 100;

$pages = ($show/$perpage);
$page = getG('page',1);
if ($page > $pages) { $page = $pages; }
$start = (($page - 1) * $perpage);
$end = ($page * $perpage);

$archived = getG('archived',0);
$ytd = ($archived ? YES : getG('ytd',NO));

$error = new error('Top 500 Customers Address Update');

$rnk = new rankings($pg);

// get the current
$rnk->setRankings('sales', $ytd, $archived);
$rankings = $rnk->getRankings();

// get the all-time
$rnk->setRankings('sales');
$alltime_rankings = $rnk->getRankings();

$pg = new admin_page();
$pg->setTitle('Top 500 Customers Address Update');
$pg->head('Top 500 Customers Address Update');

echo 'Showing ranks ' . ($start + 1) . " to $end<p />";

$st = new states();

$headings = array(
	'Rank',
	'Phone',
	'First',
	'Last',
	'Address',
	'City',
	'State',
	'Zip',
	($ytd ? 'YTD' : 'Value'),
	($ytd ? 'AT Rnk' : -1),
	($ytd ? 'All Time' : -1)
);

$rnk_field = 'rnk_' . ($ytd ? 'ytd_' : '') . 'sales';
$val_field = 'rnk_' . ($ytd ? 'ytd_' : '') . 'sales_value';
$show = array(
	$rnk_field     => array('show'),
	'cus_phone'    => array('text',15),
	'cus_fname'    => array('text',15),
	'cus_lname'    => array('text',20),
	'cus_address'  => array('text',25),
	'cus_city'     => array('text',15),
	'cus_state'    => array('select',1, $st->states),
	'cus_zip'      => array('text',7),
	$val_field     => array('show'),
	'all_time_rnk' => ($ytd ? array('show') : -1),
	'all_time_val' => ($ytd ? array('show') : -1)
);

?>
<script type="text/javascript">
	function doSubmit(dest)
	{
		var frm = document.getElementById('topfrm');
		frm.dest.value = dest;
		frm.submit();
	}
</script>

<font size="1">
	<b>Range:</b>
	<?php echo (!$ytd ? '<b><u>' : '<a href="/admin/utilities/top500.php?ytd=' . NO . '">');?>All Time<?php echo (!$ytd ? '</u></b>' : '</a>');?>
	|
	<?php echo ($ytd ? '<b><u>' : '<a href="/admin/utilities/top500.php?ytd=' . YES . '">');?>Year to Date<?php echo ($ytd ? '</u></b>' : '</a>');?><br />
	<b>YTD Year:</b>
	<?php
	$curyear = date('Y');
	$low = 2004; // the first year of archived
	for ($year=$curyear; $year>=$low; $year--)
	{
		$link = NO;
		$link_year = ($year==$curyear ? 0 : $year);
		if ($link_year == $archived)
		{
			echo '<b><u>';
		}
		elseif (!$ytd || $link_year != $archived)
		{
			$link = YES;
			?><a href="/admin/utilities/top500.php?ytd=<?php echo YES;?>&archived=<?php echo $link_year;?>"><?php
		}
		echo $year;
		if ($link)
		{
			echo '</a>';
		}
		if ($link_year == $archived)
		{
			echo '</u></b>';
		}
		if ($link_year != $low)
		{
			echo ' | ';
		}
	}
	?>
</font>

<p />

<input type="button" value="Print Customer List/Labels &gt;" onclick="document.location='top500_print.php?ytd=<?php echo $ytd;?>&archived=<?php echo $archived;?>'" class="btn" />

<p />

<?php echo $pg->outlineTableHead();?>
<form method="post" action="top500Update.php" id="topfrm">
	<input type="hidden" name="act" value="save" />
	<input type="hidden" name="page" value="<?php echo $page;?>" />
	<input type="hidden" name="pages" value="<?php echo $pages;?>" />
	<input type="hidden" name="ytd" value="<?php echo $ytd;?>" />
	<input type="hidden" name="archived" value="<?php echo $archived;?>" />
	<input type="hidden" name="dest" value="" />
	<tr align="center" bgcolor="<?php echo $pg->color('table-head');?>">
		<?php
		while (list($a, $heading) = each($headings))
		{
			if ($heading != -1)
			{
				?><td><b><?php echo $heading;?></b></td><?php
			}
		}
		?>
	</tr>
	<?php

	if (!count($rankings) && $ytd && $archived)
	{
		?>
		<tr bgcolor="<?php echo $pg->color('table-cell');?>">
			<td colspan="<?php echo count($show);?>" align="center">
				No archived rankings were found for <?php echo $archived;?><br />
				Please <a href="/admin/archive_rankings.php?year=<?php echo $archived;?>">click here</a> to archive the rankings for <?php echo $archived;?>.<br />
				After clicking the link, please wait for a minute!
			</td>
		</tr>
		<?php
	}

	$idx = -1;
	while (list($customerID, $arr) = each($rankings))
	{
		$idx++;
		if ($arr[$rnk_field] > $start && $arr[$rnk_field] <= $end)
		{
			$arr[$val_field] = number_format($arr[$val_field],2);
			$arr['all_time_rnk'] = '&nbsp;';
			$arr['all_time_val'] = '&nbsp;';
			if (isset($alltime_rankings[$customerID]))
			{
				$arr['all_time_rnk'] = $alltime_rankings[$customerID]['rnk_sales'];
				$arr['all_time_val'] = number_format($alltime_rankings[$customerID]['rnk_sales_value'],2);
			}

			$bg = (($idx%2) ? $pg->color('table-cell') : $pg->color('table-cell2'));

			?><tr bgcolor="<?php echo $bg;?>"><?php
			while (list($key, $config) = each($show))
			{
				if ($config == -1) { continue; }
				$type = $config[0];
				$size = @$config[1];
				$vals = @$config[2];

				?><td align="right"><?php

				if ($type == 'show')
				{
					echo $arr[$key];
				}
				elseif ($type == 'text')
				{
					if ($key == 'cus_phone') { $arr[$key] = $pg->format('phone', $arr[$key]); }
					?><input type="text" name="set[<?php echo $customerID;?>][<?php echo $key;?>]" size="<?php echo $size;?>" value="<?php echo $arr[$key];?>" /><?php
				}
				elseif ($type == 'select')
				{
					?><select name="set[<?php echo $customerID;?>][<?php echo $key;?>]" size="<?php echo $size;?>"><?php
					while (list($k, $v) = each($vals))
					{
						$s = ($k==$arr[$key] ? ' selected="selected"' : '');
						?><option value="<?php echo $k;?>"<?php echo $s;?>><?php echo $v;?></option><?php
					}
					?></select><?php
				}

				?></td><?php
			}
			reset($show);
			?></tr><?php
		}
	}

	$pg->outlineTableFoot();
	?>
	<p />
	<input type="button" value="&lt; Save &amp; Previous" onclick="doSubmit('previous')" class="btn" />
	<input type="button" value="&lt; Save &amp; Same &gt;" onclick="doSubmit('same')" class="btn" />
	<input type="button" value="Save &amp; Next &gt;" onclick="doSubmit('next')" class="btn" />
	<p />
	<input type="reset" value="Reset Form" class="btn" />
</form>
<?php

$pg->foot();
?>