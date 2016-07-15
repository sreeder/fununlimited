<?php
include('../../include/include.inc');

$cl = new check_login(STORE);
$error = new error('Sales Graphs');

$popup = getG('popup',NO);
$tab = getG('tab','hourly',array('hourly','daily','monthly'));
$date = getG('date', date('m/d/Y'));
$time = @strtotime($date);

$pg = new admin_page();
$pg->setFull((!$popup ? YES : NO));
$pg->setTitle('Current Sales Graphs - ' . date('m/d/Y', $time));
$pg->head('Current Sales Graphs - ' . date('m/d/Y', $time));
$pg->addOnload("showTab('$tab')");

// get the sales figures
$sf = new sales_figures($pg);
$sf->setReferenceTime(@strtotime($date));
$type_words = $sf->getTypeWords();
$type = getGP('type','sales',array_keys($type_words));

// hourly
$this_hourly = $sf->getHourly( 0,$type); // today
$avg_hourly  = $sf->getHourly(-1,$type); // average
list($projected_hourly,$crossover_hourly) = $sf->getProjected($this_hourly,$avg_hourly); // projected
$sf->removeIrrelevant($this_hourly,$avg_hourly,$projected_hourly);

// daily
$this_daily = $sf->getDaily( 0,$type); // this month
$avg_daily  = $sf->getDaily(-1,$type); // average
list($projected_daily,$crossover_daily) = $sf->getProjected($this_daily,$avg_daily); // projected
$sf->removeIrrelevant($this_daily,$avg_daily,$projected_daily);

// monthly
$this_monthly = $sf->getMonthly( 0,$type); // this year
$avg_monthly  = $sf->getMonthly(-1,$type); // average
list($projected_monthly,$crossover_monthly) = $sf->getProjected($this_monthly,$avg_monthly); // projected
$sf->removeIrrelevant($this_monthly,$avg_monthly,$projected_monthly);

// data for /makeGraph.php image creator
$graph_data = array(
	'hourly' => array(
		'key' => array(
			date('l F j, Y', $time) . (date('m/d/Y')==date('m/d/Y', $time) ? ' [Today]' : ''),
			'Average ' . date('l', $time),
			'Projected'
		),
		'data' => array(
			$this_hourly,
			$avg_hourly,
			$projected_hourly
		),
		'crossover' => $crossover_hourly
	),
	'daily' => array(
		'key' => array(
			date('F', $time) . ' ' . date('Y', $time),
			'Average ' . date('F', $time),
			'Projected'
		),
		'data' => array(
			$this_daily,
			$avg_daily,
			$projected_daily
		),
		'crossover' => $crossover_daily
	),
	'monthly' => array(
		'key' => array(
			date('Y', $time),
			'Average Year',
			'Projected'
		),
		'data' => array(
			$this_monthly,
			$avg_monthly,
			$projected_monthly
		),
		'crossover' => $crossover_monthly
	)
);
$_SESSION['graph_data'] = $graph_data;

$graph_width  = 600;
$graph_height = 300;

?>
<style type="text/css">
	.graphcell
	{
		border:solid 1px black;
		border-top:0px;
		text-align:center;
		width:<?php echo $graph_width;?>px;
		height:<?php echo $graph_height;?>px;
	}
	.tabsel
	{
		background:url('/images/navbar/navbar_sel.gif');
		width:120px;
		text-align:center;
		vertical-align:top;
		padding-top:4px;
		color:#FFFFFF;
		font-weight:bold;
		font-size:12;
		cursor:hand;
	}
	.tabnosel
	{
		background:url('/images/navbar/navbar_nosel.gif');
		width:120px;
		text-align:center;
		vertical-align:top;
		padding-top:4px;
		color:#FFFFFF;
		font-weight:bold;
		font-size:12;
		cursor:hand;
	}
	#data
	{
		height:50px;
	}
</style>

