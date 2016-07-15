<?php
include('../../include/include.inc');

$pg = new admin_page();
$pg->setTitle('Item Images');
$pg->setFull(NO);
$pg->head('Item Images');

$error = new error('Item Images');

$act = getGP('act');
$platformID = getGP('platformID');
$status = getGP('status');
$imgID = getGP('imgID');
$fromselect = getGP('fromselect');

$type = getGP('type');
if (strlen($type))
{
	$_SESSION['image_type'] = $type;
}
else
{
	$type = $_SESSION['image_type'];
}

$pla = new platforms($pg,$platformID);
?><b>Platform:</b> <?=$pla->platform_name();?><p /><?php

// output a status line, if applicable
if (strlen($status))
{
	$itm = new items($pg);
	if ($imgID)
	{
		$path = $itm->image_path($imgID,YES);
		$st = '<table border="0" cellspacing="0" cellpadding="3"><tr><td><img src="'.$path.'"></td><td>'.$status.'</td></tr></table>';
	}
	else
	{
		$st = $status;
	}
	$pg->status($st);
}

if ($act == "select")
{
	$letter = @$_POST['letter'];
	$title = trim(@$_GET['title']);

	// build list of applicable letters
	$letters = array();
	for ($i=65; $i<=90; $i++) { $letters[$i] = NO; }

	$sql = "SELECT iti_filename FROM item_images WHERE iti_platformID=$platformID";
	$result = mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);
	while ($row = mysql_fetch_assoc($result)) { $letters[ord(strtoupper(substr($row['iti_filename'],0,1)))] = YES; }

	?>
	<script type="text/javascript">
		function set_letter(let)
		{
			var frm = document.letterfrm;
			frm.letter.value = let;
			frm.submit();
		}
	</script>
	<form method="post" action="image.php" name="letterfrm">
	<input type="hidden" name="act" value="select">
	<input type="hidden" name="platformID" value="<?=$platformID;?>">
	<input type="hidden" name="fromselect" value="<?=$fromselect;?>">
	<input type="hidden" name="letter" value="">
	</form>
	<?php

	while (list($ord,$val) = each($letters))
	{
		echo " ";

		if ($val == YES) { ?><a href="javascript:set_letter('<?=chr($ord);?>')" style="text-decoration:underline"><?php }
		else { ?><font color="#BBBBBB"><?php }

		echo chr($ord);

		if ($val == YES) { ?></a><?php }
		else { ?></font><?php }

		echo " ";
	}

	?><hr width="75%" size="-1" color="#CCCCCC"><p /><?php

	if (strlen($title) || strlen($letter))
	{
		// find any possible matches
		if (strlen($title)) { $criteria = '%'.mysql_real_escape_string(implode("%",explode(" ",$title))).'%'; }
		else { $criteria = "$letter%"; }

		$itm = new items($pg);
		$itm->image_search($platformID,$criteria);

		if (count($itm->image_results))
		{
			// there were matches for the title/letter
			?>
			The image<?=(count($itm->image_results)==1?'':'s');?> below <?php
			if (strlen($title)) { ?>match<?=(count($itm->image_results)==1?'es':'');?> the item title <b><?=$title;?></b>.<?php }
			else { ?>begin with the letter <?=strtoupper($letter);?>.<?php }
			?>
			<p />
			Please select the image, a letter, upload a new image below.
			<p />
			<?php
			$itm->image_search_results($platformID,$fromselect);
		}
		else
		{
			// there were no matches for the item title (this will never happen when selecting a letter...)
			?>
			There were no images matching the item title <b><?=$title;?></b>.
			<p />
			Please select a letter above, or upload a new image below.
			<p />
			<?php
		}
	}
	else
	{
		?>
		Please select the first letter of the image you are looking for,<br />
		or upload a new image below.
		<p />
		<?php
	}

	?>
	<p />
	<?php
	uploadForm($platformID,$fromselect);
	?>
	<p />
	<hr width="75%" size="-1" color="#CCCCCC">
	<form method="post" action="image.php" name="noimgform">
	<input type="hidden" name="act" value="doselect">
	<input type="hidden" name="imgID" value="0">
	</form>

	<a href="javascript:document.noimgform.submit()">Click here to select a blank image.</a>
	<?php
}
elseif ($act == "doselect")
{
	?>
	<script type="text/javascript">
		window.opener.document.itemfrm.elements['info[<?php echo $type;?>_imgID]'].value = <?php echo getGP('imgID');?>;
		window.opener.document.itemfrm.refresh.value = <?php echo YES;?>;
		window.opener.document.itemfrm.submit();
		window.close();
	</script>
	<?php
}
elseif ($act == "upload")
{
	if ($fromselect && $imgID)
	{
		?>
		<form method="post" action="image.php" name="selimgform">
		<input type="hidden" name="act" value="doselect">
		<input type="hidden" name="imgID" value="<?=$imgID;?>">
		</form>
		<a href="javascript:document.selimgform.submit()">Click here</a> to select the uploaded image,<br />
		or <a href="image.php?act=select&platformID=<?=$platformID;?>&fromselect=<?php echo $fromselect;?>">click here</a> to return to the image selection page.
		<p />
		<?php
	}

	uploadForm($platformID,$fromselect);
}
elseif ($act == "doupload")
{
	$dir = $_SERVER['DOCUMENT_ROOT'].'/images/items/';
	$saveto = $dir . "platform$platformID/";
	$mogrify = 'start /B "mogrify" ' . $dir . "mogrify/mogrify.exe";

	$extensions = array('jpg','jpeg','gif'); // valid extensions

	$width  = 400; // maximum width of large image
	$twidth  = 40; // maximum width of thumbnail
	$theight = 40; // maximum height of thumbnail

	$count = 1;

	// create the platform image directory if needed
	if (!file_exists($saveto))
	{
		mkdir($saveto) or die('Unable to create save directory! Please contact administrator.');
	}

	if (isset($_FILES['img']))
	{
		$img = $_FILES['img'];
		$orig_ext = array_pop(explode(".",$img['name']));
		$ext = strtolower(array_pop(explode(".",$img['name'])));
		$noextname = implode("_",explode(" ",rtrim(basename($img['name']),".$orig_ext")));

		if (!strlen($img['name']))
		{
			errlink('Please select an image to upload','upload');
		}
		elseif (!in_array($ext,$extensions))
		{
			errlink("Invalid extension: $ext",'upload');
		}
		else
		{
			// copy the file (move the temp file), resize it, and create a thumbnail
			$tmp_name = $img['tmp_name'];

			$ext = strtolower($ext);
			$name = "${noextname}.$ext";
			$tname = "${noextname}_thumb.$ext";

			$path = $saveto.$name;

			$moved = move_uploaded_file($tmp_name,$path);

			if ($moved)
			{
				$tpath = $saveto.$tname;
				$copied = copy($path,$tpath);

				if ($copied)
				{
					// resize the images as needed
					$size = getimagesize($path);
					if ($size[0] < $width) { $width = $size[0]; }

					// resize the large image
					$cmd = $mogrify . " -resize $width $path";
					exec($cmd,$output,$retval);

					if ($retval) { errlink("mogrify error: $output[0]",'upload'); }
					else
					{
						// resize the thumbnail
						$cmd = $mogrify . " -resize $twidth $tpath";
						exec($cmd,$output,$retval);

						// determine if it's too tall or not
						$size = getimagesize($tpath);
						if ($size[1] > $theight)
						{
							// resize the height
							$cmd = $mogrify . " -resize x$theight $tpath";
							exec($cmd,$output,$retval);
						}

						if ($retval) { errlink("mogrify error: $output[0]",'upload'); }
						else
						{
							// image uploaded, thumbnail created; add entry to database
							$dbname = mysql_real_escape_string($name);
							$sql = "DELETE FROM item_images WHERE iti_platformID=$platformID AND iti_filename='$dbname'";
							mysql_query($sql,$db);
							$error->mysql(__FILE__,__LINE__);

							$sql = "INSERT INTO item_images VALUES (NULL,$platformID,'$dbname')";
							mysql_query($sql,$db);
							$error->mysql(__FILE__,__LINE__);

							$imgID = mysql_insert_id();

							if (getP('select'))
							{
								$url = "/admin/setup_items/image.php?act=doselect&imgID=$imgID";
							}
							else
							{
								$url = "/admin/setup_items/image.php?act=upload&platformID=$platformID&status=Image uploaded: $name&imgID=$imgID&fromselect=$fromselect";
							}
							$pg->showUpdating('Image Uploaded',$url);
						}
					}
				}
				else { errlink('Error creating thumbnail file','upload'); }
			}
			else { errlink('Error moving temp file','upload'); }
		}
	}
}
elseif ($act == "delete")
{
	$imgID = $_POST['imgID'];
	$itm = new items($pg);
	$path = $itm->image_path($imgID,NO);
	$tpath = $itm->image_path($imgID,YES);

	$sql = "DELETE FROM item_images WHERE iti_imgID=$imgID";
	mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);

	$sql = "UPDATE ITEMS SET itm_box_imgID=0 WHERE itm_box_imgID=$imgID";
	mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);

	$sql = "UPDATE ITEMS SET itm_nobox_imgID=0 WHERE itm_nobox_imgID=$imgID";
	mysql_query($sql,$db);
	$error->mysql(__FILE__,__LINE__);

	// delete the images
	if (basename($path) != 'none.gif')
	{
		@unlink($path);
		@unlink($tpath);
	}
	else { $path = 'no image'; }

	$path = basename($path);
	$url = "/admin/setup_items/image.php?act=select&platformID=$platformID&fromselect=$fromselect&status=Image deleted: $path";
	$pg->showUpdating('Image Uploaded',$url);
}

