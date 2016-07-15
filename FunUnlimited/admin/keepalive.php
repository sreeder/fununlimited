<?php
// keep the session alive by reloading this hidden iframe page every 3 minutes
include('../include/include.inc');

$pg = new admin_page();
$pg->setFull(NO);
$pg->setCenter(NO);
$pg->head();

if (!isset($_SESSION['keepalive_reloaded'])) { $_SESSION['keepalive_reloaded'] = -1; }
$_SESSION['keepalive_reloaded']++;

?><font color="white"><?=$_SESSION['keepalive_reloaded'];?></font><?php

?>
<script type="text/javascript">
	setTimeout('refreshPage()',180000);
	//document.body.ondblclick = refreshPage;
	function refreshPage() { document.location=document.location; }
</script>
<?php

$pg->foot();
?>