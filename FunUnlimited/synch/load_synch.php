<?php
/*
* Fun Unlimited database synchronization utility
* Runs every morning at 2am
*
* This file handles the importing on the server end
*/
include('synch_settings.inc');

$date = @$_GET['date'];
$dump_file = "{$date}.synch.allDB.dump";
$zip_file = "{$date}.synch.allDB.zip";

if (!strlen($date)) { echo "ERROR: No date provided!\n"; }
//elseif (!file_exists($zip_file)) { echo "ERROR: {$zip_file} does not exist!\n"; }
elseif (!file_exists($dump_file)) { echo "ERROR: {$dump_file} does not exist!\n"; }
else
{
	// unzip the synch file
	/*
	$output = array();
	set_time_limit(180);
	echo "Unzipping database file $zip_file...\n"; flush();
	$unzip = (PRODUCTION ? 'gunzip' : 'wzunzip.exe -o');
	$unzip_cmd = "{$unzip} $zip_file";
	echo "Command: $unzip_cmd\n"; flush();
	exec($unzip_cmd,$output);
	?><div align="left"><pre><?php print_r($output); ?></pre></div><?php
	$dump_size = filesize($dump_file);
	echo 'Database dump file unzipped; dump file is '.number_format($dump_size,0)." bytes\n\n"; flush();

	// import the database file
	$output = array();
	set_time_limit(3000);
	echo "Importing database file $dump_file...\n"; flush();
	exec("mysql -ufununlimited -pfununlimited fununlimited < $dump_file",$output);
	if (count($output))
	{
		echo "Output:\n";
		print_r($output);
	}

	echo "Database file successfully imported!\n"; flush();

	// delete the database file
	@unlink($dump_file);
	*/
}

echo "\n";
?>