<?php
include('../../include/include.inc');

$cl = new check_login();

$page = (isset($_GET['page'])?$_GET['page']:@$_POST['page']);
if (!is_numeric($page)) { $page = 1; }

$pg = new admin_page();
$pg->setTitle('Whole Platform Item Info');
$pg->head();

$error = new error('Whole Platform Item Info');

$criteria = @$_SESSION['platform_item_criteria'];

if (isset($criteria['platformID']) && is_array(@$criteria['elements']))
{
	$platformID = $criteria['platformID'];
	$elems = $criteria['elements'];
	$pla = new platforms($pg,$platformID);

	// number of items to show per-page and item index boundaries
	$perpage = 100;
	$start = (($page - 1) * $perpage);

	// set/pull the element variables
	include('platform_items_elements.php');

	// pull the items for the selected platform
	$timer_class->startTimer('get_items');
	$items = array();
	$itemIDs = array();
	$result = runQuery($platformID,'*',"LIMIT $start,$perpage");
	while ($row = mysql_fetch_assoc($result))
	{
		$itemIDs[] = $row['itm_itemID'];
		$items[] = $row;
	}
	$timer_class->stopTimer('get_items');

	// if there are items and they have selected the source pricing element, pull the source pricing
	$timer_class->startTimer('sourcepricing');
	if (count($items) && in_array('sourcepricing',$elems))
	{
		$source_pricing = array(); // format: $source_pricing[itemID][sourceID] = #

		if (count($price_sourceIDs))
		{
			$sql = "SELECT * FROM item_source_values WHERE isv_sourceID IN (".implode(',',$price_sourceIDs).") AND isv_itemID IN (".implode(',',$itemIDs).")";
			$result = mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				if (!isset($source_pricing[$row['isv_itemID']])) { $source_pricing[$row['isv_itemID']] = array(); }
				$source_pricing[$row['isv_itemID']][$row['isv_sourceID']] = $row['isv_value'];
			}
		}
	}
	$timer_class->stopTimer('sourcepricing');

	// if there are items and they have selected the source ratings element, pull the source ratings
	$timer_class->startTimer('sourceratings');
	if (count($items) && in_array('sourceratings',$elems))
	{
		$source_ratings = array(); // format: $source_ratings[itemID][sourceID] = #

		if (count($rating_sourceIDs))
		{
			$sql = "SELECT * FROM item_source_values WHERE isv_sourceID IN (".implode(',',$rating_sourceIDs).") AND isv_itemID IN (".implode(',',$itemIDs).")";
			$result = mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				if (!isset($source_ratings[$row['isv_itemID']])) { $source_ratings[$row['isv_itemID']] = array(); }
				$source_ratings[$row['isv_itemID']][$row['isv_sourceID']] = $row['isv_value'];
			}
		}
	}
	$timer_class->stopTimer('sourceratings');

	?><table border="0" width="100%"><tr><td align="left"><table border="0" cellspacing="0" cellpadding="0" width="800"><tr><td align="center"><?php
	$pg->pageHead('Whole Platform Item Info');
	$pla->show_platform(YES,'/admin/setup_items/platform_items.php',NO);
	?>[ <b>Page #<?=$page;?></b> ]<p /><?php

	if (isset($_GET['updated']))
	{
		$pg->status(($_GET['updated'] ? @$_GET['count'] . ' items have been updated<br />' . @$_GET['changed'] . ' database rows were changed' : 'No item information was updated'));
	}

	if (!count($items))
	{
		?>
		There are no items in the platform/on the page you have selected.
		<p />
		<input type="button" value="&lt; Return to Criteria Form" onclick="document.location='/admin/setup_items/platform_items.php'" class="btn" />
		</td></tr></table>
		<?php
	}
	else
	{
		// output the item form
		?>
		Change any item's information and press a submit button below.
		</td></tr></table></td></tr></table>
		<p />
		<?php

		// rebuild the passed elements - expand any expandable elements
		$new_elems = array();
		while (list($a,$element) = each($elems))
		{
			if ($element == 'companies') { $new_elems[] = 'company1'; $new_elems[] = 'company2'; }
			elseif ($element == 'quantities') { $new_elems[] = 'qtynew'; $new_elems[] = 'qtyused'; }
			elseif ($element == 'pricing') { $new_elems[] = 'pricenew'; $new_elems[] = 'priceused'; }
			elseif ($element == 'sourcepricing')
			{
				while (list($a,$price_sourceID) = each($price_sourceIDs)) { $new_elems[] = "sp_$price_sourceID"; }
				reset($price_sourceIDs);
			}
			elseif ($element == 'sourceratings')
			{
				while (list($a,$rating_sourceID) = each($rating_sourceIDs)) { $new_elems[] = "sr_$rating_sourceID"; }
				reset($rating_sourceIDs);
			}
			elseif ($element != 'title') { $new_elems[] = $element; }
		}
		$elems = array_merge(array('title'),$new_elems); // force title into the front of the pack

		// set the column titles
		$columns = array();
		while (list($a,$element) = each($elems))
		{
			$columns[$element] = $elements[$element][0];
		}
		reset($elems);

		// get the boundaries
		$start = 0; $end = $perpage;
		if ($end > count($items)) { $end = count($items); }
		$addforindex = (($page-1)*$perpage);
		$heads_every = 20;

		$timer_class->startTimer('show_items');
		$pg->outlineTableHead();
		?>
		<form method="post" action="/admin/setup_items/platform_itemsUpdate.php" id="itmfrm">
		<input type="hidden" name="act" value="updateitems" />
		<input type="hidden" name="page" value="<?=$page;?>" />
		<?php
		$idx = -1;
		$head_count = -1;
		for ($i=$start; $i<$end; $i++)
		{
			$idx++;
			$head_count++;
			$arr = $items[$i];
			$bg = (($idx%2)?$pg->color('table-cell'):$pg->color('table-cell2'));
			$itemID = $arr['itm_itemID'];

			if (!($head_count%$heads_every))
			{
				// output the colum headers
				?>
				<tr bgcolor="<?=$pg->color('table-head');?>">
					<td>&nbsp;</td>
					<?php
					while (list($element,$heading) = each($columns)) { ?><td align="center"><b><?=$heading;?></b></td><?php }
					reset($columns);
					?>
				</tr>
				<?php
			}

			?>
			<tr bgcolor="<?=$bg;?>" align="center">
				<td align="right"><font color="#CCCCCC"><?=($addforindex+$i+1);?></font></td>
				<?php
				while (list($element,$heading) = each($columns))
				{
					$config = $elements[$element];
					$title = $config[0];
					$type = $config[1];
					$size = $config[2];
					$field = $config[3];
					$data_array = @$config[4];

					// get the value
					if (substr($field,0,3) == 'sp_')
					{
						$price_sourceID = substr($field,3);
						$value = @$source_pricing[$itemID][$price_sourceID];
					}
					elseif (substr($field,0,3) == 'sr_')
					{
						$rating_sourceID = substr($field,3);
						$value = @$source_ratings[$itemID][$rating_sourceID];
					}
					else { $value = $arr[$field]; }

					?><td><?php
					if ($type == 'text') { ?><input type="text" name="info[<?=$itemID;?>][<?=$field;?>]" size="<?=$size;?>" value="<?=$value;?>" /><?php }
					elseif ($type == 'select')
					{
						if (!is_array($data_array)) { echo '$data_array not array'; }
						else
						{
							?><select name="info[<?=$itemID;?>][<?=$field;?>]" size="<?=$size;?>"><?php
							while (list($val,$show) = each($data_array))
							{
								if ($val == $value) { $s = ' selected="selected"'; } else { $s = ''; }
								?><option value="<?=$val;?>"<?=$s;?>><?=$show;?></option><?php
							}
							?></select><?php
						}
					}
					?></td><?php
				}
				reset($columns);
				?>
			</tr>
			<?php
		}
		$pg->outlineTableFoot();
		$timer_class->stopTimer('show_items');
		?>
		<p />
		<?=submitNavCode($platformID,$page);?>
		</form>
		<?php
	} // else items
} // if criteria
else
{
	$pg->error('Invalid criteria');
}

