<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Barebones Utilities');
$pg->head('Barebones Utilities');

?>
<b>Select a Utility Below:</b>
<p />
<a href="items.php">Item Information Lookup (Pricing/Quantity)</a>
<p />
<a href="tradecust.php">Trade Sheet Customer Entry</a>
<p />
<input type="button" value="&lt; Return to Store Management" onclick="document.location='/admin/index.php'" class="btn" />
<?php

$pg->foot();
?>