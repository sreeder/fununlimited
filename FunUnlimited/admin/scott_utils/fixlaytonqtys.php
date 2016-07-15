<?php
include('../../include/include.inc');
check_scottc_loggedin();

$do = 'import'; // 'write' or 'import'
$layton_storeID = 10;

if ($do == "write")
{
	$f = fopen('layton_qtys.csv','w');

	$sql = "SELECT itm_itemID,itm_upc,pla_name,itm_title,qty_new,qty_used FROM items,platforms,quantity WHERE itm_platformID=pla_platformID AND itm_itemID=qty_itemID AND qty_storeID=$layton_storeID AND (qty_new>0 OR qty_used>0) ORDER BY pla_name,itm_title";
	$result = mysql_query($sql,$db);
	while ($row = mysql_fetch_assoc($result))
	{
		$vals = array();
		while (list($k,$v) = each($row)) { $vals[] = str_replace('"',"'",str_replace(',',' ',$v)); }
		fwrite($f,implode(',',$vals)."\n");
	}

	fclose($f);
}
elseif ($do == "import")
{
	echo "Fixing layton quantities...<br />";flush();
	$sqls = array();

	?>
	<b>Items not in <u>quantity</u> table:</b>
	<p />
	<table border="1" cellspacing="0" cellpadding="2">
		<tr>
			<td><b>UPC</b></td>
			<td><b>Platform</b></td>
			<td><b>Title</b></td>
			<td><b>New</b></td>
			<td><b>Used</b></td>
		</tr>
		<?php
		$file = file('layton_qtys.csv');
		while (list($a,$line) = each($file))
		{
			list($itemID,$upc,$platform,$title,$qty_new,$qty_used) = explode(',',trim($line));

			// make sure the item exists in the layton quantities
			$sql = "SELECT * FROM quantity WHERE qty_storeID=$layton_storeID AND qty_itemID=$itemID";
			$result = mysql_query($sql,$db);
			if (!mysql_num_rows($result))
			{
				?>
				<tr>
					<td><?=$upc;?></td>
					<td><?=$platform;?></td>
					<td><?=$title;?></td>
					<td><?=$qty_new;?></td>
					<td><?=$qty_used;?></td>
				</tr>
				<?php
			}
			else
			{
				$sqls[] = "UPDATE quantity SET qty_new=(qty_new+$qty_new),qty_used=(qty_used+$qty_used) WHERE qty_storeID=$layton_storeID AND qty_itemID=$itemID";
			}
		}
		?>
	</table>
	<p />
	<?php
	echo "Updating quantities for ".count($sqls)." items...";

	?><b>$sqls</b><pre><?=print_r($sqls);?></pre><?php

	while (list($a,$sql) = each($sqls))
	{
		set_time_limit(30);
		//mysql_query($sql,$db);
		if (mysql_errno()) { die('MySQL error: '.mysql_error()); }
	}
}

?>
done!