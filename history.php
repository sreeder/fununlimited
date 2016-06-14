<?php
$check_login = true;
include('include/include.inc');

$pg = new page();
$cart = new cart($pg);
$pg->setTitle('Order History');
$pg->head('Order History');

?>
&nbsp;<p />&nbsp;<p />&nbsp;<p />
[ <b>Order History</b> ]
<p />&nbsp;<p />&nbsp;<p />&nbsp;
<?php

$pg->foot();
?>