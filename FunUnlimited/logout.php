<?php
include('include/include.inc');

$_SESSION['store_loggedin'] = NO;

$pg = new page();
$pg->showUpdating('Logging you out...','/index.php');
?>