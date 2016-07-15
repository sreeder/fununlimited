<?php
include('include/include.inc');

$pg = new page();
$pg->setTitle('Welcome');
$pg->head();

$itm = new items($pg);
$itm->setItems(array(1,10,100,1000));
$items = $itm->getItems();

?><div align="left"><pre><?php print_r($items); ?></pre></div><?php

$pg->foot();
?>