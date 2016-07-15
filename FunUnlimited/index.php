<?php
include('include/include.inc');

if (PRODUCTION) { $host = 'fununlimitedonline'; } // production - redirect stores to their own server
else { $host = 'fununlimitedonlinedev'; } // development

//header("Location: http://$host/admin/index.php");

if (1)
{
	$pg = new page();
	$pg->setTitle('Welcome');
	$pg->head();

	?>
	[ <b>Home</b> ]
	<p />
	<?php

	if (0)
	{
		?><div align="left"><pre><?=print_r($_SESSION);?></pre></div><?php
	}

	$pg->foot();
}
?>