<script type="text/javascript">
	var curtab = 'hourly';
	var curtype = '<?php echo $type;?>';
	var runtot = true;
	var labels = true;

	function doHide()
	{
		$('data').innerHTML = '<b>Please Hold...</b>';
	}

	function showTab(which)
	{
		curtab = which;
		var all_tabs = ['<?php echo implode("','", array_keys($graph_data));?>'];
		for (var i=0; i<all_tabs.length; i++)
		{
			$(all_tabs[i]).className = (all_tabs[i]==which ? 'tabsel' : 'tabnosel');
			$('datatable_' + all_tabs[i]).style.display = (all_tabs[i]==which ? 'block' : 'none');
		}
		$('graphimg').src =
			'/admin/makeGraph.php?'
			+ 'which=' + curtab
			+ '&runtot=' + (runtot ? <?php echo YES;?> : <?php echo NO;?>)
			+ '&labels=' + (labels ? <?php echo YES;?> : <?php echo NO;?>)
			+ '&width=<?php echo $graph_width;?>'
			+ '&height=<?php echo $graph_height;?>'
			+ '&image=<?php echo YES;?>'
			+ '&date=' + $F('date')
		;
		//prompt('booga',$('graphImg').src);
	}
	function setRunningTotal(chk)
	{
		runtot = chk;
		showTab(curtab);
	}
	function setLabels(chk)
	{
		labels = chk;
		showTab(curtab);
	}

	/**
	* Change the type
	* @param	string	new_type
	*/
	function changeType(new_type)
	{
		var url = '/admin/reports/sales_graphs.php?'
			+ 'popup=<?php echo $popup;?>'
			+ '&type=' + new_type
			+ '&tab=' + curtab
			+ '&date=' + $F('date')
		;
		go(url);
		doHide();
	}

	/**
	* Change the date
	*/
	var cur_date = '';
	function checkDate()
	{
		if (!cur_date.length)
		{
			cur_date = $F('date');
		}

		if ($F('date') != cur_date)
		{
			changeType(curtype);
		}
		else
		{
			setTimeout('checkDate()', 500);
		}
	}

	/**
	* Go back/forward a date
	* @param	integer	dir	-1 for YESterday, 0 for today, 1 for tomorrow
	*/
	function goDate(dir)
	{
		if (dir == -30)
		{
			$('date').value = '<?php echo date('m/d/Y', strtotime(date('m/d/Y', $time) . '-1 month'));?>';
		}
		else if (dir == -1)
		{
			$('date').value = '<?php echo date('m/d/Y', strtotime(date('m/d/Y', $time) . '-1 day'));?>';
		}
		else if (dir == 0)
		{
			$('date').value = '<?php echo date('m/d/Y');?>';
		}
		else if (dir == 1)
		{
			$('date').value = '<?php echo date('m/d/Y', strtotime(date('m/d/Y', $time) . ' +1 day'));?>';
		}
		else if (dir == 30)
		{
			$('date').value = '<?php echo date('m/d/Y', strtotime(date('m/d/Y', $time) . ' +1 month'));?>';
		}
	}
</script>
<script language="javascript" src="/scripts/calendar.js"></script>

<?php
$pg->addOnload('checkDate()');

if ($popup)
{
	?>
	<input type="button" value="&lt; Close Window" onclick="window.close()" class="btn" /><br />
	<img src="/images/blank.gif" width="1" height="10" /><br />
	<?php
} // if popup

?>
<div id="data">
<table border="0">
	<tr>
		<td>
			<a href="javascript:void(0)" onclick="goDate(-30)" title="Previous Month"><img src="/images/search_first_small.gif" width="17" height="18" border="0" /></a>
			&nbsp;
			<a href="javascript:void(0)" onclick="goDate(-1)" title="Previous Day"><img src="/images/search_prev_small.gif" width="12" height="18" border="0" /></a>
			&nbsp;
		</td>
		<td>
			<input type="text" size="12" name="date" id="date" readonly="readonly" onclick="datecal.popup()" value="<?php echo $date;?>" />
			<a href="javascript:datecal.popup();"><img src="/images/calendar/cal.gif" width="16" height="16" border="0" align="bottom" alt="Click to select a date" /></a>
			<input type="button" value="Today" onclick="goDate(0)" />

			<script type="text/javascript">
				var datecal = new calendar2($('date'));
			</script>
		</td>
		<td>
			&nbsp;
			<a href="javascript:void(0)" onclick="goDate(1)" title="Next Day"><img src="/images/search_next_small.gif" width="12" height="18" border="0" /></a>
			&nbsp;
			<a href="javascript:void(0)" onclick="goDate(30)" title="Next Month"><img src="/images/search_last_small.gif" width="17" height="18" border="0" /></a>
		</td>
	</tr>
</table>

