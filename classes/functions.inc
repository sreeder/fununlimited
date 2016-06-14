<?php
/**
 * Miscellaneous functions
 *
 * @author Scott Carpenter <s-carp@comcast.net>
 * @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
 * @version 10/05/2012
 * @package Fun Unlimited
 */

/**
* Return the invoice type word (sale, trade, return)
* @param integer $type
* @param integer $caps capitalize the first word? [optional, default NO]
* @return string
*/
function invType($type,$caps=NO)
{
	$words = array(
		SALE    => 'sale',
		TRADE   => 'trade',
		RETURNS => 'return'
	);

	if ($caps) { return ucwords(@$words[$type]); }
	else { return @$words[$type]; }
}

/**
* Return 'New' or 'Used'
* @param integer $nu
* @return string
* @access public
*/
function getNU($nu)
{
	return ($nu==ITEM_NEW ? 'New' : 'Used');
}

/**
* Return the largest of two numbers
* @param integer $num1
* @param integer $num2
* @return integer
*/
function largest($num1,$num2) { return ($num1>$num2?$num1:$num2); }

/**
* In order to run files in /admin/scott_utils, make sure Scott Carpenter is logged in
* @param $integer $title [optional, default YES]
*/
function check_scottc_loggedin($title=YES)
{
	if (!isset($_SESSION['scottc_loggedin']))
	{
		if ($title)
		{
			?>
			Scott Carpenter must be logged in to use this function!
			<p />
			<?php
		}
		?>
		<form method="post" action="/admin/scott_utils/scottclogin.php">
			Username: <input type="text" name="user" size="10" /><br />
			Password: <input type="password" name="pass" size="10" /><br />
			<input type="submit" value="Log In &gt;" />
		</form>
		<?php
		die();
	}
	else
	{
		?>
		<table border="1" bordercolor="#000000" cellspacing="0" cellpadding="5">
			<tr>
				<td bgcolor="#CCCCCC">Scott Carpenter is logged in (<a href="/admin/scott_utils/scottclogout.php">Log Out</a>)</td>
			</tr>
		</table>
		<p />
		<?php
	}
}

if (!function_exists('array_combine'))
{
	function array_combine($a,$b)
	{
		$num = count($a);
		if ($num != count($b) || $num == 0) { return false; }

		$a = array_values($a);
		$b = array_values($b);

		$c = array();
		for ($i=0; $i<$num; $i++) { $c[$a[$i]] = $b[$i]; }
		return $c;
	}
}

/**
* Return the GET value, or if not set, the POST value. If $default provided and GET/POST not set, it is returned.
* @param string $var
* @param mixed $default [optional, default '']
* @param array $limit array of values to limit the return to (if the value isn't in the array, the default is returned) [optional, default array()]
* @return mixed
*/
function getGP($var,$default='',$limit=array())
{
	if (isset($_GET[$var])) { return (!count($limit) || (count($limit) && iin_array($_GET[$var],$limit)) ? $_GET[$var] : $default); }
	elseif (isset($_POST[$var])) { return (!count($limit) || (count($limit) && iin_array($_POST[$var],$limit)) ? $_POST[$var] : $default); }
	else { return $default; }
}

/**
* Return the GET value. If $default provided and GET not set, it is returned.
* @param string $var if -1, $_GET is returned [optional, default -1]
* @param mixed $default [optional, default '']
* @param array $limit array of values to limit the return to (if the value isn't in the array, the default is returned) [optional, default array()]
* @access public
* @return mixed
*/
function getG($var=-1,$default='',$limit=array())
{
	if ($var == -1) { return $_GET; }
	elseif (isset($_GET[$var])) { return (!count($limit) || (count($limit) && iin_array($_GET[$var],$limit)) ? $_GET[$var] : $default); }
	else { return $default; }
}

/**
* Return the POST value. If $default provided and POST not set, it is returned.
* @param string $var if -1, $_POST is returned [optional, default -1]
* @param mixed $default [optional, default '']
* @param array $limit array of values to limit the return to (if the value isn't in the array, the default is returned) [optional, default array()]
* @access public
* @return mixed
*/
function getP($var=-1,$default='',$limit=array())
{
	if ($var == -1) { return $_POST; }
	elseif (isset($_POST[$var])) { return (!count($limit) || (count($limit) && iin_array($_POST[$var],$limit)) ? $_POST[$var] : $default); }
	else { return $default; }
}

