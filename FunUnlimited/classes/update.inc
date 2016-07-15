<?php
/**
* Auto-updating class
* NOTE: this file can be included by itself!
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class update
{
	/**
	* Database connection
	* @var object $db
	*/
	var $db;

	/**
	* Current storeID
	* @var integer $storeID
	*/
	var $storeID;

	/**
	* MD5 hash/authorization string
	* @var string $md5auth
	*/
	var $md5auth;

	/**
	* File download errors
	* @var array $errors
	*/
	var $errors;

	/**
	* Updates
	* @var array $updates
	*/
	var $updates;

	/**
	* Location of code-fetching script
	* @var string $getcode_location
	*/
	var $getcode_location;

	/**
	* Software location
	* @var string $software_directory
	*/
	var $software_directory;

	/**
	* Class constructor
	* Initializes variables and connects to the database
	*/
	function update()
	{
		$this->storeID = 0;
		$this->errors = array(
			0=>'Store authentication failed',
			1=>'No file name given',
			2=>'File does not exist',
			3=>'File not readable'
		);
		$this->getcode_location = 'http://206.130.100.155/getcode.php';

		if (!defined('YES')) { define('YES',1); }
		if (!defined('NO')) { define('NO',0); }
		if (!defined('ENCODEKEY')) { define('ENCODEKEY','FunUnlmtd'); }
		$this->software_directory = 'c:/webroot/FunUnlimited';
		chdir($this->software_directory);

		$production = ($_SERVER['SERVER_ADDR']=='127.0.0.1'?false:true);
		$host = ($production ? '206.130.100.155' : 'localhost');
		$host = 'localhost';
		$this->db = mysql_connect($host,'fununlimited','fununlimited') or die('Unable to connect to database: ' . mysql_error());
		mysql_query('USE fununlimited',$this->db) or die('Unable to select fununlimited database');

		// set $php_errormsg when an error occurs
		ini_set('track_errors','On');
	}

	/**
	* Run an SQL query and return the result object (or die if there is an error)
	* @param string $sql
	* @return object
	*/
	function runQuery($sql)
	{
		$result = mysql_query($sql,$this->db);
		if (mysql_errno()) { die('<p /><b>MySQL Error:</b><br />'.mysql_error()."<p /><b>Query:</b><br />$sql"); }
		return $result;
	}

	/**
	* Set the storeID
	* @param integer $storeID
	*/
	function setStoreID($storeID)
	{
		$this->storeID = $storeID;

		$sql = "SELECT MD5(CONCAT(use_user,DECODE(use_pass,'".ENCODEKEY."'))) AS md5auth FROM users WHERE use_storeID=$storeID";
		$result = $this->runQuery($sql);
		$row = mysql_fetch_assoc($result);
		$this->md5auth = $row['md5auth'];
	}

	/**
	* Get the storeID
	* @return integer
	*/
	function getStoreID() { return $this->storeID; }

	/**
	* Authenticate a store and it's md5 key: md5(user.pass)
	* @param string $md5auth
	* @return integer
	*/
	function authenticate($md5auth)
	{
		global $db;

		$auth = NO;

		if (strlen($md5auth))
		{
			$sql = "SELECT COUNT(*) AS count FROM users WHERE use_storeID=$this->storeID AND MD5(CONCAT(use_user,DECODE(use_pass,'".ENCODEKEY."')))='$md5auth'";
			$result = $this->runQuery($sql);
			$row = mysql_fetch_assoc($result);

			$auth = ($row['count']?YES:NO);
		}

		return $auth;
	}

	/**
	* Pull in updates
	* @param integer $complete [optional, default NO]
	* @param integer/array $updateIDs array of updateIDs (can be array or single updateID) [optional, default array()]
	*/
	function setUpdates($complete=NO,$updateIDs=array())
	{
		global $db;

		if (!is_array($updateIDs)) { $updateIDs = array($updateIDs); }

		if (count($updateIDs)) { $updatewhere = " AND upd_updateID IN (".implode(',',$updateIDs).")"; } else { $updatewhere = ""; }
		if ($complete == BOTH || strlen($updatewhere)) { $complete = array(YES,NO); } else { $complete = array($complete); }

		$this->updates = array();
		$sql = "SELECT * FROM updates WHERE upd_complete IN (".implode(',',$complete).") AND upd_active=" . YES . "{$updatewhere} ORDER BY upd_time DESC";
		$result = $this->runQuery($sql);

		while ($row = mysql_fetch_assoc($result))
		{
			$updateID = $row['upd_updateID'];

			$row['files'] = array();
			$fsql = "SELECT * FROM update_files WHERE upf_updateID=$updateID";
			$fresult = $this->runQuery($fsql);
			while ($frow = mysql_fetch_assoc($fresult)) { $row['files'][] = $frow; }

			$row['sql'] = array();
			$ssql = "SELECT * FROM update_sql WHERE ups_updateID=$updateID ORDER BY ups_order";
			$sresult = $this->runQuery($ssql);
			while ($srow = mysql_fetch_assoc($sresult)) { $row['sql'][] = $srow; }

			$row['completed_stores'] = array();
			$csql = "SELECT * FROM update_completed_stores WHERE ucs_updateID=$updateID";
			$cresult = $this->runQuery($csql);
			while ($crow = mysql_fetch_assoc($cresult)) { $row['completed_stores'][] = $crow['ucs_storeID']; }

			$this->updates[$row['upd_updateID']] = $row;
		}
	}

	/**
	* Retrieve one/all updates
	* @param integer $updateID updateID of the update to return [optional, default 0]
	*/
	function getUpdates($updateID=0)
	{
		if (isset($this->updates[$updateID])) { return $this->updates[$updateID]; }
		else { return $this->updates; }
	}

	/**
	* Return any unapplied updateIDs for the current store
	* @return array
	*/
	function getUnappliedUpdateIDs()
	{
		$updates = $this->getUpdates();
		$updateIDs = array();

		while (list($updateID,$arr) = each($updates))
		{
			if (!in_array($_SESSION['storeID'],$arr['completed_stores'])) { $updateIDs[] = $updateID; }
		}

		return $updateIDs;
	}

	/**
	* Download files, run SQL queries, and set update as applied for the current store
	* @param integer $updateID
	*/
	function applyUpdate($updateID)
	{
		global $db;

		$this->setUpdates(BOTH,$updateID);
		$info = $this->getUpdates($updateID);

		if (!count($info)) { uecho("Invalid updateID: $updateID"); }
		elseif (in_array($this->storeID,$info['completed_stores'])) { uecho("The provided update has already been applied to the current store!"); }
		else
		{
			uecho("<b>Applying update #$updateID...</b>");
			uecho("Update description: {$info['upd_description']}");
			uecho("Update creation date/time: ".date('m/d/Y h:ia',$info['upd_time']));

			// parse out SQL queries
			$sqls = array();
			while (list($a,$arr) = each($info['sql'])) { $sqls[] = $arr['ups_sql']; }
			$this->executeQueries($sqls);

			// parse out filenames
			$files = array();
			while (list($a,$arr) = each($info['files'])) { $files[] = $arr['upf_file']; }
			$this->downloadFiles($files);

			$this->addToCompletedStores($updateID,$_SESSION['storeID']);
		}
	}

	/**
	* Execute SQL queries
	* @param array $sqls
	*/
	function executeQueries($sqls)
	{
		// !!! NEED TO PROVIDE WAY TO ROLL THIS BACK !!!
		// !!! ups_rollback_sql field? !!!
		if (count($sqls))
		{
			uecho("Executing ".count($sqls)." SQL quer".(count($sqls)!=1?'ies':'y')."...");
			while (list($a,$sql) = each($sqls))
			{
				set_time_limit(60);
				$this->runQuery($sql);
			}
			uecho("Done executing SQL queries!");
		}
	}

	/**
	* Download and replace the given files
	* @param array $files
	* @return integer YES/NO if all files were downloaded/moved
	*/
	function downloadFiles($files)
	{
		if (count($files))
		{
			$contents = array(); // format: $contents[idx] = <file_content>

			// build list of URLs to retrieve
			$sendURLs = array();
			while (list($a,$file) = each($files)) { $sendURLs[] = "$this->getcode_location?storeID={$this->storeID}&md5auth={$this->md5auth}&file=".urlencode($file); }
			reset($files);

			// retrieve code from URLS
			uecho("Downloading updated source files...");
			while (list($a,$url) = each($sendURLs))
			{
				set_time_limit(300);
				uecho("Downloading: {$files[$a]}");

				$read = file_get_contents($url);
				if ($this->isDownloadErrorCode($read))
				{
					$this->showErrorDie("downloadFiles: File Download Error:<p />".nl2br($read),$php_errormsg);
				}

				$contents[] = $read;
				$size = strlen($read);
				uecho("Done; downloaded ".number_format($size,0)." bytes");
			}
			uecho("All files downloaded!");

			// back up current files
			uecho("Backing up existing copies of all files...");
			while (list($a,$file) = each($files))
			{
				set_time_limit(60);
				$copied = @copy($file,"{$file}.original");
				if (!$copied) { $this->showErrorDie("downloadFiles: Unable to copy $file to {$file}.original",$php_errormsg); }
			}
			reset($files);
			uecho("Existing copies of all files backed up!");

			// overwrite current files with downloaded files
			uecho("Overwriting current files with updated files...");
			while (list($a,$file) = each($files))
			{
				set_time_limit(60);
				$write = $contents[$a];
				$f = @fopen($file,'w');
				if (!$f)
				{
					$this->rollBackFiles($files);
					$this->showErrorDie("downloadFiles: Unable to open $file for writing!",$php_errormsg);
				}
				else
				{
					fwrite($f,$write);
					fclose($f);
					uecho("Wrote file: $file");
				}
			}
			reset($files);
			uecho("Overwrote current files with updated files!");

			$this->cleanFiles();
		}
	}

	/**
	* Roll back files to originals
	* @param array $files
	*/
	function rollBackFiles($files)
	{
		uecho("Rolling back all files to originals...");
		while (list($a,$file) = each($files))
		{
			set_time_limit(60);
			$original = "{$file}.original";
			if (file_exists($original))
			{
				$copied = @copy($original,$file);
				if (!$copied) { $this->showErrorDie("rollBackFiles: Unable to copy $file to {$file}.original",$php_errormsg); }
			}
		}
		uecho("All files rolled back to originals");
	}

	/**
	* Delete all .original files in the entire software tree structure
	*/
	function cleanFiles()
	{
		uecho("Cleaning up temporary files...");

		$delete = $this->rglob($this->software_directory,'*.original');
		while (list($a,$file) = each($delete)) { @unlink($file); }

		uecho("Temporary files cleaned up!");
	}

	/**
	* Recursive version of glob()
	* @param string $dir base directory
	* @param string $pattern file pattern (IE: "*.txt")
	* @param int $flags glob() flags
	* @return array containing all pattern-matched files
	*/
	function rglob($dir,$pattern,$flags=NULL)
	{
		set_time_limit(60);
		$dir = escapeshellcmd($dir);
		$files = glob("$dir/$pattern",$flags);

		foreach (glob("$dir/*",GLOB_ONLYDIR) as $subdir)
		{
			$subfiles = $this->rglob($subdir,$pattern,$flags);
			$files = array_merge($files,$subfiles);
		}

		// weed out the empty array values
		if (!is_array($files)) { $files = array(); }
		$new_files = array();
		while (list($a,$file) = each($files))
		{
			if (strlen($file)) { $new_files[] = $file; }
		}
		$files = $new_files;

		return $files;
	}

	/**
	* Output an error code and it's corresponding text
	* @param integer $code
	*/
	function outputDownloadError($code,$file) { echo "getcode_error\n$code\n".@$this->errors[$code]."\n$file"; }

	/**
	* Returns YES/NO if the code is an error report
	* @param string $contents
	* @return integer
	*/
	function isDownloadErrorCode($contents)
	{
		$lines = explode("\n",$contents);
		if ($lines[0] == "getcode_error") { return YES; }
		else { return NO; }
	}

	/**
	* Add a storeID to the given update's completed store list; if all stores have been added, set the update as completed
	* @param integer $updateID
	* @param integer $storeID
	*/
	function addToCompletedStores($updateID,$storeID)
	{
		global $db;

		$this->setUpdates(BOTH,$updateID);
		$info = $this->getUpdates($updateID);

		if (count($info) && !in_array($this->storeID,$info['completed_stores']))
		{
			// insert row into update_completed_stores table
			$sql = "INSERT INTO update_completed_stores VALUES ($updateID,$storeID)";
			$this->runQuery($sql);
			$info['completed_stores'][] = $storeID;
			uecho("Update #$updateID completed!<br />&nbsp;");

			// see if all storeIDs have been updated
			$sql = "SELECT COUNT(*) AS count FROM stores WHERE sto_storeID NOT IN (".implode(',',$info['completed_stores']).")";
			$result = $this->runQuery($sql);
			$row = mysql_fetch_assoc($result);
			$count = $row['count'];

			if (!$count)
			{
				// set the update as completed
				$sql = "UPDATE updates SET upd_complete=" . YES . " WHERE upd_updateID=$updateID";
				$this->runQuery($sql);
			}
		}
	}

	/**
	* Output administrator contact information and die
	* @param string $text error text
	* @param integer $email email error to Scott Carpenter (scott@payforstay.com)? [optional, default YES]
	*/
	function showErrorDie($text,$phperr,$email=YES)
	{
		echo $text;
		?>
		<p />
		<hr width="100%" size="-1" color="#000000" />
		<p />
		Please note the above error(s) and contact <a href="mailto:scott@payforstay.com">Scott Carpenter (scott@payforstay.com)</a> as soon as possible!
		<?php

		if ($email)
		{
			$body = "Date/Time: ".date('m/d/Y h:ia')."\n";
			$body .= "StoreID: $this->storeID\n";
			$body .= "Error:\n$text\n\n";
			$body .= "Last PHP Error Message:\n$phperr\n\n";
			$body .= "\$GLOBALS contents:\n".print_r($GLOBALS,true);
			$sent = @mail('scott@payforstay.com','FunUnlimited Auto-Update Error!',wordwrap($body,72));

			echo "<p /><b>\$php_errormsg: $phperr</b><p />";

			if (!$sent) { echo "<p />An attempt was made to send an email to Scott Carpenter, but it failed."; }
			else { echo "<p />An email was successfully sent to Scott Carpenter"; }
		}

		die();
	}
}

function uecho($text) { echo "� $text<br />"; flush(); }
?>