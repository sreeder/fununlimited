<?php
include('../../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Generate Order - Printable');
$pg->head();

$error = new error('Generate Order Printable');

$ord = new order($pg);
$ord->printable_list();

$pg->foot();
?>