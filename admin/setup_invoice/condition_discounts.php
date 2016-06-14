<?php
include('../../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setTitle('Condition Discounts');
$pg->head('Condition Discounts');

$cod = new condition_discounts($pg);

$act = @$_POST['act'];
$active = (isset($_GET['active'])?$_GET['active']:(isset($_POST['active'])?$_POST['active']:YES));

if ($act == "")
{
	// display percentage form
	$cod->discounts_form();
}
elseif ($act == "set")
{
	// set the discounts
	$cod->pull_post();
	$cod->set();

	if ($cod->was_set())
	{
		$pg->status('Set '.count($cod->discounts).' discounts');
		$cod->discounts_form();
	}
	else
	{
		echo "ERROR SETTING DISCOUNTS! CONTACT SCOTT! (this shouldn't happen...)";
	}
}

$pg->foot();
?>