/**
 * Return the value in the array. If $default provided and the key is not set, it is returned.
 * @param  array  &$array  the array
 * @param  string $var     if -1, the array is returned [optional, default -1]
 * @param  mixed  $default [optional, default '']
 * @param  array  $limit   array of values to limit the return to (if the value isn't in the array, the default is returned) [optional, default array()]
 * @return mixed
 */
function getA(&$array, $var=-1, $default='', $limit=array())
{
	if ($var == -1)
	{
		return $array;
	}
	elseif (isset($array[$var]))
	{
		return (!$limit || iin_array($array[$var], $limit) ? $array[$var] : $default);
	}
	else
	{
		return $default;
	}
}

/**
* Case-insensitive in_array
* @param mixed $needle
* @param array $haystack
* @return boolean
*/
function iin_array($needle,$haystack)
{
	$needle = (string)$needle;
	if (!is_array($haystack)) { return; }
	elseif (is_array($needle)) { return in_array($needle,$haystack); }
	else
	{
		while (list($k,$v) = each($haystack))
		{
			if (strtolower((string)$v) == strtolower($needle)) { return YES; }
		}
		return NO;
	}
}

/**
* Return the 's' if the given number merits it
* @param integer $num
* @return string
*/
function getS($num) { return ($num!=1 ? 's' : ''); }

/**
* Output a header("Location...") redirect
* @param string $location
* @access public
*/
function headerLocation($location)
{
	if (headers_sent())
	{
		$pg = new page();
		$pg->error('Unable to complete headerLocation call. Headers already sent!');
	}
	else
	{
		if (strpos($location,'://') === false)
		{
			$location = "http://{$_SERVER['SERVER_NAME']}" . (substr($location,0,1)!='/' ? '/' : '') . $location;
		}
		header("Location: $location");
	}

	die();
}

/**
* Make sure that there is at least one element in the array
* If not, add a '0' as the only element
* @param array &$array
* @access public
*/
function checkArray(&$array)
{
	if (!is_array($array) || !count($array)) { $array = array(0); }
}

/**
* Return the array imploded and surrounded by () for queries
* @param array $array
* @return string
* @access public
*/
function getIn($array)
{
	checkArray($array);
	return "('" . implode("','",$array) . "')";
}

/**
* Return the CSS class for the given IDX
* @param integer $idx
* @return string
* @access public
*/
function getRowClass($idx)
{
	return (!($idx%2) ? 'line_even' : 'line_odd');
}

/**
* Remove the 'xxx_' prefix from the elements in the array
* @param array &$reginfo
* @access public
*/
function fixInfoArray(&$reginfo)
{
	$new_reginfo = array();
	while (list($key,$val) = each($reginfo))
	{
		$new_reginfo[substr($key,4)] = $val;
	}

	$reginfo = $new_reginfo;
}

/**
* Given an array of key=>value pairs and a prefix, generates a pre_field='value',pre_field=[...] set for an insert into the database
* @param	array	$vals
* @param	string	$prefix
* @return	string
* @access	public
*/
function createInsertVals($vals,$prefix='')
{
	$insert = array();
	while (list($key,$val) = each($vals))
	{
		if (strlen($prefix))
		{
			$key = $prefix . '_' . $key;
		}
		$insert[] = "$key='" . mysql_real_escape_string($val) . "'";
	}
	return implode(', ',$insert);
}
function createUpdateVals($vals,$prefix='')
{
	return createInsertVals($vals,$prefix);
}

/**
* Return the javascript 'true' or 'false' for the boolean value
* @param	boolean	$bool
* @return	string
* @access	public
*/
function jstf($bool)
{
	return ($bool ? 'true' : 'false');
}

/**
* Given an array, return an array with the specified value as the key and the value
* @param	array	$array
* @param	mixed	$key	key in each array element who's value will be used as the key and value (if -1, each array element is the value to use) [optional, default -1]
* @return	array
* @access	public
*/
function makeKeyValueArray($array,$key=-1)
{
	$new_array = array();
	while (list($a,$arr) = each($array))
	{
		if ($key == -1)
		{
			$new_array[$arr] = $arr;
		}
		elseif (is_array($key))
		{
			$use_key = @$arr[$key[0]];
			$use_value = @$arr[$key[1]];
			$new_array[$use_key] = $use_value;
		}
		else
		{
			$value = @$arr[$key];
			$new_array[$value] = $value;
		}
	} // each array element

	unset($new_array['']);

	return $new_array;
} // end function makeKeyValueArray

