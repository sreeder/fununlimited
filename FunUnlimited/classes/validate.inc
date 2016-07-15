<?php
/**
* Validation of various things (phone numbers, credit card numbers, etc)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class validate
{
	/**
	* Validate a phone number - a valid phone # has either 10 or 11 numbers; if it has 11 numbers, the first one must be a 1
	* @param string $number;
	* @return integer YES/NO
	*/
	function phone($number)
	{
		$val = $this->strip($number);
		if (strlen($val) == 10) { return YES; }
		if (strlen($val) == 11)
		{
			if (substr($val,0,1) == 1) { return YES; } else { return NO; }
		}
	}

	/**
	* Validate a zip code - a valid zip code has 5 digits, possibly followed by 4 digits
	* @param string $zip
	* @return integer YES/NO
	*/
	function zip_code($zip)
	{
		$val = $this->strip($zip);
		if (strlen($val) == 5 || strlen($val) == 9) { return YES; }
		else { return NO; }
	}

	/*
	* Validate an e-mail address - a valid e-mail address has the format: user@host.ext where ext must be 2-4 characters
	* @param string $email
	* @return integer YES/NO
	*/
	function email($email)
	{
		if (preg_match("^[a-z0-9\._-]+@+[a-z0-9\._-]+\.+[a-z]{2,4}$",$email)) { return YES; }
		else { return NO; }
	}

	/**
	* Strips everything but numbers from a string and returns the new string
	* @param string $string
	* @return string Stripped value
	*/
	function strip($string) { return preg_replace('([^0-9]*)','',$string); }
}
?>
