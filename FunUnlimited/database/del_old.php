<?php
// delete old dump files
for ($i=2; $i<7; $i++)
{
	$date = date('D',strtotime("-$i days"));
	`del {$date}*.dump`;
}
?>