<?php
include('../../include/include.inc');

$cl = new check_login(STORE);
$error = new error('Summary Report');

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Summary Report');
$pg->head('Summary Report');

$crit = (getG('sortby') ? $_SESSION['summary_crit'] : getP());
$_SESSION['summary_crit'] = $crit;

$act     = getA($crit, 'act');
$groupby = getA($crit, 'groupby');
$show    = getA($crit, 'show');
if ($act != 'print')
{
	headerLocation('/admin/reports/summary.php');
}

// output the summary report
if (isset($_GET['sortby']))
{
	$summary_data = $_SESSION['summary_data'];
	$summary_range = $_SESSION['summary_range'];
}
else
{
	$inv = new invoice($pg);
	$inv->setSummaryData($crit);
	$summary_data = $inv->getSummaryData();
	$summary_range = $inv->getSummaryRange();
	unset($inv);

	// cache the data
	$_SESSION['summary_data'] = $summary_data;
	$_SESSION['summary_range'] = $summary_range;
}
$th_label = ucwords($groupby);

$empl = new employees($pg);
$empl->get_employees(BOTH);
$employees = $empl->employees;
unset($empl);

// sort the data
$sortbys = array(
	'platform'      => 'Platform',
	'item'          => 'Item',
	'sales'         => 'Gross Sales',
	'trades_cash'   => 'Trades - Cash',
	'trades_credit' => 'Trades - Credit',
	'returns'       => 'Returns',
	'net_sales'     => 'Net Sales'
);
$sortby = getG('sortby', 'sales');

if ($groupby != 'platform' && $groupby != 'item')
{
	unset($sortbys['platform']);
}
if ($groupby != 'item')
{
	unset($sortbys['item']);
}

?>
<p class="noprint">
	<input type="button" value="&lt; Back" onclick="go('summary.php')" class="btn" />
</p>

<p>
	<b>Date Range:</b> <?php echo $summary_range;?><br />
	<b>Showing:</b> <?php echo ucwords($show);?>s
</p>

<p class="noprint">
	<b>Sort By:</b>
	<?php
	$shown = 0;
	$count = count($sortbys);
	foreach ($sortbys as $sb => $name)
	{
		if ($sb == $sortby)
		{
			$all_data = $summary_data['_all'];
			unset($summary_data['_all']);

			$sort = new sort();
			if ($sb == 'platform')
			{
				if ($groupby == 'item')
				{
					$summary_data = $sort->doSort($summary_data, 'platform', SORT_ASC, 'item', SORT_ASC);
				}
				else
				{
					$summary_data = $sort->doSort($summary_data, $sb, SORT_ASC);
				}
			}
			else
			{
				$sort_dir = ($sb=='item' ? SORT_ASC : SORT_DESC);
				$summary_data = $sort->doSort($summary_data, $sb, $sort_dir);

				if ($groupby == 'employee')
				{
					$sort->fixArrayKeys($summary_data, 'employeeID');
				}
			}
			unset($sort);

			$summary_data['_all'] = $all_data;
			unset($all_data);

			?>
			<b><u><?php echo $name;?></u></b>
			<?php
		}
		else
		{
			?>
			<a href="summary_print.php?sortby=<?php echo $sb;?>"><?php echo $name;?></a>
			<?php
		}

		$shown++;
		if ($shown < $count)
		{
			echo ' | ';
		}
	}
	?>
</p>

<p class="note">
	<b>Format:</b> [Total Qty] [<?php echo ucwords($show);?> $]<br />
	Net Sales = Gross Sales - (Trades Cash + Trades Credit + Returns)
</p>

<table border="0">
	<tr>
		<?php
		$rowspan = (count($summary_data) + 1);

		if ($groupby == 'item')
		{
			?>
			<th>Platform</th>
			<th class="lb" rowspan="<?php echo $rowspan;?>"><img src="/images/blank.gif" /></th>
			<th>Item</th>
			<?php
		}
		else
		{
			?>
			<th><?php echo $th_label;?></th>
			<?php
		}
		?>
		<th class="lb" rowspan="<?php echo $rowspan;?>"><img src="/images/blank.gif" /></th>
		<th colspan="2">Gross Sales</th>
		<th class="lb" rowspan="<?php echo $rowspan;?>"><img src="/images/blank.gif" /></th>
		<th colspan="2">Trades Cash</th>
		<th class="lb" rowspan="<?php echo $rowspan;?>"><img src="/images/blank.gif" /></th>
		<th colspan="2">Trades Credit</th>
		<th class="lb" rowspan="<?php echo $rowspan;?>"><img src="/images/blank.gif" /></th>
		<th colspan="2">Returns</th>
		<th class="lb" rowspan="<?php echo $rowspan;?>"><img src="/images/blank.gif" /></th>
		<th colspan="2">Net Sales</th>
	</tr>
	<?php
	$max_title_length = 35;
	foreach ($summary_data as $label => $arr)
	{
		$totals = (is_string($label) && $label == '_all');
		$class = ($totals ? 'bold' : '');
		$show_label = ($totals ? 'TOTALS:' : $label);

		if ($groupby == 'employee' && !$totals)
		{
			$emp_info = $employees[$label];
			$show_label = $emp_info['emp_lname'] . ', ' . $emp_info['emp_fname'];
			unset($emp_info);
		}

		?>
		<tr class="<?php echo $class;?>">
			<?php
			if ($groupby == 'item')
			{
				$item = $arr['item'];
				if (strlen($item) > $max_title_length)
				{
					$item = substr($item, 0, $max_title_length) . '...';
				}
				?>
				<td><?php echo ($totals ? $show_label : $arr['platform']);?></td>
				<td><?php echo $item;?></td>
				<?php
			}
			else
			{
				?>
				<td align="right"><?php echo $show_label;?></td>
				<?php
			}
			?>
			<td align="right"><?php echo number_format($arr['count_sales'], 0);?></td>
			<td align="right">$<?php echo number_format($arr['sales'], 2);?></td>
			<td align="right"><?php echo number_format($arr['count_trades_cash'], 0);?></td>
			<td align="right">$<?php echo number_format($arr['trades_cash'], 2);?></td>
			<td align="right"><?php echo number_format($arr['count_trades_credit'], 0);?></td>
			<td align="right">$<?php echo number_format($arr['trades_credit'], 2);?></td>
			<td align="right"><?php echo number_format($arr['count_returns'], 0);?></td>
			<td align="right">$<?php echo number_format($arr['returns'], 2);?></td>
			<td align="right"><?php echo number_format($arr['count_sales'], 0);?></td>
			<td align="right">$<?php echo number_format($arr['net_sales'], 2);?></td>
		</tr>
		<?php
	}
	?>
</table>
<?php

$pg->foot();
?>
