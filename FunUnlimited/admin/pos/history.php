<?php
include('../../include/include.inc');

$cl = new check_login();

$cust = new customers($pg);

$pg = new admin_page();
$pg->setTitle('Customer History');
$pg->head('Customer History',YES);

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$quick = @$_GET['quick'];
if (strlen($quick))
{
	$act = "search";
	// DEFINE SEARCH CRITERIA HERE
}

if ($act == "")
{
	// display the history search form
	?>
	This feature is not yet available.
	<?php
}
elseif ($act == "search")
{
	// perform the history search and display the results
	?>
	[ history search results ]
	<p />
	quick form: <?=$quick;?>
	<?php
}

$pg->foot();
?>