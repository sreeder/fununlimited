<?php
include('../../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setTitle('Minimum Prices');
$pg->head('Minimum Prices');

$mp = new min_prices($pg);

$act = @$_POST['act'];
$active = (isset($_GET['active'])?$_GET['active']:(isset($_POST['active'])?$_POST['active']:YES));

if ($act == "")
{
	// display minimum price form
	$mp->prices_form();
}
elseif ($act == "set")
{
	// set the prices
	$mp->pull_post();
	$mp->set();

	if ($mp->was_set())
	{
		$pg->status('Set '.count($mp->prices).' minimum prices');
		$mp->prices_form();
	}
	else
	{
		echo "ERROR SETTING PRICES! CONTACT SCOTT! (this shouldn't happen...)";
	}
}

$pg->foot();
?>