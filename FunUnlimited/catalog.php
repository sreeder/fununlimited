<?php
include('include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

$error = new error('Catalog');

$pg = new page();
$pg->setTitle('Browse Our Catalog');
$pg->head('Browse Our Catalog');

?>
<table border="0" cellspacing="0" cellpadding="0">
	<tr>
		<td align="center" valign="top">
			<span class="orange_label">&raquo; Browse by Platform</span>
			<p />
			<?php
			// find all platforms with items that have quantity and prices
			$timer_class->startTimer('get_platforms');
			$sql = "SELECT * FROM platforms,items,quantity,prices WHERE pla_platformID=itm_platformID AND itm_itemID=qty_itemID AND qty_storeID=1 AND (qty_new>0 OR qty_used>0) AND qty_itemID=prc_itemID AND (prc_new>0 OR prc_used>0)\nGROUP BY itm_platformID\nORDER BY pla_name";
			$result = mysql_query($sql,$db);
			$error->mysql(__FILE__,__LINE__);

			$platforms = array(); // format: $platforms[#] = array(platformID,name,image)
			while ($row = mysql_fetch_assoc($result))
			{
				$platforms[] = array($row['pla_platformID'],$row['pla_name'],$row['pla_image']);
			}
			$timer_class->stopTimer('get_platforms');

			?>
			<script type="text/javascript">
				// select a platform
				function selPlatform(platformID)
				{
					if (platformID)
					{
						document.location = '/search.php?act=search&search_type=<?=ITEM_SEARCH_ADVANCED;?>&platformID=' + platformID;
					}
				}

				// [un]highlight a platform cell
				function hl(platformID)
				{
					document.getElementById('p' + platformID).style.backgroundColor='#EEEEEE';
				}
				function nhl(platformID)
				{
					document.getElementById('p' + platformID).style.backgroundColor='#FFFFFF';
				}
			</script>
			Select:
			<select name="platformID" size="1" onchange="selPlatform(this.value)">
				<option value=""></option>
				<?php
				while (list($a,list($platformID,$name,$image)) = each($platforms))
				{
					?>
					<option value="<?=$platformID;?>"><?=$name;?></option>
					<?php
				}
				reset($platforms);
				?>
			</select>
			<p />
			<table border="0" cellspacing="0" cellpadding="0" class="catalog_browse">
				<?php
				$platforms = array_pad($platforms,(ceil(count($platforms)/3)*3),-1);
				for ($i=0; $i<count($platforms); $i+=3)
				{
					$show = array(
						$platforms[$i],
						$platforms[($i+1)],
						$platforms[($i+2)],
					);
					?>
					<tr class="catalog_browse">
						<?php
						while (list($a,$arr) = each($show))
						{
							if ($arr == -1)
							{
								?><td colspan="2">&nbsp;</td><?php
							}
							else
							{
								list($platformID,$name,$image) = $arr;
								?>
								<td align="center" valign="middle" onmouseover="hl(<?=$platformID;?>)" onmouseout="nhl(<?=$platformID;?>)" onclick="selPlatform(<?=$platformID;?>)">
									<?php
									$maxwidth = 35;
									if (strlen($image) && file_exists("images/platforms/$image"))
									{
										$image = "images/platforms/$image";
										$size = getimagesize($image);

										if ($size[0] <= $maxwidth) { $whline = $size[3]; }
										else
										{
											$height = ceil(($maxwidth/$size[0])*$size[1]);
											$whline = 'width="' . $maxwidth . '" height="' . $height . '"';
										}
									}
									else
									{
										$image = 'images/blank_white.gif';
										$whline = 'width="' . $maxwidth . '" height="30"';
									}
									?>
									<a href="javascript:selPlatform(<?=$platformID;?>)"><img src="<?=$image;?>" <?=$whline;?> border="0" /></a>
								</td>
								<td valign="middle" id="p<?=$platformID;?>" onmouseover="hl(<?=$platformID;?>)" onmouseout="nhl(<?=$platformID;?>)" onclick="selPlatform(<?=$platformID;?>)">
									<a href="javascript:selPlatform(<?=$platformID;?>)"><font size="2"><?=$name;?></font></a>
								</td>
								<?php
							}
						}
						?>
					</tr>
					<?php
				}
				?>
			</table>
		</td>
	</tr>
	<tr>
		<td>
			&nbsp;<br />
			<img src="/images/blank_black.gif" width="550" height="1" /><br />
			&nbsp;
		</td>
	</tr>
	<tr>
		<td align="center" valign="top">
			<span class="orange_label">&raquo; Search Catalog</span>
			<p />
			<?php
			$its = new item_search($pg);
			$its->action = 'search.php';
			$its->advanced_form();
			?>
		</td>
	</tr>
</table>
<?php

$pg->foot();
?>