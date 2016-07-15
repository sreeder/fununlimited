<?php
/*
* Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
include('../../include/include.inc');

$act = getP('act');
$ytd = getP('ytd');
$archived = getP('archived');
$page = getP('page');
$pages = getP('pages');
$dest = getP('dest');
$set = getP('set');

$pg = new admin_page();
$error = new error('Top 500 Update');

if ($act == "save")
{
	$page = ($dest=='next' ? ($page<$pages ? ($page+1) : $pages) : ($dest=='same' ? $page : ($page>1 ? ($page-1) : 1)));

	$sqls = array();
	while (list($customerID,$arr) = each($set))
	{
		$vals = array();
		while (list($k,$v) = each($arr))
		{
			if (in_array($k,array('cus_phone','cus_zip'))) { $v = validate::strip($v); }

			$vals[] = "$k='".mysql_real_escape_string($v)."'";
		}
		$sqls[] = "UPDATE customers SET ".implode(',',$vals)." WHERE cus_customerID=$customerID";
	}

	while (list($a,$sql) = each($sqls))
	{
		mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);
	}
}

$return = "/admin/utilities/top500.php?page=$page&ytd=$ytd&archived=$archived";

$pg->showUpdating('Updating Top 500 Customers...',$return);
?>