<?php
include('../include/include.inc');

$cl = new check_login(STORE);

$pg = new admin_page();
$pg->setFull(NO);
$pg->setCenter(NO);
$pg->setTitle('Updating Software');
$pg->head('Updating Software');

$error = new error('Updating Software');
$upd = new update($pg);
$upd->setStoreID($_SESSION['storeID']);

uecho("Checking for unapplied updates...");
$upd->setUpdates(NO);
$updateIDs = $upd->getUnappliedUpdateIDs();

if (!count($updateIDs)) { uecho("None found!"); }
else
{
	uecho("Found ".count($updateIDs)." unapplied update".(count($updateIDs)!=1?'s':'').'<br />');

	// loop through and apply each update
	while (list($a,$updateID) = each($updateIDs)) { $upd->applyUpdate($updateID); }
}

?>
<p />
<input type="button" value="&lt; Return to Previous Page" onclick="history.go(-1)" />
<?php

$pg->foot();
?>