if ($act != 'select')
{
	$pg->cancel('javascript:window.close()');
}

function uploadForm($platformID,$fromselect)
{
	global $pg;

	?>
	Select an image to upload by clicking <b>Browse</b> below.<br />
	Please note, you can only upload <b>JPEG</b> and <b>GIF</b> files.
	<p />
	<?php

	$pg->outlineTableHead();
	?>
	<form method="post" enctype="multipart/form-data" action="image.php" name="imgform">
	<input type="hidden" name="act" value="doupload">
	<input type="hidden" name="platformID" value="<?=$platformID;?>">
	<input type="hidden" name="fromselect" value="<?=$fromselect;?>">
	<tr>
		<td bgcolor="<?=$pg->color('table-label');?>"><b>Filename:</b></td>
		<td bgcolor="<?=$pg->color('table-cell');?>"><input type="file" name="img" size="30"></td>
	</tr>
	<?php
	$pg->outlineTableFoot();
	?>

	<p />

	<?php
	if ($fromselect)
	{
		?>
		<input type="checkbox" name="select" id="select" value="<?php echo YES;?>" checked="checked" class="nb" />
		<label for="select">Select this image after uploading</label>
		<p />
		<?php
	}
	?>
	<input type="submit" value="Upload Image &gt;" class="btn">
	</form>
	<?php

	$pg->addOnload('document.imgform.img.focus()');
}

function errlink($err,$act)
{
	global $pg,$platformID,$fromselect;

	$pg->error($err);
	?><a href="image.php?act=<?=$act;?>&platformID=<?=$platformID;?>&fromselect=<?=$fromselect;?>">&lt; Click here to try again</a><?php
}

$pg->foot();
?>