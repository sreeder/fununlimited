<?php
// update the rankings
include('../classes/error.inc');
include('../classes/database.inc');
include('../classes/flags.inc');
include('../classes/admin_page.inc');
include('../classes/rankings.inc');

$pg = new admin_page();
$rnk = new rankings($pg);
$rnk->updateRankings(0,1);
?>