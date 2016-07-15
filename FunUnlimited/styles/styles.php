<?php
include('../include/include.inc');

header("Content-type: text/css");

$pg = new admin_page();
$font = "Verdana,Helvetica";
?>

body { font-family:<?=$font;?>;font-size:12;color:<?=$pg->color('text');?>; }
a { color:<?=$pg->color('link');?>;text-decoration:none; }
a:hover { color:<?=$pg->color('link-hover');?>;text-decoration:underline; }
.eight { font-family:<?=$font;?>;font-size:8; }
.nine { font-family:<?=$font;?>;font-size:9; }
.ten { font-family:<?=$font;?>;font-size:10; }
.eleven { font-family:<?=$font;?>;font-size:11; }
.normal { font-family:<?=$font;?>;font-size:12; }
.thirteen { font-family:<?=$font;?>;font-size:13; }
.fourteen { font-family:<?=$font;?>;font-size:14; }
.menu { font-family:<?=$font;?>;font-size:10;font-weight:bold;line-height:12pt;padding:0px 2px 0px 2px;color:<?=$pg->color('menu-text');?>; }
.pagebottom { font-family:<?=$font;?>;font-size:9;color:#777777; }
a.menu { color:<?=$pg->color('menu-link');?>;text-decoration:none; }
a.menu:hover { color:<?=$pg->color('menu-link-hover');?>;text-decoration:none; }
a.mainmenu { color:<?=$pg->color('menu-link');?>;text-decoration:none; }
a.mainmenu:hover { color:<?=$pg->color('menu-link-hover');?>;text-decoration:underline; }
a.graylink { color:#555555;text-decoration:underline; }
a.graylink:hover { color:#555555;text-decoration:none; }
table tr td { white-space:nowrap;font-family:<?=$font;?>;font-size:9; }
input { font-family:<?=$font;?>;font-size:9;border:solid 1px #999999;vertical-align:middle; }
input.nb { border-width:0px; }
textarea { font-family:<?=$font;?>;font-size:9;border:solid 1px #999999; }
.radio { font-family:<?=$font;?>;font-size:9; }
.btn { cursor:hand;background:url(/images/btnback.jpg);background-color:#EEEEEE;margin-bottom:1px; }
.smbtn { cursor:hand;background:url(/images/btnback.jpg);font-size:9;vertical-align:middle;background-color:#EEEEEE; }
select { font-family:<?=$font;?>;font-size:9;vertical-align:middle; }
.optbox { color:#000000;font-weight:bold;text-decoration:none; }
.optbox:hover { text-decoration:none; }
.medgray { color:#AAAAAA; }
form { display:inline; }
label { cursor:hand; }
.bb { border:solid 1px black;border-collapse:collapse;padding:3px; }

tr.bold td
{
	font-weight:bold;
}