//$timer_class->viewTimes();

/**
* Run the query with the pieces filled in and return the result reference
* @param integer $platformID
* @param string $select SELECT $select FROM ...
* @param string $limit LIMIT line [optional, default '']
* @return reference
*/
function runQuery($platformID,$select,$limit='')
{
	global $db,$error,$t;

	set_time_limit(60);
	$sql = "SELECT $select FROM items,quantity,prices WHERE itm_platformID=$platformID AND itm_active=" . YES . " AND itm_itemID=prc_itemID AND qty_storeID={$_SESSION['storeID']} AND prc_itemID=qty_itemID ORDER BY itm_title $limit";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);

	return $result;
}

/**
* Return array of pages and the first title of the page
* @param $platformID
* @return array
*/
function getFirstTitles($platformID)
{
	global $perpage;

	$titles = array();

	// get total items in platform
	$result = runQuery($platformID,'COUNT(*) AS count');
	$row = mysql_fetch_assoc($result);
	$total = $row['count'];

	$pages = ceil($total/$perpage);
	for ($i=0; $i<$pages; $i++)
	{
		$first_index = ($i*$perpage);
		$result = runQuery($platformID,'itm_title',"LIMIT $first_index,1");
		$row = mysql_fetch_assoc($result);
		$titles[$i] = $row['itm_title'];
	}

	return array($total,$pages,$titles);
}

