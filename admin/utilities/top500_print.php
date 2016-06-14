<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$show = 500;
$percol = 3;
$break_every = 15;
$ytd = getG('ytd',NO);
$archived = getG('archived',0);

$error = new error('Top 500 Customers Address Print');

$rnk = new rankings($pg);
$rnk->setRankings('sales',$ytd,$archived);
$rankings = $rnk->getRankings();

$field = 'rnk_' . ($ytd ? 'ytd_' : '') . 'sales';

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Top 500 Customers Address Update');
$pg->head();

?>
<style type="text/css">
	table.list
	{
		border:dashed 1px #AAAAAA;
		border-collapse:collapse;
	}
	tr.list td
	{
		font-size:14px;
		border:dashed 1px #AAAAAA;
		padding:3px;
	}
	tr.break
	{
		page-break-after:always;
	}
</style>

<table class="list">
	<?php
	$shown = 0;
	$rows = 0;
	while (list($a,$arr) = each($rankings))
	{
		if ($arr[$field] <= $show)
		{
			if (!$shown)
			{
				$rows++;
				if (!($rows%$break_every)) { ?><tr class="break"><td colspan="<?=$percol;?>"></td></tr><?php }

				?><tr class="list"><?php
			}
			$shown++;

			?>
			<td>
				<?="{$arr['cus_fname']} {$arr['cus_lname']}";?><br />
				<?=$arr['cus_address'];?><br />
				<?="{$arr['cus_city']}, {$arr['cus_state']} ".$pg->format('zip',$arr['cus_zip']);?>
			</td>
			<?php

			if ($shown == $percol)
			{
				?></tr><?php
				$shown = 0;
			}
		}
	}
	?>
</table>
<?php

$pg->foot();
?>