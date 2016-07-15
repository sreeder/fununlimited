<?php
/*
* Fun Unlimited database synchronization utility
* Runs every morning at 2am
*
* This file handles the exporting on the store end
*/
include('synch_settings.inc');
$continue = YES;
?>

Synchronizing in-store database with online database

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!                                    !!!!
!!!! DO NOT CLOSE OR CANCEL THIS WINDOW !!!!
!!!!                                    !!!!
!!!! DO NOT USE THE SOFTWARE UNTIL THIS !!!!
!!!!      PROCESS IS 100% COMPLETE      !!!!
!!!!                                    !!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

<?php
/*****************************
 * synchronize online orders *
 ****************************/

// grab all online orders for that day pulls from the last synch to now and inserts
// into the store's database, automatically incrementing and using the invoiceIDs

$sql = 'SELECT syn_last_synch FROM synch_settings';
$result = mysql_query($sql,$db);
if (!mysql_num_rows($result)) { $last_time = 0; }
else
{
	$row = mysql_fetch_assoc($result);
	$last_time = $row['syn_last_synch'];
}

echo 'Getting orders from previous synch (' . date('m/d/Y h:ia',$last_time) . ") to now...\n"; flush();

$orders_url = "http://{$server}/synch/get_orders.php?last_time={$last_time}";
$orders_output = @file($orders_url);

if ($orders_output === false)
{
	error("Unable to import latest orders: {$orders_url}!!!");
	$continue = NO;
}

