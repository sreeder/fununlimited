<?php
/**
* Website constants
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

$sql = 'SELECT flg_const,flg_number FROM flags';
$result = mysql_query($sql,$db);
while ($row = mysql_fetch_assoc($result))
{
	$_SESSION['flags'][$row['flg_const']] = $row['flg_number'];
	define(strtoupper($row['flg_const']),$row['flg_number']);
}
unset($sql);