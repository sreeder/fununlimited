<?php
/**
* Setup for items (adding/editing/removing)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class items
{
	/**
	* page class reference
	* @var reference $pg
	*/
	var $pg;

	/**
	* The current itemID
	* @var integer $itemID
	*/
	var $itemID;

	/**
	* Information for the current item
	* @var array $info
	*/
	var $info;

	/**
	* Status variable used for error reporting (bad email, bad zip code, success, etc)
	* @var array $status
	*/
	var $status;

	/**
	* Image search results
	* @var array $image_results
	*/
	var $image_results;

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
	function items(&$pg)
	{
		$this->timer =& $GLOBALS['timer_class'];
		$this->pg =& $pg;
		$this->itemID = 0;
		$this->items = array();
		$this->info = array(
			'features'           => array(),
			'item_source_values' => array(),
			'item_field_values'  => array(),
			'price'              => array()
		);

		$this->error = new error('Items');
	}

	/**
	* Pull in item information
	* @param	mixed		$itemIDs
	* @param	boolean	$only_image	only pull the image info? [optional, default NO]
	* @access	public
	*/
	function setItems($itemIDs,$only_image=NO)
	{
		global $db;

		$this->timer->startTimer(__CLASS__.'::'.__FUNCTION__);

		$this->items = array();
		$imageIDs = array();
		if (!is_array($itemIDs)) { $itemIDs = array($itemIDs); }

		$where = array(
			'itm_itemID IN ' . getIn($itemIDs),
			'itm_yearID=yer_yearID',
			'itm_platformID=pla_platformID',
			'itm_typeID=typ_typeID',
		);

		$sql = 'SELECT * FROM items,years,platforms,types WHERE ' . implode(' AND ',$where);
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		$merge = array(
			'features'           => array(),
			'item_source_values' => array(),
			'item_field_values'  => array(),
			'price'              => array(),
			'quantity'           => array(),
			'qty_new'            => 0,
			'qty_used'           => 0,
			'image_box'          => array('FULL'=>'', 'THUMB'=>''),
			'image_nobox'        => array('FULL'=>'', 'THUMB'=>'')
		);
		while ($row = mysql_fetch_assoc($result))
		{
			print_r($row);
			$imageIDs[] = $row['itm_box_imgID'];
			$imageIDs[] = $row['itm_nobox_imgID'];
			$row = array_merge($row,$merge);
			$this->items[$row['itm_itemID']] = $row;
		}

		$itemIDs = array_keys($this->items);

		if (count($itemIDs))
		{
			if (!$only_image)
			{
				$in = getIn($itemIDs);
				$sql = "SELECT * FROM itm_fet_rel WHERE ifr_itemID IN $in";
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
				while ($row = mysql_fetch_assoc($result))
				{
					$this->items[$row['ifr_itemID']]['features'][] = $row['ifr_featureID'];
				}

				$sql = "SELECT * FROM prices WHERE prc_itemID IN $in";
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
				while ($row = mysql_fetch_assoc($result))
				{
					$this->items[$row['prc_itemID']]['price'][ITEM_NEW] = $row['prc_new'];
					$this->items[$row['prc_itemID']]['price'][ITEM_USED] = $row['prc_used'];
				}

				$sql = "SELECT * FROM item_source_values WHERE isv_itemID IN $in";
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
				while ($row = mysql_fetch_assoc($result))
				{
					$this->items[$row['isv_itemID']]['item_source_values'][$row['isv_sourceID']] = $row['isv_value'];
				}

				$sql = "SELECT * FROM item_field_values WHERE ifv_itemID IN $in";
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
				while ($row = mysql_fetch_assoc($result))
				{
					$this->items[$row['ifv_itemID']]['item_field_values'][$row['ifv_fieldID']] = $row['ifv_value'];
				}

				$sql = "SELECT * FROM quantity WHERE qty_itemID IN $in AND qty_storeID=" . $_SESSION['storeID'];
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
				while ($row = mysql_fetch_assoc($result))
				{
					$this->items[$row['qty_itemID']]['qty_new'] = $row['qty_new'];
					$this->items[$row['qty_itemID']]['qty_used'] = $row['qty_used'];
					$this->items[$row['qty_itemID']]['quantity'] = array(
						ITEM_NEW  => $row['qty_new'],
						ITEM_USED => $row['qty_used']
					);
				}
			}

			// images
			if (array_sum($imageIDs))
			{
				$images = array();
				$sql = 'SELECT * FROM item_images WHERE iti_imgID IN ' . getIn($imageIDs);
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
				while ($row = mysql_fetch_assoc($result))
				{
					$dir = '/images/items/platform' . $row['iti_platformID'] . '/';
					$file = $row['iti_filename'];
					$boom = explode('.',$file);
					$ext = array_pop($boom);
					$pre = implode('.',$boom);
					$images[$row['iti_imgID']] = array(
						'FULL'  => $dir . $row['iti_filename'],
						'THUMB' => $dir . $pre . '_thumb.' . $ext
					);
				}

				while (list($itemID,$arr) = each($this->items))
				{
					if (isset($images[$arr['itm_box_imgID']]))
					{
						$this->items[$itemID]['image_box'] = $images[$arr['itm_box_imgID']];
					}
					if (isset($images[$arr['itm_nobox_imgID']]))
					{
						$this->items[$itemID]['image_nobox'] = $images[$arr['itm_nobox_imgID']];
					}
				}
				reset($this->items);
			}
		}

		$this->timer->stopTimer(__CLASS__.'::'.__FUNCTION__);
	}

	/**
	* Return one/all item information
	* @param integer $itemID [optional, default 0]
	* @return array
	* @access public
	*/
	function getItems($itemID=0)
	{
		if ($itemID) { return (isset($this->items[$itemID]) ? $this->items[$itemID] : array()); }
		else { return $this->items; }
	}

	/**
	* Pulls in information for an item
	* @param integer $itemID
	*/
	function set_itemID($itemID)
	{
		global $db;

		$this->itemID = $itemID;
		$this->info = array('features'=>array(),'item_source_values'=>array(),'item_field_values'=>array(),'price'=>array());

		if (strlen($itemID))
		{
			$sql = "SELECT * FROM items,platforms WHERE itm_itemID=$itemID AND itm_platformID=pla_platformID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			if (mysql_num_rows($result))
			{
				$row = mysql_fetch_assoc($result);
				while (list($key,$val) = each($row))
				{
					$this->info[substr($key,4)] = stripslashes($val);
				}
			}

			$sql = "SELECT * FROM itm_fet_rel WHERE ifr_itemID=$itemID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			if (mysql_num_rows($result))
			{
				while ($row = mysql_fetch_assoc($result))
				{
					$this->info['features'][] = $row['ifr_featureID'];
				}
			}

			$sql = "SELECT prc_new,prc_used FROM prices WHERE prc_itemID=$itemID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			if (mysql_num_rows($result))
			{
				$row = mysql_fetch_assoc($result);
				$this->info['price'][ITEM_NEW] = $row['prc_new'];
				$this->info['price'][ITEM_USED] = $row['prc_used'];
			}

			$sql = "SELECT * FROM item_source_values WHERE isv_itemID=$itemID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			if (mysql_num_rows($result))
			{
				while ($row = mysql_fetch_assoc($result))
				{
					$this->info['item_source_values'][$row['isv_sourceID']] = $row['isv_value'];
				}
			}

			$sql = "SELECT * FROM item_field_values WHERE ifv_itemID=$itemID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			if (mysql_num_rows($result))
			{
				while ($row = mysql_fetch_assoc($result))
				{
					$this->info['item_field_values'][$row['ifv_fieldID']] = $row['ifv_value'];
				}
			}

			$sql = "SELECT * FROM quantity WHERE qty_itemID=$itemID AND qty_storeID=".$_SESSION['storeID'];
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			if (mysql_num_rows($result))
			{
				$row = mysql_fetch_assoc($result);
				$this->info['qty_new'] = $row['qty_new'];
				$this->info['qty_used'] = $row['qty_used'];
			}
		}
	}

	/**
	* Copy search criteria entries into $this->info
	* @param array $criteria
	*/
	function criteria_to_info($criteria)
	{
		$oldinfo = $this->info;
		$this->info = $criteria;
		if (!isset($this->info['features'])) { $this->info['features'] = array(); }

		if (is_array($oldinfo) && count($oldinfo))
		{
			while (list($k,$v) = each($oldinfo))
			{
				if (!is_array(@$this->info[$k]) && !strlen(@$this->info[$k])) { $this->info[$k] = $v; }
			}
		}
	}

	/**
	* Displays the platform selection form
	* @param string $action Form action
	* @param string $randID A random ID (used for pulling in searched data when the add form comes up) [optional, default '']
	* @param integer $platformID platformID to select by default [optional, default '']
	* @param string $setupc UPC to fill in on the item info screen [optional, default '']
	* @param integer $fromreceiveorder Are they coming from the receive order page? [optional, default NO]
	* @param integer $fromreturns Are they coming from the return page? [optional, default NO]
	*/
	function platform_form($action,$randID="",$platformID="",$setupc="",$fromreceiveorder=NO,$fromreturns=NO)
	{
		$pla = new platforms($this->pg,0);

		?>
		Please select a platform below for the new item:
		<p />

		<script type="text/javascript">
			function verify(frm)
			{
				if (frm.platformID.options[frm.platformID.selectedIndex].value == '') { alert('Please select a platform'); return false; }
				else { return true; }
			}
		</script>
		<?php

		$this->pg->outlineTableHead();
		?>

		<form method="post" action="<?=$action;?>" name="itemfrm" id="itemfrm" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="selplatform">
		<input type="hidden" name="itemID" value="<?=$this->itemID;?>">
		<input type="hidden" name="findingmatch" value="<?=$this->info['upc'];?>">
		<input type="hidden" name="randID" value="<?=$randID;?>">
		<input type="hidden" name="setupc" value="<?=$setupc;?>">
		<input type="hidden" name="fromreceiveorder" value="<?=$fromreceiveorder;?>">
		<input type="hidden" name="fromreturns" value="<?=$fromreturns;?>">
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Platform:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<table border="0" cellspacing="0" cellpadding="0" width="100%">
					<tr>
						<td>
							<select name="platformID" size="1" style="vertical-align:middle" onchange="this.form.submit()">
								<option value=""></option>
								<?php
								$pla->set_item('platforms');
								while (list($a,$arr) = each($pla->values))
								{
									if ($arr[0] == $platformID) { $s = ' selected="selected"'; } else { $s = ''; }
									?>
									<option value="<?=$arr[0];?>"<?=$s;?>><?=$arr[1];?></option>
									<?php
								}
								?>
							</select>
						</td>
					</tr>
				</table>
			</td>
		</tr>
		<?php
		$this->pg->outlineTableFoot();

		?>
		<p />
		<input type="submit" value="Select Platform &gt;" class="btn">
		</form>
		<?php
	}

	/**
	* Displays the add/edit form
	* @param	string	$msg				[optional, default '']
	* @param	string	$moveto				[optional, default '']
	* @param	boolean	$frominvoice		coming from invoice? if YES, provides link back to invoice at each stage [optional, default NO]
	* @param	boolean	$fromupc			coming from UPC setup? if YES, provides link back to UPC setup at each stage [optional, default NO]
	* @param	boolean	$fromqty			coming from whole platform quantity setup? if YES, provides link back to whole platform quantity setup at each stage [optional, default NO]
	* @param	boolean	$fromreceiveorder	coming from receive order? if YES, provides link back to receive order at each stage [optional, default NO]
	* @param	boolean	$fromreturns		coming from returns? if YES, provides link back to returns at each stage [optional, default NO]
	* @param	boolean	$popup				in a popup? [optional, default NO]
	*/
	function add_form($msg="",$moveto="",$frominvoice=NO,$fromupc=NO,$fromqty=NO,$fromreceiveorder=NO,$fromreturns=NO,$popup=YES)
	{
		if ($this->itemID) { $word = "edit"; } else { $word = "add"; }

		if (strlen(@$_POST['randID']) && strlen(@$_SESSION['add_key']) && $_POST['randID'] == $_SESSION['add_key'])
		{
			$this->criteria_to_info($_SESSION['add_criteria']);
			//unset($_SESSION['add_key']);
			//unset($_SESSION['add_criteria']);
		}

		if (!is_array(@$this->info['features'])) { $this->info['features'] = array(); }

		$this->pg->pageHead(ucwords("$word Item"),3);

		if (strlen($msg)) { echo "$msg<p />"; }

		if (!strlen(@$this->info['percopy'])) { $this->info['percopy'] = YES; }

		if (!strlen(@$this->info['active'])) { $this->info['active'] = YES; }
		$ayc = ($this->info['active']==YES?' checked="checked"':'');
		$anc = ($this->info['active']==NO?' checked="checked"':'');

		if (count($this->status)) { $this->show_errors(); }

		// output the form/table
		?>
		<script type="text/javascript">
			var unknownidx = -1;

			function set_unknown()
			{
				var obj = document.itemfrm.elements['info[typeID]'];

				for (var i=0; i<obj.options.length; i++)
				{
					if (obj.options[i].text == 'Unknown Type') { unknownidx = i; break; }
				}

				if (unknownidx == -1) { alert('IDX for Unknown Type could not be found!\n\nPlease tell Scott!'); }
			}

			function verify(frm)
			{
				var ret = true;
				var foc = '';

				if (frm.elements['info[title]'].value == '') { alert('You must enter a title.'); ret = false; foc = 'title'; }
				<?php
				/*
				!!! just require a title for now !!!

				if (!$fromqty)
				{
					?>
					else if (frm.elements['info[yearID]'].options[frm.elements['info[yearID]'].selectedIndex].value == '') { alert('You must select a year.'); ret = false; foc = 'yearID'; }
					else if (frm.elements['info[company1ID]'].options[frm.elements['info[company1ID]'].selectedIndex].value == '') { alert('You must select a company.'); ret = false; foc = 'company1ID'; }
					else if (frm.elements['info[typeID]'].options[frm.elements['info[typeID]'].selectedIndex].value == '') { alert('You must select a type.'); ret = false; foc = 'typeID'; }
					<?php
				}
				else
				{
					*/
					?>
					if (frm.elements['info[yearID]'].options[frm.elements['info[yearID]'].selectedIndex].value == '') { frm.elements['info[yearID]'].selectedIndex = frm.elements['info[yearID]'].options.length-1; }
					if (frm.elements['info[typeID]'].options[frm.elements['info[typeID]'].selectedIndex].value == '') { frm.elements['info[typeID]'].selectedIndex = unknownidx; }
					<?php
					/*
				}
				*/
				?>

				if (ret == true && frm.elements['info[upc]'].value == '') { ret = confirm('You didn\'t enter a UPC number. Are you sure you would like to <?=$word;?> the item?\n\nOK = YES, Cancel = No'); }

				if (ret == true) { return true; }
				else
				{
					if (foc.length) { eval("frm.elements['info["+foc+"]'].focus()"); }
					return false;
				}
			}

			function add(item)
			{
				open_window('/admin/setup_items/platforms.php?act=add&platformID=<?=$this->info['platformID'];?>&parentform=itemfrm&popup=<?=YES;?>&item='+item,'setupwin',725,275,'YES');
			}

			function chgimg(id,path,w,h)
			{
				var frm = document.itemfrm;
				frm.elements['info[box_imgID]'].value = id;
				document.itemimg.src = path;
				document.itemimg.width = w;
				document.itemimg.height = h;
			}

			function image(act, img_type)
			{
				open_window('/admin/setup_items/image.php?fromselect=<?php echo YES;?>&act=' + act + '&type=' + img_type + '&platformID=<?php echo $this->info['platformID'];?>&title=' + document.itemfrm.elements['info[title]'].value,'imagewin',725,450,'YES',true);
			}
		</script>

		<form method="post" action="items.php" name="itemfrm" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="do<?=$word;?>">
		<input type="hidden" name="itemID" value="<?=$this->itemID;?>">
		<input type="hidden" name="refresh" value="<?=NO;?>">
		<input type="hidden" name="refreshto" value="">
		<input type="hidden" name="frominvoice" value="<?=$frominvoice;?>">
		<input type="hidden" name="fromupc" value="<?=$fromupc;?>">
		<input type="hidden" name="fromqty" value="<?=$fromqty;?>">
		<input type="hidden" name="fromreceiveorder" value="<?=$fromreceiveorder;?>">
		<input type="hidden" name="fromreturns" value="<?=$fromreturns;?>">
		<input type="hidden" name="popup" value="<?php echo $popup;?>">

		<?php
		if ($frominvoice)
		{
			?><input type="button" value="&lt; Return to Invoice" onclick="document.location='/admin/pos/invoice.php?act=view'" class="btn"><?php
		}
		if ($fromupc)
		{
			?><input type="button" value="&lt; Return to UPC Setup" onclick="document.location='/admin/setup_items/enterupc.php?act=showitems&platformID=<?=$this->info['platformID'];?>'" class="btn"><?php
		}
		if ($fromqty)
		{
			?><input type="button" value="&lt; Return to Whole Platform Quantity Setup" onclick="document.location='/admin/setup_items/begqty.php?act=showitems&platformID=<?=$this->info['platformID'];?>'" class="btn"><?php
		}
		if ($fromreceiveorder)
		{
			?><input type="button" value="&lt; Return to Receive Order" onclick="document.location='/admin/utilities/receive_order.php'" class="btn"><?php
		}
		if ($fromreturns)
		{
			?><input type="button" value="&lt; Return to Returns" onclick="document.location='/admin/pos/returns.php'" class="btn"><?php
		}
		?>
		<input type="submit" value="<?=ucwords("$word Item");?> &gt;" class="btn">
		<input type="reset" value="Reset Form &gt;" class="btn">
		<p />

		<?php
		$yr = new years(); // pull in the current years
		$pla = new platforms($this->pg,$this->info['platformID']);

		$this->pg->outlineTableHead();
		?>

		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>UPC Number:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[upc]" size="25" value="<?=@$this->info['upc'];?>" onkeypress="return checkenter(this,event)"></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Title:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[title]" size="50" value="<?=htmlspecialchars(@$this->info['title']);?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Description:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><textarea name="info[description]" rows="5" cols="45" scrolling="auto"><?=@$this->info['description'];?></textarea></td>
		</tr>
		<?php
		$image_types = array(
			'box'   => 'Box',
			'nobox' => 'No Box'
		);
		while (list($type, $label) = each($image_types))
		{
			?>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b><?php echo $label;?> Image:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>">
					<?php
					if (!strlen(@$this->info[$type . '_imgID']))
					{
						$this->info[$type . '_imgID'] = 0;
					}
					$path = $this->image_path($this->info[$type . '_imgID'],YES);

					if (!file_exists($path))
					{
						$path = $_SESSION['root'] . 'images/items/none.gif';
					}

					$size = @getimagesize($path);
					$wh = @$size[3];

					?>
					<input type="hidden" name="info[<?php echo $type;?>_imgID]" value="<?=@$this->info[$type . '_imgID'];?>">
					<table border="0" cellspacing="0" cellpadding="3">
						<tr>
							<td><img src="<?=$path;?>" border="1" bordercolor="#000000" name="itemimg" <?=$wh;?>></td>
							<td>
								<a href="javascript:image('select', '<?php echo $type;?>')">Select Image &gt;</a><br />
								<a href="javascript:image('upload', '<?php echo $type;?>')">Upload New Image &gt;</a>
							</td>
						</tr>
					</table>
				</td>
			</tr>
			<?php
		} // each image type
		?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Age:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[age]" size="5" value="<?=@$this->info['age'];?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Year:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><select name="info[yearID]" size="1"><option value=""></option><?php
				while (list($a,$arr) = each($yr->years))
				{
					if ($arr['yer_yearID'] == @$this->info['yearID']) { $s = ' selected="selected"'; } else { $s = ''; }
					?><option value="<?=$arr['yer_yearID'];?>"<?=$s;?>><?=$arr['yer_year'];?></option><?php
				}
			?></select></td>
		</tr>
		<tr>
			<a name="platforms"></a>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Platform:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<?php
				$pla->set_item('platforms');
				while (list($a,$arr) = each($pla->values))
				{
					if ($arr[0] == @$this->info['platformID']) { echo $arr[1]; break; }
				}
				?>
				<input type="hidden" name="info[platformID]" value="<?=$this->info['platformID'];?>">
			</td>
		</tr>
		<tr>
			<a name="companies"></a>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Company 1:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<select name="info[company1ID]" size="1" style="vertical-align:middle"><option value=""></option><?php
					$pla->set_item('companies');
					while (list($a,$arr) = each($pla->values))
					{
						if ($arr[0] == @$this->info['company1ID']) { $s = ' selected="selected"'; } else { $s = ''; }
						?><option value="<?=$arr[0];?>"<?=$s;?>><?=$arr[1];?></option><?php
					}
					if (!count($pla->values)) { ?><option value="">No Companies</option><?php }
				?></select>
			</td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Company 2:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<table border="0" cellspacing="0" cellpadding="0" width="100%">
					<tr>
						<td>
							<select name="info[company2ID]" size="1" style="vertical-align:middle"><option value=""></option><?php
								reset($pla->values);
								while (list($a,$arr) = each($pla->values))
								{
									if ($arr[0] == @$this->info['company2ID']) { $s = ' selected="selected"'; } else { $s = ''; }
									?><option value="<?=$arr[0];?>"<?=$s;?>><?=$arr[1];?></option><?php
								}
								if (!count($pla->values)) { ?><option value="">No Companies</option><?php }
							?></select>
						</td>
						<td align="right" valign="middle">
							<a href="javascript:add('companies')">Add New Company &gt;</a>
						</td>
					</tr>
				</table>
			</td>
		</tr>
		<tr>
			<a name="types"></a>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Type:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<table border="0" cellspacing="0" cellpadding="0" width="100%">
					<tr>
						<td>
							<select name="info[typeID]" size="1" style="vertical-align:middle"><option value=""></option><?php
								$pla->set_item('types');
								while (list($a,$arr) = each($pla->values))
								{
									if ($arr[0] == @$this->info['typeID']) { $s = ' selected="selected"'; } else { $s = ''; }
									?><option value="<?=$arr[0];?>"<?=$s;?>><?=$arr[1];?></option><?php
								}
								if (!count($pla->values)) { ?><option value="">No Types</option><?php }
							?></select>
						</td>
						<td align="right" valign="middle">
							<a href="javascript:add('types')">Add New Type &gt;</a>
						</td>
					</tr>
				</table>
			</td>
		</tr>
		<!-- dynamic fields -->
		<?php
		$pla->set_item('fields');
		while (list($a,$arr) = each($pla->values))
		{
			$fieldID = $arr[0];
			$desc = $arr[1];
			$size = $arr[2];
			$val = @$this->info['item_field_values'][$fieldID];
			?>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b><?=$desc;?>:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>">
					<table border="0" cellspacing="0" cellpadding="0" width="100%">
						<tr>
							<td><input type="text" name="info[item_field_values][<?=$fieldID;?>]" size="<?=$size;?>" value="<?=htmlspecialchars($val);?>"></td>
							<td align="right" valign="middle"><?php
								if (($a+1) == count($pla->values)) { ?><a href="javascript:add('fields')">Add New Field &gt;</a><?php }
								else { ?>&nbsp;<?php }
							?></td>
						</tr>
					</table>
				</td>
			</tr>
			<?php
		}

		if (!count($pla->values))
		{
			?>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>">&nbsp;</td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>" align="right">
					<a href="javascript:add('fields')">Add New Form Field &gt;</a>
				</td>
			</tr>
			<?php
		}
		?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Pricing:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<table border="0" cellspacing="0" cellpadding="1">
					<tr>
						<td valign="middle"><b>New:</b></td>
						<td>$<input type="text" size="6" name="info[price][<?=ITEM_NEW;?>]" value="<?=@$this->info['price'][ITEM_NEW];?>" onblur="this.value=format_price(this.value)" style="text-align:right;vertical-align:middle"></td>
					</tr>
					<tr>
						<td valign="middle"><b>Used:</b></td>
						<td>$<input type="text" size="6" name="info[price][<?=ITEM_USED;?>]" value="<?=@$this->info['price'][ITEM_USED];?>" onblur="this.value=format_price(this.value)" style="text-align:right;vertical-align:middle"></td>
					</tr>
				</table>
			</td>
		</tr>
		<tr>
			<a name="features"></a>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Features:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<table border="0" cellspacing="0" cellpadding="0" width="100%">
					<tr>
						<td align="left" valign="top" width="50%">

								<?php
								$pla->set_item('features');
								$half = ceil((count($pla->values)/2));
								$i = -1;
								while (list($a,$arr) = each($pla->values))
								{
									$i++;
									if ($i == $half) { ?></td><td align="left" valign="top" width="50%"><?php }
									if (in_array($arr[0],@$this->info['features'])) { $s = ' checked="checked"'; } else { $s = ''; }
									?><input type="checkbox" name="info[features][]" value="<?=$arr[0];?>" id="f<?=$arr[0];?>" style="vertical-align:middle;border-width:0"<?=$s;?>> <label style="cursor:hand" for="f<?=$arr[0];?>"><?=$arr[1];?></label><br /><?php
								}
								?>

							<p />
							<div align="right"><a href="javascript:add('features')">Add New Feature &gt;</a></div>
						</td>
					</tr>
				</table>
			</td>
		</tr>
		<tr>
			<a name="sources"></a>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Prices/Ratings:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<table border="0" cellspacing="1" cellpadding="3" width="100%">
					<tr>
						<td align="center" bgcolor="<?=$this->pg->color('table-label');?>"><b>Price</b></td>
						<td align="center" bgcolor="<?=$this->pg->color('table-label');?>"><b>Rating</b></td>
					</tr>
					<tr>
						<?php
						$pla->set_item('sources');

						$price = array();
						$rating = array();

						while (list($a,$arr) = each($pla->values))
						{
							if ($arr[2] == PRICE) { $price[] = $arr; } else { $rating[] = $arr; }
						}
						?>
						<td align="left" valign="top" width="50%">
							<?php
							while (list($a,$arr) = each($price))
							{
								?>$<input type="text" name="info[item_source_values][<?=$arr[0];?>]" id="p<?=$arr[0];?>" size="6" value="<?=@$this->info['item_source_values'][$arr[0]];?>" style="text-align:right;vertical-align:middle"> <label for="p<?=$arr[0];?>"><?=$arr[1];?></label><br /><?php
							}
							?>
						</td>
						<td align="left" valign="top" width="50%">
							<?php
							while (list($a,$arr) = each($rating))
							{
								?><input type="text" name="info[item_source_values][<?=$arr[0];?>]" id="r<?=$arr[0];?>" size="5" value="<?=@$this->info['item_source_values'][$arr[0]];?>" style="text-align:right;vertical-align:middle"> <label for="r<?=$arr[0];?>"><?=$arr[1];?></label><br /><?php
							}
							?>
						</td>
					</tr>
				</table>
				<p />
				<div align="right"><a href="javascript:add('sources')">Add New Price/Rating Source &gt;</a></div>
			</td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Quantity:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<table border="0" cellspacing="0" cellpadding="0" height="25">
					<tr>
						<td>
							New: <input type="text" name="info[qty_new]" size="4" value="<?=@$this->info['qty_new'];?>"<?php if ($word == 'edit') { ?> onfocus="qtywarn.style.display='block'" onblur="qtywarn.style.display='none'"<?php } ?> style="text-align:right;vertical-align:middle">
							Used: <input type="text" name="info[qty_used]" size="4" value="<?=@$this->info['qty_used'];?>"<?php if ($word == 'edit') { ?> onfocus="qtywarn.style.display='block'" onblur="qtywarn.style.display='none'"<?php } ?> style="text-align:right;vertical-align:middle">
						</td>
						<td>&nbsp;</td>
						<td class="nine" valign="middle"><div id="qtywarn" style="display:none"><b>Please be careful when changing these values...<br />Make sure you enter the correct quantities!</b></div></td>
					</tr>
				</table>
			</td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Notes:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<textarea name="info[notes]" rows="5" cols="40"><?=@$this->info['notes'];?></textarea>
			</td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Active:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<input type="radio" name="info[active]" value="<?=YES;?>" id="activey" style="vertical-align:middle;border-width:0px"<?=$ayc;?>> <label style="cursor:hand" for="activey">YES</label>
				<input type="radio" name="info[active]" value="<?=NO;?>" id="activen" style="vertical-align:middle;border-width:0px"<?=$anc;?>> <label style="cursor:hand" for="activen">No</label>
			</td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>">&nbsp;</td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<input type="checkbox" name="info[percopy]" value="<?=YES;?>" id="percopy" style="vertical-align:middle;border-width:0px"<?=($this->info['percopy']==YES?' checked="checked"':'');?>> <label for="percopy">This item is affected by per-copy discounts</label>
			</td>
		</tr>
		<?php
		$this->pg->outlineTableFoot();
		?>

		<p />
		<?php
		if ($frominvoice)
		{
			?><input type="button" value="&lt; Return to Invoice" onclick="document.location='/admin/pos/invoice.php?act=view'" class="btn"><?php
		}
		if ($fromupc)
		{
			?><input type="button" value="&lt; Return to UPC Setup" onclick="document.location='/admin/setup_items/enterupc.php?act=showitems&platformID=<?=$this->info['platformID'];?>'" class="btn"><?php
		}
		if ($fromqty)
		{
			?><input type="button" value="&lt; Return to Whole Platform Quantity Setup" onclick="document.location='/admin/setup_items/begqty.php?act=showitems&platformID=<?=$this->info['platformID'];?>'" class="btn"><?php
		}
		if ($fromreceiveorder)
		{
			?><input type="button" value="&lt; Return to Receive Order" onclick="document.location='/admin/utilities/receive_order.php'" class="btn"><?php
		}
		if ($fromreturns)
		{
			?><input type="button" value="&lt; Return to Returns" onclick="document.location='/admin/pos/returns.php'" class="btn"><?php
		}
		?>
		<input type="submit" value="<?=ucwords("$word Item");?> &gt;" class="btn">
		<input type="reset" value="Reset Form &gt;" class="btn">
		</form>
		<?php

		if (!strlen($moveto))
		{
			if (strlen($this->info['upc'])) { $field = 'title'; } else { $field = 'upc'; }
			$this->pg->addOnload("document.itemfrm.elements['info[$field]'].focus()");
		}
		else
		{
			$this->pg->addOnload("document.location='#$moveto'");
		}

		$this->pg->addOnload('set_unknown()');
	}

	/**
	* Parses $_POST
	*/
	function pull_post()
	{
		$this->itemID = @$_POST['itemID'];
		$this->info = @$_POST['info'];
		$this->info['title'] = stripslashes(@$this->info['title']);
		$this->info['description'] = stripslashes(@$this->info['description']);
		$this->selID = @$_POST['selID'];

		if (!isset($this->info['percopy'])) { $this->info['percopy'] = NO; }
	}

	/**
	* Adds/edits the information stored in $this->info
	* @return integer new itemID
	*/
	function add()
	{
		global $db;

		// first, if needed, check for a duplicate UPC
		$isdupe = NO;
		if (strlen(@$this->info['upc']))
		{
			$sql = "SELECT itm_itemID FROM items WHERE itm_upc='".$this->info['upc']."'";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			if (mysql_num_rows($result))
			{
				$row = mysql_fetch_assoc($result);
				$cditemID = $row['itm_itemID'];

				if (!$this->itemID || ($this->itemID && $cditemID != $this->itemID)) { $isdupe = YES; }
			}
		}

		if ($isdupe == NO)
		{
			// insert into 'items'
			$sql = ($this->itemID?"UPDATE items":"INSERT INTO items")." SET ";

			$its = new item_search($this->pg);
			$this->info['title_search'] = $its->format_title_search($this->info['title']);

			$toitems = array(
				'upc',
				'title',
				'title_search',
				'description',
				'age',
				'box_imgID',
				'nobox_imgID',
				'yearID',
				'platformID',
				'company1ID',
				'company2ID',
				'typeID',
				'percopy',
				'notes',
				'active'
			);
			$vals = array();
			while (list($a,$f) = each($toitems))
			{
				//added by Stephanie R 7/30/16
				
				$testval = mysql_real_escape_string(stripslashes(stripslashes(@$this->info[$f])));
				$newval= !empty($testval)? "'$testval'" : "NULL";
				$vals[] = "itm_$f= $newval";
			}
			$sql .= implode(",",$vals);

			if ($this->itemID) { $sql .= " WHERE itm_itemID=$this->itemID"; }

			print_r($a);
			print_r($toitems);
			print_r($vals);
			echo $sql;
			echo $db;
			mysql_query($sql,$db);

			$isdupe = $this->error->mysql(__FILE__,__LINE__,YES);

			$itemID = ($this->itemID ? $this->itemID : mysql_insert_id());

			// insert into 'itm_fet_rel'
			if ($this->itemID)
			{
				// delete the old entries
				$sql = "DELETE FROM itm_fet_rel WHERE ifr_itemID=$this->itemID";
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}
			if (count(@$this->info['features']))
			{
				$sql = "INSERT INTO itm_fet_rel VALUES ";
				$vals = array();
				while (list($a,$featureID) = each($this->info['features'])) { $vals[] = "($itemID,$featureID)"; }
				$sql .= implode(",",$vals);

				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}

			// insert into 'prices'
			while (list($type,$price) = each($this->info['price']))
			{
				if (!is_numeric($price) || !strlen($price)) { $price = 0; }
				$this->info['price'][$type] = $price;
			}

			$prc = new prices($pg);
			$prc->setPrice(
				$itemID,
				$this->info['price'][ITEM_NEW],
				$this->info['price'][ITEM_USED]
			);

			// insert into 'item_source_values'
			if ($this->itemID)
			{
				// delete the old entries
				$sql = "DELETE FROM item_source_values WHERE isv_itemID=$this->itemID";
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}
			if (count(@$this->info['item_source_values']))
			{
				$sql = "INSERT INTO item_source_values VALUES ";
				$vals = array();
				while (list($sourceID,$val) = each($this->info['item_source_values']))
				{
					if (strlen(trim($val))) { $vals[] = "($itemID,$sourceID,$val)"; }
				}
				$sql .= implode(",",$vals);

				if (count($vals))
				{
					mysql_query($sql,$db);
					$this->error->mysql(__FILE__,__LINE__);
				}
			}

			// insert into 'item_field_values'
			if ($this->itemID)
			{
				// delete the old entries
				$sql = "DELETE FROM item_field_values WHERE ifv_itemID=$this->itemID";
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}
			if (count(@$this->info['item_field_values']))
			{
				$sql = "INSERT INTO item_field_values VALUES ";
				$vals = array();
				while (list($fieldID,$val) = each($this->info['item_field_values']))
				{
					if (strlen(trim($val))) { $vals[] = "($itemID,$fieldID,'".mysql_real_escape_string($val)."')"; }
				}
				$sql .= implode(",",$vals);

				if (count($vals))
				{
					mysql_query($sql,$db);
					$this->error->mysql(__FILE__,__LINE__);
				}
			}

			// insert into 'quantity'
			if (!is_numeric($this->info['qty_new'])) { $this->info['qty_new'] = 0; }
			if (!is_numeric($this->info['qty_used'])) { $this->info['qty_used'] = 0; }

			$sql = "DELETE FROM quantity WHERE qty_storeID=".$_SESSION['storeID']." AND qty_itemID=$itemID";
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			// get all storeIDs and insert a row into the quantity table
			$storeIDs = array();
			$sql = "SELECT sto_storeID FROM stores";
			$result = mysql_query($sql,$db);
			while ($row = mysql_fetch_assoc($result)) { $storeIDs[] = $row['sto_storeID']; }

			while (list($a,$storeID) = each($storeIDs))
			{
				if ($_SESSION['storeID'] == $storeID) { $qnew = $this->info['qty_new']; $qused = $this->info['qty_used']; }
				else { $qnew = 0; $qused = 0; }

				$sql = "INSERT IGNORE INTO quantity VALUES ($storeID,$itemID,$qnew,$qused)";
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}

			$this->status[0] = ($this->itemID ? EDITED : ADDED);

			if ($this->itemID) { return $this->itemID; }
			else { return $itemID; }
		}
		else { $this->status[0] = DUPLICATE; }

		return @$itemID;
	}

	/**
	* Check the status of the addition/edit
	* @return integer YES/NO
	*/
	function was_added()
	{
		if ($this->status[0] == ADDED || $this->status[0] == EDITED) { return YES; }
		else { return NO; }
	}

	/**
	* Deletes $this->itemID (sets as inactive)
	*/
	function delete()
	{
		global $db;

		$sql = "UPDATE items SET itm_active=".NO." WHERE itm_itemID=$this->itemID";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		$this->status[0] = DELETED;
	}

	/**
	* Undeletes $this->itemID (sets as active)
	*/
	function undelete()
	{
		global $db;

		$sql = "UPDATE items SET itm_active=" . YES . " WHERE itm_itemID=$this->itemID";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		$this->status[0] = DELETED;
	}

	/**
	* Check the status of the deletion
	* @return integer YES/NO
	*/
	function was_deleted()
	{
		if ($this->status[0] == DELETED) { return YES; }
		else { return NO; }
	}

	/**
	* Check the status of the undeletion (alias for was_deleted())
	* @return integer YES/NO
	*/
	function was_undeleted()
	{
		return $this->was_deleted();
	}

	/**
	* Generate and show any error message(s) according to the contents of $this->status
	*/
	function show_errors()
	{
		$errors = array();

		for ($i=0; $i<count($this->status); $i++)
		{
			if ($this->status[$i] == DUPLICATE) { $error = "An item with that UPC already exists! (".$this->info['upc'].")"; }

			$errors[] = $error;
		}

		$this->pg->error($errors);
	}

	/**
	* Generate and show any status message(s) according to the contents of $this->status
	*/
	function show_status()
	{
		$status = array();

		$inf = $this->get_info();
		$single = $inf['SINGLE'];

		for ($i=0; $i<count($this->status); $i++)
		{
			if ($this->status[$i] == ADDED) { $stat = "Added $single"; }
			elseif ($this->status[$i] == EDITED) { $stat = "Edited $single"; }
			elseif ($this->status[$i] == DELETED) { $stat = "Deleted $single"; }

			$status[] = $stat;
		}

		$this->pg->status($status);
	}

	/**
	* Obtains the pathname for the given imgID
	* @param integer $imgID
	* @param integer $thumb Thumbnail path? [optional, default NO]
	* @return string relative path to image
	*/
	function image_path($imgID,$thumb=NO)
	{
		global $db;

		$sql = "SELECT iti_platformID,iti_filename FROM item_images WHERE iti_imgID=$imgID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		if (mysql_num_rows($result))
		{
			$row = mysql_fetch_assoc($result);
			$file = $row['iti_filename'];
			if ($thumb) { $file = substr($file,0,strrpos($file,"."))."_thumb.".substr($file,-1*(strlen($file)-strrpos($file,".")-1)); }
			$return = $_SESSION['root']."images/items/platform".$row['iti_platformID']."/".$file;
		}
		else { $return = $_SESSION['root']."images/items/none.gif"; }

		return $return;
	}

	/**
	* Perform an item image search
	* @param string $criteria
	*/
	function image_search($platformID,$criteria)
	{
		global $db;

		$sql = "SELECT * FROM item_images WHERE iti_platformID=$platformID AND iti_filename LIKE '$criteria' ORDER BY iti_filename";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		$results = array();
		while ($row = mysql_fetch_assoc($result)) { $results[] = $row; }

		$this->image_results = $results;
	}

	/**
	* Output the item image search results
	*/
	function image_search_results($platformID,$fromselect)
	{
		?>
		<script type="text/javascript">
			var selID = 0;
			function sel(id)
			{
				selID = id;

				var obj = eval('document.itmimage.sel'+id);
				obj.checked = true;

				document.itmimage.s.disabled = false;
				document.itmimage.d.disabled = false;
			}

			function go(where)
			{
				if (!selID) { alert('You must select an image'); }
				else
				{
					if (where != 'delete' || (where == 'delete' && confirm('Are you SURE you want to delete the selected image?') == true))
					{
						var frm = document.itmimage;
						frm.act.value = where;
						frm.submit();
					}
				}
			}
		</script>

		<form method="post" action="image.php" name="itmimage">
		<input type="hidden" name="act" value="">
		<input type="hidden" name="platformID" value="<?=$platformID;?>">
		<input type="hidden" name="fromselect" value="<?=$fromselect;?>">
		<?=$this->pg->outlineTableHead();?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-head');?>">&nbsp;</td>
			<td bgcolor="<?=$this->pg->color('table-head');?>"><b>Image</b></td>
			<td bgcolor="<?=$this->pg->color('table-head');?>"><b>Filename</b></td>
		</tr>

		<?php
		while (list($a,$arr) = each($this->image_results))
		{
			$bg = (($a%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

			$path = $this->image_path($arr['iti_imgID'],YES);
			if (!file_exists($path)) { $path = $_SESSION['root'].'images/items/none.gif'; }

			$size = getimagesize($path);
			$wh = $size[3];

			?>
			<label for="sel<?=$arr['iti_imgID'];?>">
			<tr>
				<td bgcolor="<?=$bg;?>"><input type="radio" name="imgID" value="<?=$arr['iti_imgID'];?>" id="sel<?=$arr['iti_imgID'];?>" class="nb" onclick="sel(<?=$arr['iti_imgID'];?>)"></td>
				<td bgcolor="<?=$bg;?>"><img src="<?=$path;?>" onclick="sel(<?=$arr['iti_imgID'];?>)" <?=$wh;?>></td>
				<td bgcolor="<?=$bg;?>"><?=$arr['iti_filename'];?></td>
			</tr>
			</label>
			<?php
		}
		?>

		<?=$this->pg->outlineTableFoot();?>

		<p />

		<input type="button" value="Select &gt;" id="s" disabled="disabled" onclick="go('doselect')" class="btn">
		<input type="button" value="Delete &gt;" id="d" disabled="disabled" onclick="go('delete')" class="btn">
		</form>
		<?php
	}

	/**
	* Update the new prices according to the item source values - returns only the changed prices
	* @param array $itemIDs
	* @param integer $set_prices Set the prices? If NO, just returns array  [optional, default YES]
	* @return array $new_prices
	*/
	function update_new_prices($itemIDs,$set_prices=YES)
	{
		global $db;

		if (count($itemIDs))
		{
			$min_prices = array();
			$changed_prices = array();

			while (list($a,$itemID) = each($itemIDs)) { $min_prices[$itemID] = 0; }
			reset($itemIDs);

			$sql = "SELECT isv_itemID,MIN(isv_value) AS min_value FROM item_source_values,sources WHERE isv_itemID IN (".implode(',',$itemIDs).") AND isv_sourceID=sor_sourceID AND sor_type=".PRICE." GROUP BY isv_itemID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result)) { $min_prices[$row['isv_itemID']] = $row['min_value']; }

			$prc = new prices($this->pg);

			if ($set_prices)
			{
				while (list($itemID,$price) = each($min_prices))
				{
					$price = $prc->fix_price($price*1.25);
					/* As per Scott's request, this has been disabled...
					$changed = $prc->setPrice($itemID,$price,-1,YES);
					if ($changed) { $changed_prices[$itemID] = $price; }
					*/
				}
			}
			else
			{
				$sql = "SELECT prc_itemID,prc_new FROM prices WHERE prc_itemID IN (".implode(',',$itemIDs).")";
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
				while ($row = mysql_fetch_assoc($result))
				{
					if ($row['prc_new'] != $min_prices[$row['prc_itemID']]) { $changed_prices[$row['prc_itemID']] = $prc->fix_price($min_prices[$row['prc_itemID']]*1.25); }
				}
			}

			return $changed_prices;
		}

		return array();
	}

	/**
	* Given a price, if it's a round dollar, a penny is subtracted (IE: $4.00 = $3.99, $14.50 = $14.50)
	* @param integer $price
	* @return integer fixed price
	*/
	function fix_price($price)
	{
		if ($price > 0 && sprintf('%0.0f',$price) == $price) { $price = ($price-0.01); }

		return $price;
	}

	/**
	* Return the "Edit Items by Criteria" operations
	* @return	array
	* @access	public
	*/
	function getCriteriaOperations()
	{
		return array(
			'string' => array(
				'='  => 'Equal To',
				'!=' => 'Not Equal To'
			), // end string
			'number' => array(
				'='  => 'Equal To',
				'!=' => 'Not Equal To',
				'<'  => 'Less Than',
				'<=' => 'Less Than or Equal To',
				'>'  => 'Greater Than',
				'>=' => 'Greater Than or Equal To',
			) // end number
		);
	} // end function getCriteriaOperations
} // end class items

?>