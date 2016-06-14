<?php
/**
* Database resource identifier
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

if (!defined('PRODUCTION'))
{
	// this generally happens if you only include this file
	if (!isset($force_production) || !$force_production)
	{
		define('PRODUCTION',(@$_SERVER['CLIENTNAME']=='Console' || @$_SERVER['SERVER_NAME']=='fununlimitedonlinedev' ? false : true));
	}
	else
	{
		define('PRODUCTION',true);
	}
}

$host = (PRODUCTION ? '206.130.100.155' : 'localhost');
$host = 'localhost';

$db = @mysql_connect($host,'fununlimited','fununlimited');
if (!$db) { die('Unable to connect to the database: ' . mysql_error()); }
mysql_query('USE fununlimited',$db);