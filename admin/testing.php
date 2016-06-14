<?php
include('../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setTitle('Store Management');
$pg->head('Store Management');

$rank = new rankings($pg);
$rank->updateRankings();

$pg->foot();
?>