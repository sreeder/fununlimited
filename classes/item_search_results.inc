<?php
/**
* Item searching - results
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class item_search_results
{
	var $results_act;

	/**
	* Outputs the search results
	* @param integer $deleteID itemID of an item to delete from the results array [optional, default 0]
	*/
	function showResults($deleteID=0)
	{
		$_SESSION['search_page'] = $this->page;

		if ($deleteID)
		{
			$newresults = array();
			foreach ($this->results as $a => $arr)
			{
				if ($arr['itm_itemID'] != $deleteID) { $newresults[] = $arr; }
			}
			$this->results = $newresults;
			$_SESSION['search_results'] = $this->results;
			$_SESSION['search_count'] = count($this->results);
		}

		?>
		<script type="text/javascript">
			var selID = 0;
			function sel(id)
			{
				selID = id;

				var obj = eval('document.itmresults.itm' + id);
				obj.checked = true;

				document.itmresults.s1.disabled = false;
				document.itmresults.s2.disabled = false;
				<?php echo (!strlen($this->findingmatch)&&$this->criteria['active']==YES?'document.itmresults.d1.disabled = false;document.itmresults.d2.disabled = false;':'');?>
				<?php echo (!strlen($this->findingmatch)&&$this->criteria['active']==NO?'document.itmresults.a1.disabled = false;document.itmresults.a2.disabled = false;':'');?>
			}

			function go(where)
			{
				if (!selID) { alert('You must select an item'); }
				else
				{
					if ((where != 'delete' && where != 'undelete') || (where == 'delete' && confirm('Are you SURE you want to delete the selected item?\n\nPlease note: deleting an item sets it as inactive - it is NOT permanent!') == true) || (where == 'undelete' && confirm('Are you SURE you want to activate the selected item?') == true))
					{
						var frm = document.itmresults;
						frm.act.value = where;
						frm.submit();
					}
				}
			}
		</script>

		<?php
		if (strlen($this->findingmatch))
		{
			?>
			Please select the item below which corresponds to the UPC number <b><?php echo $this->findingmatch;?></b>.
			<p />
			If none of these items correspond, you may search again by <a href="items.php?act=search&searchupc=<?php echo $this->findingmatch;?>">clicking here</a>,<br />
			or continue to the add item screen by <a href="items.php?act=add&addupc=<?php echo $this->findingmatch;?>">clicking here</a>.
			<?php
		}
		else
		{
			?>
			<input type="button" value="&lt; Search Again" onclick="document.location='items.php'" class="btn" />
			<?php

			if (count($this->results) == $this->max_results)
			{
				// tell the user they need to narrow their criteria
				?>
				<p />
				<font size="1">
					<b>Note:</b> Your search returned <?php echo $this->max_results;?>+ results. Only the first <?php echo $this->max_results;?> are shown.<br />
					Please narrow your search criteria by clicking <a href="items.php">here</a>.
				</font>
				<?php
			}
		}

		$start = ($this->per_page*($this->page-1));
		$end = ($this->per_page*$this->page);
		if ($end > count($this->results)) { $end = count($this->results); }

		// output the result navigation bar
		$result_count = count($this->results);
		$pages = ceil($result_count/$this->per_page);
		?>
		<script type="text/javascript">
			var page = <?php echo $this->page;?>;
			var pages = <?php echo $pages;?>;
			function loadpage(num) { if (num != page) { var frm = document.lp; frm.page.value = num; frm.submit(); } }
			function goprev() { if ((page-1)) { loadpage((page-1)); } }
			function gonext() { if ((page+1) <= <?php echo $pages;?>) { loadpage((page+1)); } }
		</script>
		<form method="get" action="<?php echo $this->getAction();?>" name="lp">
			<input type="hidden" name="act" value="page">
			<input type="hidden" name="findingmatch" value="<?php echo $this->findingmatch;?>">
			<input type="hidden" name="page" value="">
		</form>

		<p />
		<table border="0" cellspacing="0" cellpadding="2">
			<tr>
				<td align="right">
					<a href="javascript:loadpage(1)" title="First Page"><img src="/images/search_first.gif" width="60" height="18" border="0" /></a>
					<a href="javascript:goprev()" title="Previous Page"><img src="/images/search_prev.gif" width="48" height="18" border="0" /></a>
				</td>
				<td align="center" valign="middle" style="font-size:12">
					<?php

					$padby = 3;

					$ps = (($this->page-$padby)<1?1:($this->page-$padby));
					$pe = (($this->page+$padby)>$pages?$pages:($this->page+$padby));

					if ($pe > $pages) { $pe = $pages; }

					if ($ps > 1)
					{
						?><a href="javascript:loadpage(1)" style="text-decoration:underline" title="Results 1-<?php echo $this->per_page;?>">1</a> <?php
						if (($this->page-$padby) > 2) { echo "... "; }
					}

					$links = array();
					for ($j=$ps; $j<=$pe; $j++)
					{
						$resnum = ((($j-1)*$this->per_page)+1).'-'.(($j*$this->per_page)>$result_count?$result_count:($j*$this->per_page));

						$str = '';
						if ($j != $this->page) { $str .= '<a href="javascript:loadpage('.$j.')" style="text-decoration:underline" title="Results '.$resnum.'">'; } else { $str .= "<b>"; }
						$str .= $j;
						if ($j != $this->page) { $str .= '</a>'; } else { $str .= "</b>"; }
						$links[] = $str;
					}
					echo implode(" ", $links);

					if ($pe != $pages)
					{
						$lastresnum = ((($pages-1)*$this->per_page)+1).'-'.$result_count;
						if (($this->page+$padby) < ($pages-1)) { echo " ..."; }
						?> <a href="javascript:loadpage(<?php echo $pages;?>)" style="text-decoration:underline" title="Results <?php echo $lastresnum;?>"><?php echo $pages;?></a><?php
					}

					?>
				</td>
				<td align="left">
					<a href="javascript:gonext()" title="Next Page"><img src="/images/search_next.gif" width="48" height="18" border="0" /></a>
					<a href="javascript:loadpage(<?php echo $pages;?>)" title="Last Page"><img src="/images/search_last.gif" width="60" height="18" border="0" /></a>
				</td>
				<td><img src="/images/blank.gif" width="10" height="1" /></td>
				<td>
					Jump to Page: <select name="page" size="1" onchange="loadpage(this.value)"<?php echo ($pages==1?' disabled="disabled"':'');?>><?php
						for ($i=1; $i<=$pages; $i++)
						{
							$snum = (($i-1)*$this->per_page);
							$enum = (($i*$this->per_page)>$result_count?$result_count:($i*$this->per_page))-1;
							$slet = ucwords(strtolower(substr($this->results[$snum]['itm_title'],0,2)));
							$elet = ucwords(strtolower(substr($this->results[$enum]['itm_title'],0,2)));
							if (ord(strtoupper(substr($slet,0,1))) < 65 || ord(strtoupper(substr($slet,0,1))) > 90) { $slet = '#'; }
							if (ord(strtoupper(substr($elet,0,1))) < 65 || ord(strtoupper(substr($elet,0,1))) > 90) { $elet = '#'; }
							if ($slet != $elet) { $lets = "($slet-$elet)"; }
							else { $lets = "($slet)"; }

							if ($i == $this->page) { $s = ' selected="selected"'; } else { $s = ''; }
							?><option value="<?php echo $i;?>"<?php echo $s;?>><?php echo $i;?> <?php echo $lets;?></option><?php
						}
					?></select>
				</td>
			</tr>
		</table>

		<p />
		<form method="get" action="<?php echo $this->getAction();?>" name="itmresults">
		<input type="hidden" name="act" value="edit">
		<input type="hidden" name="findingmatch" value="<?php echo $this->findingmatch;?>">

		<input type="button" value="Select &gt;" id="s1" disabled="disabled" onclick="go('edit')" class="btn">
		<?php
		if (!strlen($this->findingmatch))
		{
			if ($this->criteria['active'] == YES) { ?> <input type="button" value="Delete &gt;" id="d1" disabled="disabled" onclick="go('delete')" class="btn"><?php }
			else { ?> <input type="button" value="Activate &gt;" id="a1" disabled="disabled" onclick="go('undelete')" class="btn"><?php }
		}
		?>

		<p />

		<?php echo $this->pg->outlineTableHead('98%');?>
		<tr bgcolor="<?php echo $this->pg->color('table-head');?>">
			<td>&nbsp;</td>
			<td>&nbsp;</td>
			<td><b>Year</b></td>
			<td width="100%"><b>Title</b></td>
			<td><b>Platform</b></td>
			<td><b>Company</b></td>
			<td><b>UPC</b></td>
		</tr>

		<?php
		$blank = '<img src="'.$_SESSION['root'].'images/blank.gif" width="40" height="1"';

		$shown = -1;
		for ($i=$start; $i<$end; $i++)
		{
			$arr = $this->results[$i];
			foreach ($arr as $k => $v) { $arr[$k] = stripslashes(stripslashes($v)); }

			$shown++;

			$image_box = $blank;
			if ($arr['itm_box_imgID'])
			{
				$itm = new items($this->pg);
				$path = $itm->image_path($arr['itm_box_imgID'],YES);
				if (basename($path) != 'none.gif' && file_exists($path))
				{
					$size = getimagesize($path);
					$image_box = '<img src="'.$path.'" '.$size[3];
				}
			}
			$image_box .= ' alt="Box Image" onclick="sel('.$arr['itm_itemID'].')">';

			$image_nobox = $blank;
			if ($arr['itm_nobox_imgID'])
			{
				$itm = new items($this->pg);
				$path = $itm->image_path($arr['itm_nobox_imgID'],YES);
				if (basename($path) != 'none.gif' && file_exists($path))
				{
					$size = getimagesize($path);
					$image_nobox = '<img src="'.$path.'" '.$size[3];
				}
			}
			$image_nobox .= ' alt="No Box Image" onclick="sel('.$arr['itm_itemID'].')">';

			$bg = ((($shown%2))?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));
			?>
			<label for="itm<?php echo $arr['itm_itemID'];?>">
			<tr style="cursor:hand" bgcolor="<?php echo $bg;?>">
				<td><input type="radio" name="itemID" value="<?php echo $arr['itm_itemID'];?>" id="itm<?php echo $arr['itm_itemID'];?>" class="nb" onclick="sel(<?php echo $arr['itm_itemID'];?>)"></td>
				<td valign="top"><?php echo $image_box . $image_nobox;?></td>
				<td><?php echo $arr['yer_year'];?></td>
				<td style="white-space:normal"><?php echo $arr['itm_title'];?></td>
				<td><?php echo $arr['pla_name'];?></td>
				<td><?php echo $arr['com1_name'];?><br /><?php echo $arr['com2_name'];?></td>
				<td><?php echo $arr['itm_upc'];?></td>
			</tr>
			</label>
			<?php
		}
		?>
		<?php echo $this->pg->outlineTableFoot();?>

		<p />

		<input type="button" value="Select &gt;" id="s2" disabled="disabled" onclick="go('edit')" class="btn">
		<?php
		if (!strlen($this->findingmatch))
		{
			if ($this->criteria['active'] == YES) { ?> <input type="button" value="Delete &gt;" id="d2" disabled="disabled" onclick="go('delete')" class="btn"><?php }
			else { ?> <input type="button" value="Activate &gt;" id="a2" disabled="disabled" onclick="go('undelete')" class="btn"><?php }
		}
		?>
		</form>
		<?php
	}

	/**
	* Outputs the small item results table
	* @param integer $selmultiple can select multiple items? If YES, checkboxes used; if NO, radio
	* @param array $hidden array of hidden variables (format: 'key'=>'val') [optional, default '']
	* @param integer $showqtyprice show quantity/price info for each item? [optional, default NO]
	* @param integer $cansetupc show the 'SET UPC' button? [optional, default YES]
	*/
	function showSmallResults($selmultiple, $hidden=array(), $showqtyprice=YES, $cansetupc=YES)
	{
		$quickadd = (@$_SESSION['do_quickadd'] ? YES : NO);
		$quickadd_itemIDs = ($quickadd ? $_SESSION['quickadd_itemIDs'] : array());
		?>
		<p />
		<font size="1"><b>Note:</b> Exact matches (if any) are listed first.</font>
		<script type="text/javascript">
			var selID = 0;
			var selIDs = new Array();
			function sel(id)
			{
				selID = id;

				var obj = eval('document.itmresults.itm' + id);
				<?php
				if ($selmultiple)
				{
					?>
					if (obj.checked && !in_array(id,selIDs)) { selIDs[selIDs.length] = id; }
					else
					{
						selIDs = array_remove(id,selIDs);
					}

					if (!selIDs.length) { selID = 0; }
					<?php
				} // if allow selecting multiple
				else
				{
					?>obj.checked = true;<?php
				}
				?>

				document.itmresults.s1.disabled = (<?php echo $selmultiple;?>&&!selIDs.length?true:false);
				document.itmresults.s2.disabled = (<?php echo $selmultiple;?>&&!selIDs.length?true:false);
			}

			function go()
			{
				if (<?php echo jstf($quickadd);?>) { document.itmresults.submit(); }
				else if (<?php echo ($selmultiple?'!selIDs.length':'!selID');?>) { alert('You must select an item'); }
				else { document.itmresults.submit(); }
			}

			// set the UPC for an item
			var itemIDs = new Array();
			var platforms = new Array();
			var titles = new Array();
			var upcs = new Array();
			function set_upc(idx,fillupc)
			{
				var itemID = itemIDs[idx];
				var title = titles[idx];
				var upc = prompt('Please enter the UPC for the item titled: ' + title,fillupc);

				if (upc != null)
				{
					var existsidx = array_search(upc,upcs);

					if (upc == '' || existsidx == -1)
					{
						setupcifrm.location = '/admin/setup_items/begqty_setupc.php?idx=' + idx + '&itemID=' + itemID + '&upc=' + upc + '&change_results=<?php echo YES;?>';
					}
					else
					{
						upc_exists(upc,title,platforms[idx]);
					}
				}
			}

			// alert that the UPC is already set
			function upc_exists(upc,title,platform)
			{
				alert('The UPC ' + upc + ' already exists\n\nPlatform: ' + platform + '\nItem: ' + title);
			}

			// change the shown UPC for a given item
			function change_upc(idx,upc)
			{
				var obj = document.getElementById('upc' + idx);
				upcs[idx] = upc;
				obj.innerText = upc;
			}

			function unlock_upc() { } // this is called by begqty_setupc.php
		</script>

		<iframe name="setupcifrm" src="/admin/setup_items/begqty_setupc.php" width="1" height="1" frameborder="0" marginwidth="0" marginheight="0">
			Your browser does not support iframes. Please upgrade.
		</iframe>

		<p />
		<form method="get" action="<?php echo $this->getAction();?>" name="itmresults">
		<input type="hidden" name="act" value="<?php echo $this->results_act;?>">
		<?php
		foreach ($hidden as $key => $val)
		{
			?>
			<input type="hidden" name="<?php echo $key;?>" value="<?php echo $val;?>">
			<?php
		}

		$colspan = ($cansetupc?8:7);

		?>
		<script type="text/javascript">
			if (window.opener && window.opener != self)
			{
				document.write('<input type="button" value="&lt; Close" id="c1" onclick="self.close()" class="btn">');
			}
		</script>
		<input type="button" value="Select &gt;" id="s1" disabled="disabled" onclick="go()" class="btn">
		<p />
		<?php

		$this->pg->outlineTableHead();
		?>
		<tr bgcolor="<?php echo $this->pg->color('table-head');?>">
			<td>&nbsp;</td>
			<td>&nbsp;</td>
			<td><b>Year</b></td>
			<td><b>Title</b></td>
			<td><b>Platform</b></td>
			<td><b>Company</b></td>
			<td colspan="<?php echo ($cansetupc?2:1);?>"><b>UPC</b></td>
			<?php
			if ($showqtyprice)
			{
				?>
				<td align="center" colspan="2"><b>New</b></td>
				<td align="center" colspan="2"><b>Used</b></td>
				<?php

				$colspan += 4;
			}
			?>
		</tr>

		<?php
		$blank = '<img src="'.$_SESSION['root'].'images/blank.gif" width="40" height="1"';

		$shown = -1;
		foreach ($this->results as $a => $arr)
		{
			$shown++;

			foreach ($arr as $k => $v) { $arr[$k] = stripslashes(stripslashes($v)); }

			$itemID = $arr['itm_itemID'];

			$image_box = $blank;
			if ($arr['itm_box_imgID'])
			{
				$itm = new items($this->pg);
				$path = $itm->image_path($arr['itm_box_imgID'],YES);
				if (basename($path) != 'none.gif' && file_exists($path))
				{
					$size = getimagesize($path);
					$image_box = '<img src="'.$path.'" '.$size[3];
				}
			}
			$image_box .= ' alt="Box Image" onclick="sel('.$arr['itm_itemID'].')">';

			$image_nobox = $blank;
			if ($arr['itm_nobox_imgID'])
			{
				$itm = new items($this->pg);
				$path = $itm->image_path($arr['itm_nobox_imgID'],YES);
				if (basename($path) != 'none.gif' && file_exists($path))
				{
					$size = getimagesize($path);
					$image_nobox = '<img src="'.$path.'" '.$size[3];
				}
			}
			$image_nobox .= ' alt="No Box Image" onclick="sel('.$arr['itm_itemID'].')">';

			$bg = (!($shown%2) ? $this->pg->color('table-cell') : $this->pg->color('table-cell2'));
			?>
			<script type="text/javascript">
				itemIDs[<?php echo $a;?>]   = <?php echo $itemID;?>;
				platforms[<?php echo $a;?>] = '<?php echo mysql_escape_string($arr['pla_name']);?>';
				titles[<?php echo $a;?>]    = '<?php echo mysql_escape_string($arr['itm_title']);?>';
				upcs[<?php echo $a;?>]      = '<?php echo mysql_escape_string($arr['itm_upc']);?>';
			</script>
			<label for="itm<?php echo $itemID;?>">
			<tr style="cursor:hand" bgcolor="<?php echo $bg;?>">
				<td>
					<?php
					if ($selmultiple)
					{
						// checkbox
						$chk = (@$quickadd && in_array($itemID, $quickadd_itemIDs) ? ' checked="checked"' : '');
						?>
						<input type="checkbox" name="itemID[]" value="<?php echo $itemID;?>" id="itm<?php echo $itemID;?>" class="nb" onclick="sel(<?php echo $itemID;?>)"<?php echo $chk;?> />
						<?php
					}
					else
					{
						// radio
						?>
						<input type="radio" name="itemID" value="<?php echo $itemID;?>" id="itm<?php echo $itemID;?>" class="nb" onclick="sel(<?php echo $itemID;?>)" />
						<?php
					}
					?>
				</td>
				<td valign="top"><?php echo $image_box . $image_nobox;?></td>
				<td><?php echo $arr['yer_year'];?></td>
				<td style="white-space:normal"><?php echo $arr['itm_title'];?></td>
				<td><?php echo $arr['pla_name'];?></td>
				<td><?php echo $arr['com1_name'];?><br /><?php echo $arr['com2_name'];?></td>
				<td><div id="upc<?php echo $a;?>"><?php
					if (strlen($arr['itm_upc'])) { echo $arr['itm_upc']; } else { echo "&nbsp;"; }
				?></div></td>
				<?php
				if ($cansetupc) { ?><td><div style="width:100%;text-align:right"><a href="javascript:set_upc(<?php echo $a;?>, '<?php echo $arr['itm_upc'];?>')"><img src="/images/setupc.gif" width="54" height="11" border="0" /></a></div></td><?php }
				if ($showqtyprice)
				{
					?>
					<td align="right"><?php echo $arr['qty_new'];?></td>
					<td align="right">$<?php echo number_format($arr['prc_new'],2);?></td>
					<td align="right"><?php echo $arr['qty_used'];?></td>
					<td align="right">$<?php echo number_format($arr['prc_used'],2);?></td>
					<?php
				}
				?>
			</tr>
			</label>
			<?php
		}
		?>
		<?php echo $this->pg->outlineTableFoot();?>

		<p />

		<script type="text/javascript">
			if (window.opener && window.opener != self)
			{
				document.write('<input type="button" value="&lt; Close" id="c1" onclick="self.close()" class="btn">');
			}
		</script>
		<input type="button" value="Select &gt;" id="s2" disabled="disabled" onclick="go()" class="btn">
		</form>
		<?php

		if ($quickadd)
		{
			$this->pg->addOnload('go()');
		}
	} // end function showSmallResults
} // end class item_search_results

?>