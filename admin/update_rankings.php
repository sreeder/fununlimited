<?php
include('../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setTitle('Manual Rankings Update');
$pg->head('Manual Rankings Update');

echo 'Updating rankings...<p />';
$rank = new rankings($pg);
$rank->updateRankings();
echo '<p />Done!';

$pg->foot();
?>