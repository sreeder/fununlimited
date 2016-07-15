<?php
/*
* Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include('../../include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$return = @$_POST['return'];
if (!strlen($return)) { $return = '/admin/pos/pos.php'; }

$pg = new admin_page();
$cus = new customers($pg);

if ($act == "add")
{
	// add note to customer
	$cus->addNote();
}
elseif ($act == "delete")
{
	// delete a note from the customer
	$cus->deleteNote(@$_GET['noteID']);
}

$pg->showUpdating('Updating Notes...',$return);
?>