<span class="note">
	<b>Type:</b>
	<?php
	$idx = 0;
	while (list($link_type,$type_word) = each($type_words))
	{
		$idx++;
		if ($link_type == $type)
		{
			?><b><u><?php
		}
		else
		{
			?><a href="javascript:void(0)" onclick="changeType('<?php echo $link_type;?>');return false"><?php
		}

		echo $type_word;

		echo ($link_type==$type ? '</u></b>' : '</a>');
		echo ($idx<count($type_words) ? ' | ' : '');
	} // each type name
	?>
</span>
</div>

<table border="0" cellspacing="0" cellpadding="0" width="602">
	<tr>
		<td align="left" width="43" background="/images/navbar/navbar_bg.gif"><img src="/images/navbar/navbar_left.gif" width="43" height="31" /></td>
		<td id="hourly" class="tabsel" onclick="showTab('hourly')">Hourly</td>
		<td id="daily" class="tabnosel" onclick="showTab('daily')">Daily</td>
		<td id="monthly" class="tabnosel" onclick="showTab('monthly')">Monthly</td>
		<td align="right" width="199" background="/images/navbar/navbar_bg.gif">
			<input type="checkbox" id="runtot" onclick="setRunningTotal(this.checked)" checked="checked" class="nb" /> <label for="runtot">Running Total</label>
			<input type="checkbox" id="labels" onclick="setLabels(this.checked)" checked="checked" class="nb" /> <label for="labels">Labels</label>
			<img src="/images/navbar/navbar_right.gif" width="20" height="31" align="top" />
		</td>
	</tr>
	<tr>
	<td colspan="5" class="graphcell">
		<img id="graphimg" src="/images/blank.gif" width="<?php echo $graph_width;?>" height="<?php echo $graph_height;?>" />
	</td>
</table>

<p />

<?php
$cols = 12;

while (list($which,$arr) = each($graph_data))
{
	array_push($arr['key'],'Difference');
	$key = $arr['key'];
	$data = $arr['data'];
	$runtot = array(0,0,0);

	$chunks1 = array_chunk($data[0],$cols,true);
	$chunks2 = array_chunk($data[1],$cols,true);

	?>
	<span id="datatable_<?php echo $which;?>" style="display:<?php echo ($which=='hourly'?'block':'none');?>">
		<table border="0" cellspacing="1" cellpadding="3">
			<?php
			while (list($b,$chunk1) = each($chunks1))
			{
				$chunk2 = $chunks2[$b];

				?>
				<tr>
					<td>&nbsp;</td>
					<?php
					$keys = array_keys($chunk1);
					while (list($k,$v) = each($keys))
					{
						?>
						<td align="center" bgcolor="<?php echo $pg->color('table-head');?>">
							<b><?php echo $sf->formatLabel($which,$v);?></b>
						</td>
						<?php
					}
					?>
					<td>&nbsp;</td>
				</tr>
				<?php
				$diffs = array();
				while  (list($k,$c1val) = each($chunk1))
				{
					$c2val = $chunk2[$k];
					$diff = number_format(($c1val-$c2val),2);
					if ($diff < 0)
					{
						$diff = '<font color="red"><b>' . $diff . '</b></font>';
					}
					else
					{
						$diff = "+$diff";
					}
					$diffs[$k] = $diff;
				}
				reset($chunk1);

				$data_arr = array($chunk1,$chunk2,$diffs);
				while (list($a,$arr) = each($data_arr))
				{
					?>
					<tr>
						<td bgcolor="<?php echo $pg->color('table-head');?>">
							<b><?php echo $key[$a];?></b>
						</td>
						<?php
						while (list($k,$v) = each($arr))
						{
							$runtot[$a] += $v;
							?>
							<td align="right" bgcolor="<?php echo $pg->color(($a<2 ? 'table-cell2' : 'table-cell'));?>">
								<?php echo ($a<2 ? number_format($v,2) : $v);?>
								<?php echo ($a<2 ? '<br />' . number_format($runtot[$a],2) : '');?>
							</td>
							<?php
						}
						?>
						<td bgcolor="<?php echo ($a<2 ? $pg->color('table-head') : '#FFFFFF');?>">
							<?php echo ($a<2 ? '� Total' : '&nbsp;');?>
							<?php echo ($a<2 ? '<br />� Running' : '');?>
						</td>
					</tr>
					<?php
				}
			}
			?>
		</table>
	</span>
	<?php
}
reset($graph_data);

$pg->foot();
?>