<?php
include('../include/include.inc');

$cl = new check_login();

$pg = new admin_page();
$pg->setTitle('Store Management');
$pg->head('Store Management');

?>
Please select a function from the left menu.
<?php

$pg->foot();
?>