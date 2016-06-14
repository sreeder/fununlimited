<?php
$check_login = true;
include('include/include.inc');

$pg = new page();
$pg->setTitle('Account Information');
$pg->head('Account Information');

// if applicable, show a status
if (getG('saved')) { $pg->status('Your account information has been saved'); }

?>
<a href="/edit_info.php">Edit Account Information</a>
<p />
<?php

?><div align="left"><pre><?php print_r($_SESSION['userinfo']); ?></pre></div><?php

$pg->foot();
?>