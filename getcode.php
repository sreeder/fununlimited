<?php
// File: getcode.php
// Description: given a filename, return the code for the given file
//
// This file is part of the auto-updating utilities
include('classes/update.inc');

$upd = new update();

$storeID = @$_GET['storeID'];
$md5auth = @$_GET['md5auth'];
$file = @$_GET['file'];

$upd->setStoreID($storeID);

if (!$upd->authenticate($md5auth)) { $upd->outputDownloadError(0,$file); }
elseif (!strlen($file)) { $upd->outputDownloadError(1,$file); }
elseif (!file_exists($file)) { $upd->outputDownloadError(2,$file); }
elseif (!is_readable($file)) { $upd->outputDownloadError(3,$file); }
else
{
	// output the file's contents
	$read = file($file);
	for ($i=0; $i<count($read); $i++) { $read[$i] = trim($read[$i],"\n"); }
	echo implode("\n",$read);
}

?>