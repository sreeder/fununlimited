<?php
/**
* Checks that the user is logged in; if not, display the login page; also validates a user level
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class check_login
{
	/**
	* Class constructor
	* Checks that the user is logged in/is the required level and acts accordingly
	* @param integer $level The required level of the user (the user's level must be <= the required) [optional, default 0]
	*/
	function check_login($level=0)
	{
		if (!@$_SESSION['loggedin'])
		{
			header('Location: /admin/login.php');
			die();
		}
		else if ($level)
		{
			// check the user's level
			if ($_SESSION['usertype'] > $level)
			{
				// they aren't the correct level - report
				$pg = new admin_page();
				$pg->setTitle('No Permissions');
				$pg->head('No Permissions');
				echo 'You do not have permissions to access this page.';
				$pg->foot();
				die();
			}
		}
	}
}

?>