<?php
include('../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setTitle('Archive Rankings');
$pg->head($pg->_getTitle());

// change the number below to the year you would like to archive
$year = getG('year', 0);

$rank = new rankings($pg);
$rank->archiveRankings($year);

$referrer = $_SERVER['HTTP_REFERER'];
headerLocation($referrer);

$pg->foot();
?>