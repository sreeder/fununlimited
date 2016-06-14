<?php
/**
* Report generation class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class reports
{
	/**
	* page class reference
	* @var reference $pg
	*/
	var $pg;

	/**
	* Error reporting class
	* @var identifier $error
	*/
	var $error;

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	*/
	function reports(&$pg)
	{
		$this->pg =& $pg;

		$this->error = new error('Reports Class');
	}
}

?>