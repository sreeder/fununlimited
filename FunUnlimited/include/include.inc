<?php
/**
* Main include file - initializes session and includes all other include files
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 10/05/2012
* @package Fun Unlimited
*/


/************************
 ** INITIALIZE SESSION **
 ***********************/





/*



// ----------------------------------------------------------------------------------------------------
// - Display Errors
// ----------------------------------------------------------------------------------------------------
ini_set('display_errors', 'On');
ini_set('html_errors', 0);

// ----------------------------------------------------------------------------------------------------
// - Error Reporting
// ----------------------------------------------------------------------------------------------------
error_reporting(-1);

// ----------------------------------------------------------------------------------------------------
// - Shutdown Handler
// ----------------------------------------------------------------------------------------------------
function ShutdownHandler()
{
    if(@is_array($error = @error_get_last()))
    {
        return(@call_user_func_array('ErrorHandler', $error));
    };

    return(TRUE);
};

register_shutdown_function('ShutdownHandler');

// ----------------------------------------------------------------------------------------------------
// - Error Handler
// ----------------------------------------------------------------------------------------------------
function ErrorHandler($type, $message, $file, $line)
{
    $_ERRORS = Array(
        0x0001 => 'E_ERROR',
        0x0002 => 'E_WARNING',
        0x0004 => 'E_PARSE',
        0x0008 => 'E_NOTICE',
        0x0010 => 'E_CORE_ERROR',
        0x0020 => 'E_CORE_WARNING',
        0x0040 => 'E_COMPILE_ERROR',
        0x0080 => 'E_COMPILE_WARNING',
        0x0100 => 'E_USER_ERROR',
        0x0200 => 'E_USER_WARNING',
        0x0400 => 'E_USER_NOTICE',
        0x0800 => 'E_STRICT',
        0x1000 => 'E_RECOVERABLE_ERROR',
        0x2000 => 'E_DEPRECATED',
        0x4000 => 'E_USER_DEPRECATED'
    );

    if(!@is_string($name = @array_search($type, @array_flip($_ERRORS))))
    {
        $name = 'E_UNKNOWN';
    };

    return(print(@sprintf("%s Error in file \xBB%s\xAB at line %d: %s\n", $name, @basename($file), $line, $message)));
};

$old_error_handler = set_error_handler("ErrorHandler");

*/


date_default_timezone_set('America/Los_Angeles');





ob_start('ob_gzhandler'); // start the output buffer
/*
if (strpos($_SERVER['HTTP_ACCEPT_ENCODING'],'gzip') !== false)
{
	// tell the browser the content is compressed with gzip (!!! LOOK UP MEMORY LEAK !!!)
	header("Content-Encoding: gzip");
}
*/
session_start();

/**************
 ** CLEAN UP **
 **************/

if (strpos($_SERVER['PHP_SELF'], 'summary') === false)
{
	unset($_SESSION['summary_data']);
	unset($_SESSION['summary_range']);
}

/*********************************
 ** CHECK IN STORE/ONLINE STORE **
 *********************************/

// if the user is viewing the admin page and just viewed the store (or vice-versa), clear the session
if (!preg_match('/\/scripts\//',$_SERVER['PHP_SELF']) && !preg_match('/\/styles\//',$_SERVER['PHP_SELF']))
{
	$onadmin = (preg_match('/\/admin\//',$_SERVER['PHP_SELF']) || preg_match('/\/database\//',$_SERVER['PHP_SELF']));

	if (($onadmin && isset($_SESSION['viewed_store'])) || (!$onadmin && isset($_SESSION['viewed_admin'])))
	{
		//echo 'DELETE ' . ($onadmin ? 'ADMIN' : 'STORE') . ' SESSION<p />';
		session_destroy();
		while (list($k,$v) = each($_SESSION)) { unset($_SESSION[$k]); }
		session_start();
	}

	define('ONADMIN',$onadmin);
	if ($onadmin) { $_SESSION['viewed_admin'] = true; }
	else { $_SESSION['viewed_store'] = true; }
}
$_SESSION['start_time'] = micro_time();

// is this the production or development server?
$production = ($_SERVER['SERVER_NAME']=='fununlimitedonline' ? false : true);
define('PRODUCTION',$production);
define('ENCODEKEY','FunUnlmtd');

error_reporting(E_ALL);

if (defined('E_DEPRECATED'))
{
	error_reporting(E_ALL ^ E_DEPRECATED);
}

if (getenv('COMPUTERNAME') == 'SPIRO')
{
	//error_reporting(0);
}

/***************************************
 ** SET PATHS AND INCLUDE CLASS FILES **
 ***************************************/

// find the path to root from the current directory
$url = strtolower($_SERVER['PHP_SELF']);
$root = '';
if (substr($url,0,1) == '/') { $url = substr($url,1); }
$root_num = substr_count($url,'/');//# - 1;
for ($i=1; $i<=$root_num; $i++) { $root .= '../'; }

$_SESSION['root'] = $root;
$_SESSION['root_admin'] = "/admin/";

// include all .inc files in /classes/
$classdir = "{$root}classes/";
$include = array();
$include_last = array(
	'invoice_payment.inc',
	'invoice_search.inc',
	'invoice_view.inc',
	'invoice.inc',
	'item_search_results.inc',
	'item_search.inc'
);
$dir = opendir($root . 'classes');
while ($file = readdir($dir))
{
	$file = basename($file);
	if (strtolower(substr($file,-4)) == '.inc' && !in_array($file,$include_last)) { $include[] = $classdir . $file; }
}
sort($include);
while (list($a,$file) = each($include_last)) { $include[] = $classdir . $file; }
for ($i=0; $i<count($include); $i++) { include_once($include[$i]); }

$timer_class = new timer();
$timer_class->startTimer('entire_page',$_SESSION['start_time']);

/*****************
 ** CHECK LOGIN **
 *****************/

if (@$check_login)
{
	if (!@$_SESSION['store_loggedin'])
	{
		$return = @$_SERVER['REQUEST_URI'];
		headerLocation("/login.php?return=$return");
	}
}

// strip leading spaces and tabs/encode data (used in the output buffer)
function fix_output($data)
{
	global $force_strip_tabs;

	if (0 || @$force_strip_tabs)
	{
		// remove all tabs from output
		$data = str_replace("\t","",$data);

		if (0)
		{
			// trim trailing whitespaces on each line of output
			$exp = explode("\n",$data);
			for ($i=0; $i<count($exp); $i++) { $exp[$i] = trim($exp[$i]); }
			$data = implode("\n",$exp);
		}
	}

	$data = trim($data); // trim leading/trailing tabs/whitespace from output

	return $data;

	// !!! LOOK UP MEMORY LEAK !!!
	// if the browser supports gzip encoding, return the encoded output. otherwise, return the normal data
	//if (strpos($_SERVER['HTTP_ACCEPT_ENCODING'],'gzip') !== false) { return gzencode($data); }
	//else { return $data; }
}

// return time with milliseconds
function micro_time()
{
	$exp = explode(' ',microtime());
	return ((float)$exp[1]+(float)$exp[0]);
}

/* END OF FILE */
/* Location: ./include/include.inc */