if ($continue)
{
	$sqls = array();
	$columns = array(); // $columns['table'] = array(columns_in_order)
	$setif = array(
		'CREATEID' => 'NULL',
		'LASTID'   => 'last_insert_id()'
	);

	$total_orders = 0;
	while (list($a,$line) = each($orders_output))
	{
		$line = trim($line);
		if (strlen($line))
		{
			$exp = explode($delimeter,$line);
			$command = array_shift($exp);
			$table = array_shift($exp);

			if ($command == 'columns')
			{
				$columns[$table] = $exp;
			}
			elseif ($command == 'data')
			{
				// build the query
				$vals = array();
				while (list($k,$v) = each($exp))
				{
					$key = $columns[$table][$k];
					if (isset($setif[$v]))
					{
						if ($v == 'CREATEID') { $total_orders++; }
						$vals[] = "$key={$setif[$v]}";
					}
					else { $vals[] = "$key=" . (strlen($v)&&is_numeric($v) ? '' : "'") . mysql_real_escape_string($v) . (strlen($v)&&is_numeric($v) ? '' : "'"); }
				}

				$sqls[] = "INSERT INTO $table SET " . implode(',',$vals);
			}
		}
	}

	if (!$total_orders) { echo "No online orders found\n\n"; }
	else
	{
		echo "Found $total_orders total online order(s)\n";
		echo "Inserting online orders into the local database...\n";
	}
	flush();

	while (list($a,$sql) = each($sqls))
	{
		mysql_query($sql,$db);
		if (mysql_errno())
		{
			error("Error inserting online orders into database!\nTHIS IS A SERIOUS PROBLEM!!!\n\nQuery: {$sql}");
			$continue = NO;
		}
	}

	if ($continue)
	{
		if ($total_orders)
		{
			echo "Successfully inserted orders into the database!\n\n";
		}

		/************************
		 * synchronize database *
		 ***********************/

		$date = date('Mdy');
		$dump_file = "{$date}.synch.allDB.dump";
		$zip_file = "{$date}.synch.allDB.zip";

		// dump database
		set_time_limit(180);
		echo "Dumping database into $dump_file...\n"; flush();

		$mysql4 = version_compare(mysql_get_server_info(),'4.0','>=');
		if ($mysql4)
		{
			echo "Using MySQL version 4+ dump query\n"; flush();
			exec("mysqldump -ufununlimited -pfununlimited --compact --add-drop-table fununlimited > $dump_file");
		}
		else
		{
			echo "Using MySQL version 3 dump query\n"; flush();
			exec("mysqldump -ufununlimited -pfununlimited --extended-insert --add-drop-table fununlimited > $dump_file");
		}
		$dump_size = filesize($dump_file);
		echo 'Database dumped; ' . number_format($dump_size,0) . " bytes\n\n"; flush();

		/*
		// zip database
		$output = array();
		set_time_limit(60000);
		echo "Zipping database file into $zip_file...\n"; flush();
		//exec("wzzip $zip_file $dump_file",$output);
		if (count($output))
		{
			echo "Output:\n";
			print_r($output);
		}
		if (!file_exists($zip_file))
		{
			error("Unable to create $zip_file");
			$continue = NO;
		}

		if ($continue)
		{
			$zip_size = filesize($zip_file);
			echo 'Database zipped; ' . number_format($zip_size,0) . " bytes\n\n"; flush();

			// upload file
			$output = array();
			set_time_limit(60000);
			echo "Uploading $dump_file to server...\n"; flush();
			echo "This could take 5-15 minutes - PLEASE BE PATIENT!!!\n"; flush();
			//exec("ftp_file $dump_file",$output);
			if (count($output))
			{
				echo "Output:\n";
				print_r($output);
			}
			echo "File upload complete!\n\n"; flush();

			// call the script on the server that unzips and loads the database
			set_time_limit(60000);
			echo "Accessing database importer on server...\n\n";
			echo "Importer output:\n";
			echo "--------------------------------------------\n";
			flush();

			$import_url = "http://{$server}/synch/load_synch.php?date={$date}";
			$import_output = @file_get_contents($import_url);

			if ($import_output === false)
			{
				error("Unable to open import URL: {$import_url}!!!");
				$continue = NO;
			}
			*/

			// import the data into the server's database
			// THIS TAKES FOREVER - IT'S A 35MB+ FILE THAT MUST BE PASSED OVER THE INTERNET A LINE AT A TIME!!!
			set_time_limit(10800); // three hours ought to be enough!
			echo "Loading database onto server...\n"; flush();
			echo 'Started: ' . date('h:i:sa') . "\n\n"; flush();
			echo "THIS PROCESS TAKES 5-15 MINUTES!!! PLEASE BE PATIENT!!!\n"; flush();
			echo "_DO NOT_ CANCEL OR CLOSE THIS WINDOW UNTIL IT IS 100% DONE!!!\n"; flush();
			echo "IF YOU DO, THE ONLINE STORE WILL BE _VERY_ MESSED UP!!!\n\n"; flush();
			exec("mysql -hwww.fununlimitedonline.com -ufununlimited -pfununlimited fununlimited < $dump_file");
			echo "Database successfully loaded onto the server!\n"; flush();
			echo 'Finished: ' . date('h:i:sa') . "\n\n"; flush();

			if ($continue)
			{
				// save the last synch time
				$sql = 'UPDATE synch_settings SET syn_last_synch=' . time();
				mysql_query($sql,$db);
				if (!mysql_affected_rows())
				{
					$sql = 'INSERT INTO synch_settings SET syn_last_synch=' . time();
					mysql_query($sql,$db);
				}

				// synch complete
				echo "--------------------------------------------\n\n";
				echo "Synchronization complete!\n\n";
				echo "If there were ANY errors, please contact Scott Carpenter ASAP!\n";
				echo "s-carp@comcast.net -or- 435-512-9323\n";
				echo "Otherwise, press any key to close this window\n\n";
				flush();
			}
		//}
	}
}

/**
* Output an error and die
* @param string $error
*/
function error($error)
{
?>

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!
!!!! ERROR: <?=$error . "\n";?>
!!!!
!!!! CONTACT SCOTT CARPENTER ASAP!!!
!!!! s-carp@comcast.net -or- 435-512-9323
!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

<?php
}
?>