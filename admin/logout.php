<?php
include('../include/include.inc');

/*
$keep = array('root','root_admin','storeID');
$vals = array();

while (list($a,$key) = each($keep)) { $vals[$key] = @$_SESSION[$key]; }
session_destroy();
session_start();
while (list($key,$val) = each($vals)) { $_SESSION[$key] = $val; }
*/

$_SESSION['loggedin'] = NO;
$_SESSION['cust_invoiceID'] = 0;

$pg = new admin_page();
$pg->setTitle('Log Out');
$pg->head('Log Out');

?>
You have been logged out...
<p />
<a href="login.php">Log In</a>

<script type="text/javascript">document.location='login.php'</script>
<?php

$pg->foot();
?>