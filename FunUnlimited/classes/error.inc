<?php
/**
* Error output/logging
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class error
{
	/**
	* The current "system" the error class is reporting
	* @var string $system
	*/
	var $system;

	/**
	* Class constructor
	* Initializes variables and sets the system
	* @param string $system [optional, default '']
	*/
	function error($system="")
	{
		$this->text = "";
		$this->system = $system;
	}

	/**
	* Output/log the error
	* @param string $text
	* @param integer $die Die after displaying the error? (YES/NO) [optional, default "YES"]
	*/
	function show($text,$die=YES)
	{
		global $db;

		if ($this->system == "") { $sys = ""; }
		else { $sys = "$this->system "; }

		$pg = new admin_page();
		$pg->error($text,$sys);

		if ($die == YES)
		{
			echo "\n<p />\nExiting...\n";
			die();
		}
	}

	/**
	* Checks for a mySQL error; if there is one, report it
	*/
	function mysql($file,$line,$return_if_duplicate=NO,$sql="")
	{
		if (mysql_errno())
		{
			$show_error = YES;
			if ($return_if_duplicate == YES && strtolower(substr(mysql_error(),0,9)) == "duplicate")
			{
				$show_error = NO; return YES;
			}

			if ($show_error == YES) { $this->show('mySQL Error in <u>'.basename($file).'</u> on line <u>'.$line.'</u>:<p />'.mysql_error().(strlen($sql)?"<p />$sql":'')); }
		}

		return NO;
	}
}