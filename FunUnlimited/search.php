<?php
include('include/include.inc');

if (getG('last') && isset($_SESSION['search_last']))
{
	$_GET = $_SESSION['search_last'];
	$_GET['prev'] = YES;
}

$act = getGP('act');
$search_type = getG('search_type');
if (!in_array($search_type,array("ITEM_SEARCH_SIMPLE","ITEM_SEARCH_ADVANCED"))) { $search_type = "ITEM_SEARCH_ADVANCED"; }

$titles = array(
	''       => 'Advanced Search',
	'search' => 'Catalog Search Results',
);
if (!in_array($act,array_keys($titles))) { $act = ''; }

$pg = new page();
$pg->setTitle($titles[$act]);
$pg->head($titles[$act]);

// save the criteria
$_SESSION['search_last'] = getG();

if ($act == '')
{
	// output the advanced search form
	$its = new item_search($pg);
	$its->setAction('search.php');
	$its->advanced_form();
}
elseif ($act == 'search')
{
	// output the search results
	$prev = getG('prev');

	$its = new item_search($pg);
	$its->setAction('search.php');
	$its->setOnline(YES);
	$its->pull_post();
	$criteria = $its->getCriteria();

	if (!$prev)
	{
		if ($search_type == "ITEM_SEARCH_SIMPLE")
		{
			// perform a simple search
			$searchby = getG('searchby');
			$text = getG('text');

			/*
			searchby values:
			--------------------------------------
			title				Title
			platform		Platform
			company			Company
			type				Type (IE: romance, sports)
			lessprice		Price Less Than
			moreprice		Price Greater Than
			pricerange	Price Range (IE: 10-14)
			age					Age (IE: 6, 10-12, 14+)
			year				Year
			--------------------------------------
			*/

			$its->criteria[$searchby] = $text;
			$its->search();
		}
		elseif ($search_type == "ITEM_SEARCH_ADVANCED")
		{
			// perform an advanced search
			$its->search();
		}
	}
	else
	{
		$its->setResults(@$_SESSION['search_results']);
	}

	$results = $its->getResults();
	$narrow = $its->getNarrow();

	if (!count($results))
	{
		?>
		Your search did not return any results. Please try again below.
		<p />
		<?php
		$its->advanced_form();
	}
	else
	{
		// output the results
		$its->page = (!isset($_GET['page']) || !is_numeric($_GET['page']) ? 1 : $_GET['page']);

		$result_count = @$_SESSION['search_count'];
		if (!strlen($result_count)) { $result_count = 0; }

		// apply narrowing filters
		$filter_results = array(); // filtered results
		while (list($a,$arr) = each($results))
		{
			$fitsall = YES;

			while (list($k,$v) = each($narrow))
			{
				if (strlen($v))
				{
					if ($k == 'platform' && $arr['itm_platformID'] != $v) { $fitsall = NO; }
					elseif ($k == 'startswith' && (($v == '#' && (ord(strtoupper(substr($arr['itm_title'],0,1))) >= 65 && ord(strtoupper(substr($arr['itm_title'],0,1))) <= 90)) || ($v != '#' && ord(strtoupper(substr($arr['itm_title'],0,1))) != ord($v)))) { $fitsall = NO; break; }
					elseif ($k == 'company' && $arr['itm_company1ID'] != $v && $arr['itm_company2ID'] != $v) { $fitsall = NO; break; }
					elseif ($k == 'type' && $arr['typ_typeID'] != $v) { $fitsall = NO; break; }
				}
			}
			reset($narrow);

			if ($fitsall) { $filter_results[] = $arr; }
		}
		reset($results);
		$results = $filter_results;

		$count = count($results);
		$pages = ceil($count/$its->per_page);

		$start = ($its->per_page*($its->page-1));
		$end = ($its->per_page*$its->page);
		if ($end > $count) { $end = $count; }

		// if they are on the 'browse catalog' page and have selected a platform
		$oncatalog = NO;
		if (isset($_SESSION['catalog_platformID']) && basename($_SERVER['PHP_SELF']) == 'catalog.php')
		{
			// output the platform name
			$its->pg->pageHead('Browse Catalog by Platform');
			$pla = new platforms($its->pg,$_SESSION['catalog_platformID']);
			$pla->show_platform(YES,'catalog.php');
			$oncatalog = YES;
		}

		// if the number of results equals $its->max_results and no narrow-by has been chosen, alert the user
		$total = 0;
		while (list($k,$v) = each($narrow))
		{
			if (strlen($v)) { $total++; }
		}
		reset($narrow);

		if ($result_count == $its->max_results && !$total)
		{
			?>
			<b>Note:</b> <?=$its->max_results;?>+ results returned. Please narrow your criteria.
			<p />
			<?php
		}

		if ($count)
		{
			// output the result navigation bar
			?>
			<script type="text/javascript">
				var page = <?=$its->page;?>;
				var pages = <?=$pages;?>;
				function loadpage(num) { if (num != page) { var frm = document.lp; frm.page.value = num; frm.submit(); } }
				function goprev() { if ((page-1)) { loadpage((page-1)); } }
				function gonext() { if ((page+1) <= <?=$pages;?>) { loadpage((page+1)); } }
			</script>

			<table border="0" cellspacing="0" cellpadding="2">
				<tr>
					<td align="right">
						<a href="javascript:loadpage(1)" title="First Page"><img src="images/search_first.gif" width="60" height="18" border="0" /></a>
						<a href="javascript:goprev()" title="Previous Page"><img src="images/search_prev.gif" width="48" height="18" border="0" /></a>
					</td>
					<td align="center" valign="middle" style="font-size:12">
						<?php
						$padby = 3;

						$ps = (($its->page-$padby)<1 ? 1 : ($its->page-$padby));
						$pe = (($its->page+$padby)>$pages ? $pages : ($its->page+$padby));

						// !!! fix this - doesn't work for $pages=(# +/- 3-4 of $padby*2) !!!
						if (0 && $pages > (($padby*2)+1) && ($pe-$ps) < ($padby*2))
						{
							// if they are within (pad*2) pages of the beginning, show (page-(pad*2)) more page links
							// if they are within (pad*2) pages of the end, show (pad*2)-(pages-page) more page links
							// this causes (pad*2) page links to always be shown

							//echo "($pe-$ps) = ".($pe-$ps).">".($padby*2);
							if (($ps-($padby*2)) < $padby) { $ps -= ($padby*2)-($pe-$ps); }
							elseif (($ps-($padby*2)) < 1) { $pe += ($padby*2)-($pe-$ps); } // add more to end
							else { $ps -= ($padby*2)-($pe-$ps); } // add more to start
						}
						elseif ($pages == ($padby*2) || $pages == (($padby*2)+1)) { $ps = 1; $pe = $pages; }

						if ($pe > $pages) { $pe = $pages; }

						if ($ps > 1)
						{
							?><a href="javascript:loadpage(1)" style="text-decoration:underline" title="Results 1-<?=$its->per_page;?>">1</a> <?php
							if (($its->page-$padby) > 2) { echo "... "; }
						}

						$links = array();
						for ($j=$ps; $j<=$pe; $j++)
						{
							$resnum = ((($j-1)*$its->per_page)+1).'-'.(($j*$its->per_page)>$count ? $count : ($j*$its->per_page));

							$str = '';
							if ($j != $its->page) { $str .= '<a href="javascript:loadpage('.$j.')" style="text-decoration:underline" title="Results '.$resnum.'">'; } else { $str .= "<b>"; }
							$str .= $j;
							if ($j != $its->page) { $str .= '</a>'; } else { $str .= "</b>"; }
							$links[] = $str;
						}
						echo implode(' ',$links);

						if ($pe != $pages)
						{
							$lastresnum = ((($pages-1)*$its->per_page)+1).'-'.$count;
							if (($its->page+$padby) < ($pages-1)) { echo ' ...'; }
							?> <a href="javascript:loadpage(<?=$pages;?>)" style="text-decoration:underline" title="Results <?=$lastresnum;?>"><?=$pages;?></a><?php
						}

						?>
					</td>
					<td align="left">
						<a href="javascript:gonext()" title="Next Page"><img src="images/search_next.gif" width="48" height="18" border="0" /></a>
						<a href="javascript:loadpage(<?=$pages;?>)" title="Last Page"><img src="images/search_last.gif" width="60" height="18" border="0" /></a>
					</td>
					<td><img src="images/blank.gif" width="10" height="1" /></td>
					<td>
						Jump to Page: <select name="page" size="1" onchange="loadpage(this.value)"<?=($pages==1 ? ' disabled="disabled"' : '');?>><?php
							for ($i=1; $i<=$pages; $i++)
							{
								$snum = (($i-1)*$its->per_page);
								$enum = (($i*$its->per_page)>$count ? $count : ($i*$its->per_page))-1;
								$slet = ucwords(strtolower(substr($results[$snum]['itm_title'],0,2)));
								$elet = ucwords(strtolower(substr($results[$enum]['itm_title'],0,2)));
								if (ord(strtoupper(substr($slet,0,1))) < 65 || ord(strtoupper(substr($slet,0,1))) > 90) { $slet = '#'; }
								if (ord(strtoupper(substr($elet,0,1))) < 65 || ord(strtoupper(substr($elet,0,1))) > 90) { $elet = '#'; }
								if ($slet != $elet) { $lets = "($slet-$elet)"; }
								else { $lets = "($slet)"; }

								if ($i == $its->page) { $s = ' selected="selected"'; } else { $s = ''; }
								?><option value="<?=$i;?>"<?=$s;?>><?=$i;?> <?=$lets;?></option><?php
							}
						?></select>
					</td>
					<td><img src="images/blank.gif" width="10" height="1" /></td>
					<td style="display:none">
						# Items Per-Page: <select name="pp" size="1" onchange="perpage(this.value)"><?php
							$vals = array(5,10,25,50);
							while (list($a,$val) = each($vals))
							{
								if ($val == $its->per_page) { $s = ' selected="selected"'; } else { $s = ''; }
								?><option value="<?=$val;?>"<?=$s;?>><?=$val;?></option><?php
							}
						?>
						</select>
					</td>
				</tr>
			</table>
			<?php
		}
		?>

		<form method="post" action="/cartUpdate.php" id="cartForm">
			<input type="hidden" name="act" value="add" />
			<input type="hidden" name="newused" value="-1" />
			<input type="hidden" name="itemID" value="-1" />
		</form>

		<form method="get" action="search.php" name="lp">
		<input type="hidden" name="act" value="search">
		<input type="hidden" name="prev" value="<?=YES;?>">
		<input type="hidden" name="search_type" value="<?=$its->search_type;?>">
		<input type="hidden" name="page" value="">
		<?php
		/*
		<input type="hidden" name="narrow_title" value="<?=@$criteria['title'];?>">
		<?php
		while (list($k,$v) = each($narrow))
		{
			?><input type="hidden" name="narrow_<?=$k;?>" value="<?=$v;?>"><?php
		}
		echo "\n";
		*/
		?>
		</form>
		<script type="text/javascript">
			/*
			function setNarrow(key,val)
			{
				var frm = document.lp;
				frm.page.value = 1;
				frm.elements['narrow_'+key].value = val;
				submitNarrow();
			}
			function submitNarrow() { document.lp.submit(); }
			*/
			function image_window(path,w,h) { open_window(path,'itmimg',w+25,h+25,false); }
			function iteminfo_window(itemID) { open_window('/admin/pos/iteminfo.php?itemID='+itemID+'&store=<?=YES;?>','iteminfo',725,500,'YES',true); }

			// add an item to the shopping cart
			function doAdd(nu,itemID)
			{
				var frm = document.getElementById('cartForm');
				frm.newused.value = nu;
				frm.itemID.value = itemID;
				frm.submit();
			}
		</script>

		<p />
		<?php
		if ($count)
		{
			?>
			<table border="0" cellspacing="1" cellpadding="3" width="95%">
				<tr>
					<td colspan="3">
						<span class="orange_label">&raquo; Search Results</span>
					</td>
				</tr>
				<?php
				$blankimg = '<img src="'.$_SESSION['root'].'images/blank.gif" width="40" height="40" border="0" />';
				$blankimg = '&nbsp;';

				$shown = -1;
				for ($i=$start; $i<$end; $i++)
				{
					$shown++;
					$bg = (($shown%2) ? '#EEEEEE' : '#FFFFFF');


					$arr = $results[$i];
					if (!$arr['itm_box_imgID']) { $image = $blankimg; }
					else
					{
						$itm = new items($its->pg);
						$path = $itm->image_path($arr['itm_box_imgID'],YES);

						if (basename($path) != 'none.gif')
						{
							$nonthumb = $itm->image_path($arr['itm_box_imgID']);

							if (file_exists($nonthumb))
							{
								$tsize = getimagesize($nonthumb);
								$size = getimagesize($path);
								$image = '<a href="javascript:image_window(\'' . $nonthumb . '\',' . $tsize[0] . ',' . $tsize[1] . ')"><img src="' . $path . '" ' . $size[3] . ' border="0" title="View ' . basename($nonthumb) . '" /></a>';
							}
							else
							{
								$image = $blankimg;
							}
						}
						else { $image = $blankimg; }
					}

					?>
					<!-- result #<?=($i+1);?> -->
					<tr bgcolor="<?=$bg;?>">
						<td align="center" valign="middle"><?=$image;?></td>
						<td class="result_title">
							<font size="2"><a href="javascript:iteminfo_window(<?=$arr['itm_itemID'];?>)"><b><?=$arr['itm_title'];?></b></a></font><br />
							<?=$arr['pla_name'];?> - <?=$arr['typ_type'];?>
						</td>
						<!--td>
							<?php
							if (strlen($arr['com1_name'])) { ?><?=$arr['com1_name'];?><br /><?php }
							if (strlen($arr['com2_name'])) { ?><?=$arr['com2_name'];?><br /><?php }
							?>
						</td-->
						<?php
						if (0)
						{
							$arr['qty_new'] = rand(1,10);
							$arr['qty_used'] = rand(1,10);
							$arr['prc_new'] = rand(1,100)+(rand(0,99)/100);
							$arr['prc_used'] = rand(1,100)+(rand(0,99)/100);
						}

						?>
						<td>
							<table border="0" width="100%">
								<?php
									if ($arr['prc_new'] > 0 && $arr['qty_new'] > 0)
									{
										?>
										<tr>
											<td><b>New:</b> <?=$arr['qty_new'];?> Available @ $<?=number_format($arr['prc_new'],2);?></td>
											<td align="right">
												<a href="/cartUpdate.php?act=add&newused=<?=ITEM_NEW;?>&itemID=<?=$arr['itm_itemID'];?>"><img src="/images/btn_addtocart.gif" width="36" height="18" border="0" alt="Add This Item to Your Cart" /></a>
											</td>
										</tr>
										<?php
									}
									if ($arr['prc_used'] > 0 && $arr['qty_used'] > 0)
									{
										?>
										<tr>
											<td><b>Used:</b> <?=$arr['qty_used'];?> Available @ $<?=number_format($arr['prc_used'],2);?></td>
											<td align="right">
												<a href="/cartUpdate.php?act=add&newused=<?=ITEM_USED;?>&itemID=<?=$arr['itm_itemID'];?>"><img src="/images/btn_addtocart.gif" width="36" height="18" border="0" alt="Add This Item to Your Cart" /></a>
											</td>
										</tr>
										<?php
									}
								?>
							</table>
						</td>
					</tr>
					<?php
				}
				?>
			</table>
			<?php
		}
		else
		{
			?>
			Your narrow-by criteria does not match any searched items.
			<p />
			Please modify your criteria or click <a href="/catalog.php">here</a> to search again.
			<?php
		}

		if (0)
		{
			// output the 'narrow by:' form
			$platforms = array();
			$startswith = array();
			$companies = array();
			$types = array();

			if (!$oncatalog)
			{
				// loop through the results
				while (list($a,$arr) = each($results))
				{
					if (!isset($platforms[$arr['itm_platformID']])) { $platforms[$arr['itm_platformID']] = $arr['pla_name']; }
					if (strlen($arr['com1_name']) && !isset($companies[$arr['itm_company1ID']])) { $companies[$arr['itm_company1ID']] = $arr['com1_name']; }
					if (strlen($arr['com2_name']) && !isset($companies[$arr['itm_company2ID']])) { $companies[$arr['itm_company2ID']] = $arr['com2_name']; }
					if (!isset($types[$arr['typ_typeID']])) { $types[$arr['typ_typeID']] = $arr['typ_type']; }
				}
				reset($results);
			}
			else
			{
				// query companies/types for the platform; find beginning letters
				if (!$_SESSION['catalog_prev'])
				{
					$sql = "SELECT com_companyID,com_name FROM companies WHERE com_platformID={$_SESSION['catalog_platformID']} ORDER BY com_name";
					$result = mysql_query($sql,$db);
					$its->error->mysql(__FILE__,__LINE__);
					while ($row = mysql_fetch_assoc($result)) { $companies[$row['com_companyID']] = $row['com_name']; }

					$sql = "SELECT typ_typeID,typ_type FROM types WHERE typ_platformID={$_SESSION['catalog_platformID']} ORDER BY typ_type";
					$result = mysql_query($sql,$db);
					$its->error->mysql(__FILE__,__LINE__);
					while ($row = mysql_fetch_assoc($result)) { $types[$row['typ_typeID']] = $row['typ_type']; }

					if ($result_count)
					{
						$startswith['#'] = '#';
						for ($i=65; $i<90; $i++) { $startswith[chr($i)] = chr($i); }
					}

					$_SESSION['catalog_narrow_startswith'] = $startswith;
					$_SESSION['catalog_narrow_companies'] = $companies;
					$_SESSION['catalog_narrow_types'] = $types;
				}
				else
				{
					$startswith = $_SESSION['catalog_narrow_startswith'];
					$companies = $_SESSION['catalog_narrow_companies'];
					$types = $_SESSION['catalog_narrow_types'];
				}
			}
			asort($platforms);
			asort($companies);
			asort($types);

			$narrowby = array();
			$narrowby[] = array('Platform','platform',$platforms);
			$narrowby[] = array('Starts With','startswith',$startswith);
			$narrowby[] = array('Company','company',$companies);
			$narrowby[] = array('Type','type',$types);

			$total = 0;
			while (list($a,list(,,$vals)) = each($narrowby)) { $total += count($vals); }
			reset($narrowby);

			if ($total > count($narrowby))
			{
				?>
				<p />
				<?=$its->pg->outlineTableHead();?>
				<form onsubmit="return false">
				<tr>
					<td bgcolor="<?=$its->pg->color('table-label');?>"><b>Narrow By:</b></td>
					<?php
					while (list($a,list($name,$key,$vals)) = each($narrowby))
					{
						if (count($vals) > 1)
						{
							?>
							<td bgcolor="<?=$its->pg->color('table-cell');?>">
								<?=$name;?><?=(@$narrow[$key] ? ' (<a href="javascript:setNarrow(\''.$key.'\',\'\')">Remove</a>)' : '');?><br />
								<select name="<?=$key;?>" size="1" onchange="setNarrow('<?=$key;?>',this.value)"><option value=""></option><?php
									while (list($id,$val) = each($vals))
									{
										if ($id == @$narrow[$key] && strlen(@$narrow[$key])) { $s = ' selected="selected"'; } else { $s = ''; }
										?><option value="<?=$id;?>"<?=$s;?>><?=$val;?></option><?php
									}
								?></select>
							</td>
							<?php
						}
					}
					?>
				</tr>
				</form>
				<?=$its->pg->outlineTableFoot();?>
				<p />
				<?php
			}
		}
	}
}

$pg->foot();
?>