<?php
include('../../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setTitle('Box Discounts');
$pg->head('Box Discounts');

$bod = new box_discounts($pg);

$act = @$_POST['act'];
$active = (isset($_GET['active'])?$_GET['active']:(isset($_POST['active'])?$_POST['active']:YES));

if ($act == "")
{
	// display percentage form
	$bod->discounts_form();
}
elseif ($act == "set")
{
	// set the discounts
	$bod->pull_post();
	$bod->set();

	if ($bod->was_set())
	{
		$pg->status('Set '.count($bod->discounts).' discounts');
		$bod->discounts_form();
	}
	else
	{
		echo "ERROR SETTING DISCOUNTS! CONTACT SCOTT! (this shouldn't happen...)";
	}
}

$pg->foot();
?>