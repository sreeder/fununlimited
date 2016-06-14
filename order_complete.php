<?php
/*
* Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
$check_login = true;
include('include/include.inc');

$pg = new page();
$pg->setTitle('Order Complete');
$pg->head('Order Complete');

?>[ order complete ]<?php

?><div align="left"><pre><?php print_r($_SESSION['transaction_data']); ?></pre></div><?php

$pg->foot();
?>