/**
* Output the submit/navigation code
* @param integer $platformID
* @param integer $page current page
*/
function submitNavCode($platformID,$page)
{
	global $pg;

	list($total,$pages,$titles) = getFirstTitles($platformID);

	?>
	<script type="text/javascript">
		function gotoPage(page)
		{
			var obj = document.getElementById('itmfrm');
			if (!obj.doupdate.checked || confirm("Are you sure you want to update the above items' information?\n\nNote: this is NOT reversible!"))
			{
				if (!obj.doupdate.checked && page == <?=$page;?>) { alert("You're already there..."); } // there's no point in going to the same page...
				else
				{
					obj.page.value = page;
					obj.submit();
				}
			}
		}
	</script>

	<table border="0" width="100%"><tr><td align="left">
		<?=$pg->outlineTableHead(800);?>
			<tr>
				<td align="center" bgcolor="<?=$pg->color('table-cell2');?>">
					If selected, all submit buttons will update the items and take you to the named page.
					<p />
					<font color="red"><b>
						It may take a minute to submit the form and update the items.<br />
						Please be patient and press the button <u>ONLY ONCE!!!</u>
					</font></b>
					<p />
					<b>Current Page:</b> #<?=$page;?> of <?=$pages;?><br />
					<b>Total Items in Platform:</b> <?=number_format($total,0);?><br />
					&nbsp;
				</td>
			</tr>
			<tr>
				<td align="center" bgcolor="<?=$pg->color('table-cell');?>">
					&nbsp;<br />
					<input type="checkbox" name="doupdate" id="doupdate" value="<?=YES;?>" checked="checked" class="nb" /> <label for="doupdate">Update Items When Button Pressed</label> (uncheck this if you just want to switch pages)
					<p />
					<input type="button" value="&lt; Previous Page" onclick="gotoPage(<?=($page-1);?>)" alt="Page <?=($page-1);?>" class="btn" style="width:150px"<?=(!($page-1)?' disabled="disabled"':'');?> />
					<input type="button" value="Same Page" onclick="gotoPage(<?=$page;?>)" alt="Page <?=$page;?>" class="btn" style="width:150px" />
					<input type="button" value="Next Page &gt;" onclick="gotoPage(<?=($page+1);?>)" alt="Page <?=($page+1);?>" class="btn" style="width:150px"<?=(($page+1)>$pages?' disabled="disabled"':'');?> />
					<p />
					<select id="pages" size="1"><?php
						while (list($p,$title) = each($titles))
						{
							$pgnum = ($p+1);
							if ($pgnum == $page) { $s = ' selected="selected"'; } else { $s = ''; }
							?><option value="<?=$pgnum;?>"<?=$s;?>><?="Page #$pgnum - ".substr($title,0,30).(strlen($title)>30?'...':'');?></option><?php
						}
					?></select>
					<input type="button" value="Selected Page &gt;" onclick="gotoPage(document.getElementById('pages').options[document.getElementById('pages').selectedIndex].value)" class="btn" style="width:150px" />
					<p />
					<font size="1"><b>Note:</b> The title in the select box is the first title on that page.</font>
				</td>
			</tr>
		<?=$pg->outlineTableFoot();?>
	</td></tr></table>
	<?php
}

$pg->foot();
?>