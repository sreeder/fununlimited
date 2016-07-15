<?php
include('../../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setTitle('Maximum Copies');
$pg->head('Maximum Copies');

$mc = new max_copies($pg);

$act = @$_POST['act'];
$active = (isset($_GET['active'])?$_GET['active']:(isset($_POST['active'])?$_POST['active']:YES));

if ($act == "")
{
	// display maximum copies form
	$mc->maxcopies_form();
}
elseif ($act == "set")
{
	// set the maximum copies
	$mc->pull_post();
	$mc->set();

	if ($mc->was_set())
	{
		$pg->status('Set '.count($mc->copies).' maximum copies');
		$mc->maxcopies_form();
	}
	else
	{
		echo "ERROR SETTING PRICES! CONTACT SCOTT! (this shouldn't happen...)";
	}
}

$pg->foot();
?>