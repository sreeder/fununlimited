<?php
include('../../include/include.inc');

$cl = new check_login();

$cust = new customers($pg);

$pg = new admin_page();
$pg->setTitle('Point-of-Sale Reports');
$pg->head('Point-of-Sale Reports',YES);

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

if ($act == "")
{
	// display the report selection form
	?>
	This feature is not yet available.
	<?php
}

$pg->foot();
?>