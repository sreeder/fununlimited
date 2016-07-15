<?php
include('../../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setTitle('Per-Copy Discounts');
$pg->head('Per-Copy Discounts');

$pcd = new percopy_discounts($pg);

$act = @$_POST['act'];
$active = (isset($_GET['active'])?$_GET['active']:(isset($_POST['active'])?$_POST['active']:YES));

if ($act == '')
{
	// display percentage form
	$pcd->discounts_form();
}
elseif ($act == "set")
{
	// set the discounts
	$pcd->pull_post();
	$pcd->set();

	if ($pcd->was_set())
	{
		$pg->status('Set '.count($pcd->discounts).' discounts');
		$pcd->discounts_form();
	}
	else
	{
		echo "ERROR SETTING DISCOUNTS! CONTACT SCOTT! (this shouldn't happen...)";
	}
}

$pg->foot();
?>