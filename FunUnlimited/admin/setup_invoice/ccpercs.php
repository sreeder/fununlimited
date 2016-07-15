<?php
include('../../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setTitle('Cash/Credit Percentages');
$pg->head('Cash/Credit Percentages');

$ccp = new ccpercs($pg);

$act = @$_POST['act'];
$active = (isset($_GET['active'])?$_GET['active']:(isset($_POST['active'])?$_POST['active']:YES));

if ($act == "")
{
	// display percentage form
	$ccp->percs_form();
}
elseif ($act == "set")
{
	// set the percentages
	$ccp->pull_post();
	$ccp->set();

	if ($ccp->was_set())
	{
		$pg->status('Set '.count($ccp->percs).' percentages');
		$ccp->percs_form();
	}
	else
	{
		echo "ERROR SETTING PERCENTAGES! CONTACT SCOTT! (this shouldn't happen...)";
	}
}

$pg->foot();
?>