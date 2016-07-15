<?php
include('include/include.inc');

$pg = new page();
$cart = new cart($pg);
$pg->setTitle('Policies');
$pg->head('Policies');

?>
&nbsp;<p />&nbsp;<p />&nbsp;<p />
[ <b>Return Policy / Privacy Policy</b> ]
<p />&nbsp;<p />&nbsp;<p />&nbsp;
<?php

$pg->foot();
?>