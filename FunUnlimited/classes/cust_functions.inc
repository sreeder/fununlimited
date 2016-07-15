<?php
/**
* Custom functions
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

function array_replace_keys($array,$replacements)
{
	while (list($k,$v) = each($array))
	{
		while (list($rk,$rv) = each($replacements))
		{
			if ($k == $rk) { $array[$k] = $rv; }
		}
		reset($replacements);
	}
	return $array;
}

function ncsort(&$arr) { usort($arr,'cmp'); }
function cmp($a,$b) { return strcasecmp($a,$b); }

?>