/**
* Output a formatted print_r() and optionally die
* @param	mixed	$var
* @param	boolean	$die	[optional, default NO]
* @access	public
*/
function printR($var, $die=NO)
{
	?><div align="left"><pre><?php echo print_r($var, true);?></pre></div><?php
	if ($die)
	{
		die();
	}
}

/**
* Output a formatted print_r() and die
* @param	mixed	$var
* @access	public
*/
function printRdie($var)
{
	printR($var, YES);
}

/**
* Send a debug email to debug@fununlimitedonline.com
* @param	mixed	$body		can be a string or an array (if array, the print_r() is sent)
* @param	string	$subject	if -1, the body is used as the subject [optional, default 'FU Debug']
*/
function debugEmail($body, $subject='FU Debug')
{
	// grab the file/line from debug_backtrace
	$bt = debug_backtrace();
	$file = $bt[0]['file'];
	$base_file = str_replace(
		str_replace(
			'\\',
			'/',
			$_SERVER['DOCUMENT_ROOT']
		),
		'',
		str_replace(
			'\\',
			'/',
			$file
		)
	);
	$line = $bt[0]['line'];

	if (is_array($body))
	{
		$body = print_r($body, true);
	}

	if ($subject == -1)
	{
		$subject = $body;
	}

	$subject .= " [$base_file:$line]";
	$body = "File: $file\nLine: $line\n------------------\n$body";

	@mail(
		'debug@fununlimitedonline.com',
		$subject,
		$body,
		'From: FU Debug <debug@fununlimitedonline.com>'
	);
} // end function debugEmail

/**
* Return ' checked="checked"' if the given value is true
* @param	bool	$bool	[optional, default true]
* @param	mixed	$value	checked if this value [optional, default true]
* @return	string
*/
function getChecked($bool=true, $value=true)
{
	return ($bool==$value ? ' checked="checked"' : '');
}

/**
* Return ' selected="selected"' if the given value is true
* @param	bool	$bool	[optional, default true]
* @param	mixed	$value	checked if this value [optional, default true]
* @return	string
*/
function getSelected($bool=true, $value=true)
{
	return ($bool==$value ? ' selected="selected"' : '');
}

/**
* Return ' disabled="disabled"' if the given value is true
* @param	bool	$bool	[optional, default true]
* @param	mixed	$value	checked if this value [optional, default true]
* @return	string
*/
function getDisabled($bool=true, $value=true)
{
	return ($bool==$value ? ' disabled="disabled"' : '');
}

/**
 * Return the first value if it isn't zero, otherwise return the other value
 * @param mixed $if
 * @param mixed $else [optional, default '']
 * @return mixed
 */
function ifNotZero($if, $else)
{
	return ($if ? $if : $else);
}

if (!function_exists('fputcsv'))
{
	/**
	 * Write an array of data to the given file as a CSV
	 * @param	object	&$handle
	 * @param	array	$arr
	 * @param	string	$fd				field delimeter [optional, default ',']
	 * @param	string	$quote			quote character [optional, default '"']
	 * @param	boolean	$prefix_plus	prefix a + to each field? [optional, default false]
	 * @param	array	$no_plus		don't prefix a plus sign to these column offsets [optional, default array()]
	 * @return	integer	number of characters written
	 */
	function fputcsv(&$handle, $arr, $fd=',', $quote='"', $prefix_plus=false, $no_plus=array())
	{
		$str = '';

		$idx = -1;

		foreach ($arr as $cell)
		{
			$idx++;

			$plus = ($prefix_plus && !in_array($idx, $no_plus) ? '+' : '');

			$cell = str_replace(
				array($quote, "\n"),
				array($quote . $quote, ''),
				$cell
			);

			// encapsulate ALL values in the quote character
			$str .= $plus . $quote . $cell . $quote . $fd;
		} // each value

		fputs($handle, substr($str, 0, -1) . chr(10)); // trim the last field delimeter and force a line feed

		return strlen($str);
	} // end function fputcsv
} // if fputcsv() doesn't exist

/* END OF FILE */
/* Location: ./classes/functions.inc */