<?php
/*
* Fun Unlimited database synchronization utility
* Runs every morning at 2am
*
* This file opens the database connection and sets any necessary vars
*/

// connect to the database
include('../classes/database.inc');
include('../classes/flags.inc');

$server = (PRODUCTION ? 'www.fununlimitedonline.com' : 'fununlimitedonline');
$delimeter = '||';
?>