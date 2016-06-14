<?php
/**
* Multidimensional array sorting - borrowed from TireGuru.net Pro
*
* @author TireGuru.net Pro (Tom Galloway/Scott Carpenter)
* @version 1.0
* @package Fun Unlimited
*/

class sort
{
  function sort()
  {
  }

	function doSort()
	{
		$i = -1;
		$args = func_get_args();
		$sorted_array = array_shift($args);
		$sort_command = "return(array_multisort(";

		if (is_array($args) && count($args[0]))
		{
			foreach ($args as $arg)
			{
				$i++;
				if(is_string($arg))
				{
					foreach($sorted_array as $row)
					{
						$sortarr[$i][] = $row[$arg];
					}
				}
				else
				{
					$sortarr[$i] = $arg;
				}

				$sort_command .= '$sortarr[' . $i . '],';
			}

			$sort_command .= '$sorted_array));';

			eval($sort_command);
			return $sorted_array;
		}
		else
		{
			return $args[0];
		}
	}

	/**
	* Fix the given array keys by setting them to the given key
	* @param array &$array
	* @param string $key
	* @access public
	*/
	function fixArrayKeys(&$array,$key)
	{
		$new_array = array();
		foreach ($array as $a => $arr)
		{
			$new_array[$arr[$key]] = $arr;
		}
		$array = $new_array;
		reset($array);
	}
}
?>