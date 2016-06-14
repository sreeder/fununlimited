<?php
/**
* Platform setup (item features, sources, etc)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class platforms
{
	/**
	* page class reference
	* @var reference $pg
	*/
	var $pg;

	/**
	* The current setup item
	* @var string $item
	*/
	var $item;

	/**
	* Setup items and their corresponding names and fields
	* @var array $items
	*/
	var $items;

	/**
	* Values for the current item
	* @var array $values
	*/
	var $values;

	/**
	* Column titles
	* @var array $field_titles
	*/
	var $field_titles;

	/**
	* The current platformID
	* @var integer $platformID
	*/
	var $platformID;

	/**
	* The name of the current platform
	* @var string $platform_name
	*/
	var $platform_name;

	/**
	* The current selID
	* @var integer $selID
	*/
	var $selID;

	/**
	* Information for the current selID (set with $this->set_selID() and passed in POST)
	* @var array $info
	*/
	var $info;

	/**
	* Is the setup screen in a popup window?
	* @var integer $popup
	*/
	var $popup;

	/**
	* If the setup screen is in a popup, what is the parent form to refresh?
	* @var string $parentform
	*/
	var $parentform;

  /**
  * Status variable used for error reporting (bad email, bad zip code, success, etc)
  * @var array $status
  */
  var $status;

	/**
	* Error reporting class
	* @var identifier $error
	*/
	var $error;

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	* @param integer $platformID
	* @param string $item Which setup item? [optional, default '']
	*/
	function platforms(&$pg,$platformID,$item="")
	{
		$this->pg =& $pg;
		$this->info = array();
		$this->platformID = $platformID;
		$this->selID = 0;
		$this->popup = NO;
		$this->items = array(
			'features'=>array(
				'TITLE'=>'Item Features',
				'SINGLE'=>'feature',
				'EXAMPLE'=>"'Backup', 'Analog', 'Mouse'",
				'NOTE'=>'When adding an item, features will be a YES/No selection.',
				'FIELDS'=>array(
					array('description','Feature Description','text',20)
				),
				'TABLE'=>array('features','fet','featureID')
			),
			'sources'=>array(
				'TITLE'=>'Price/Rating Sources',
				'SINGLE'=>'source',
				'EXAMPLE'=>"'PSM/Rating', 'EGM/Rating', 'Masco/Price', 'Big Apple/Price'",
				'NOTE'=>'If you do not select a type, it will default to <b>Price</b>.',
				'FIELDS'=>array(
					array('name','Source Name','text',20),
					array('type','Source Type','radio',1,array(PRICE=>'Price',RATING=>'Rating'))
				),
				'TABLE'=>array('sources','sor','sourceID')
			),
			'types'=>array(
				'TITLE'=>'Item Types',
				'SINGLE'=>'type',
				'EXAMPLE'=>"'Action', 'Sports-Soccer', 'Hunting', 'Racing', 'Comedy'",
				'NOTE'=>'',
				'FIELDS'=>array(
					array('type','Type','text',20)
				),
				'TABLE'=>array('types','typ','typeID')
			),
			'companies'=>array(
				'TITLE'=>'Companies',
				'SINGLE'=>'company',
				'EXAMPLE'=>"'Acclaim', 'Sony', 'Konami'",
				'NOTE'=>'',
				'FIELDS'=>array(
					array('name','Company Name','text',20)
				),
				'TABLE'=>array('companies','com','companyID')
			),
			'fields'=>array(
				'TITLE'=>'Form Fields',
				'SINGLE'=>'field',
				'EXAMPLE'=>"'Director', '# of Players', 'Hard Disk Space Required'",
				'NOTE'=>'',
				'FIELDS'=>array(
					array('name','Field Name','text',20),
					array('size','Field Size','text',3,'return onlynumbers(this.value,event,false)')
				),
				'TABLE'=>array('item_fields','itf','fieldID')
			),
			'platforms'=>array(
				'TITLE'=>'Platforms',
				'SINGLE'=>'platform',
				'EXAMPLE'=>"'Nintendo 64/N64', 'Playstation/PSX'",
				'NOTE'=>"This is used for values such as 'Arcade', 'VHS', 'DVD', and so forth.<br />The abbreviation is used on the point-of-sale invoice screen.<br />Only enter the filename for the image. It must exist in /images/platforms/",
				'FIELDS'=>array(
					array('name','Platform Name','text',20),
					array('abbr','Abbreviation','text',8),
					array('defaultbox','Default Box (Trade)','radio',1,array(BOX=>'Box',NOBOX=>'No Box')),
					array('image','Image','text',20)
				),
				'TABLE'=>array('platforms','pla','platformID')
			)
		);

		$this->error = new error('Setup');

		$this->set_item($item);
	}

	/**
	* Sets the setup item and pulls in the current values
	*/
	function set_item($item)
	{
		global $db;

		$this->item = $item;
		$this->values = array();

		if (strlen($this->item))
		{
			$inf     = $this->get_info();
			$title   = $inf['TITLE'];
			$single  = $inf['SINGLE'];
			$fields  = $inf['FIELDS'];
			$table   = $inf['TABLE'][0];
			$prefix  = $inf['TABLE'][1];
			$idfield = $inf['TABLE'][2];

			$pull_fields = array($prefix.'_'.$idfield);
			$this->field_titles = array(''=>'&nbsp;');
			while (list($a,list($f,$t)) = each($fields))
			{
				$this_field = $prefix.'_'.$f;
				$pull_fields[] = $this_field;
				$this->field_titles[$this_field] = $t;
			}

			// pull in the values
			if ($this->item == 'sources') { $sortby = "sor_type,$pull_fields[1]"; }
			else { $sortby = $pull_fields[1]; }

			if ($this->item != 'platforms') { $where = "WHERE ${prefix}_platformID=$this->platformID "; } else { $where = ""; }

			$this->values = array();
			$sql = "SELECT ".implode(",",$pull_fields)." FROM $table ${where}ORDER BY $sortby";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_row($result))
			{
				for ($i=0; $i<count($row); $i++) { $row[$i] = stripslashes($row[$i]); }
				$this->values[] = $row;
			}

			// pulls in all of the values
			$this->good_values = array();
			$sql = "SELECT * FROM $table ORDER BY $sortby";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				$this->good_values[$row[$prefix . '_' . $idfield]] = $row;
			}
		}
	}

	/**
	* Return the "good" values
	* @return	array
	* @access	public
	*/
	function getGoodValues()
	{
		return $this->good_values;
	}

	/**
	* Displays the current values for the setup item with add/edit/remove buttons
	* @param integer $select Show a 'select' button? [optional, default NO]
	* @param integer $highlightID The ID of the row to highlight [optional, default "0"]
	* @param integer $isreassign Is this for the reassignment form? [optional, default NO]
	*/
	function show_values($select=NO,$highlightID=0,$isreassign=NO)
	{
		global $db;

		if (!strlen($this->item)) { echo "No value for \$this->item!"; }
		else
		{
			$inf = $this->get_info();
			$title = $inf['TITLE'];
			$single = $inf['SINGLE'];
			$example = $inf['EXAMPLE'];
			$note = $inf['NOTE'];
			$fields = $inf['FIELDS'];
			$table = $inf['TABLE'][0];
			$prefix = $inf['TABLE'][1];
			$idfield = $inf['TABLE'][2];

			if ($select == YES) { $title = "Please Select a ".ucwords($single); }
			$this->pg->pageHead($title,3);
			if (!$select && !$isreassign) { $this->show_platform(YES); }

			if (count($this->status)) { $this->show_status(); }
			if ($isreassign == YES)
			{
				?>Please select the <?=$single;?> to assign any affected <?=($this->item=='platforms'?'values & items':'items');?> to:<p /><?php

				if ($this->item == 'platforms')
				{
					?><input type="button" value="Do Not Reassign - Just Delete &gt;" onclick="noreassign()" class="btn" /><p /><?php
				}
			}

			$letters = array();
			for ($i=65; $i<=90; $i++) { $letters[$i] = NO; }

			// pull in the values
			$show_values = array();
			while (list($a,$arr) = each($this->values))
			{
				if ($isreassign == NO || ($isreassign == YES && $arr[0] != $this->selID)) { $show_values[] = $arr; $letters[ord(strtoupper(substr($arr[1],0,1)))] = YES; }
			}

			$lastlet = '';

			// output the table
			?>
			<script type="text/javascript">
				var selID = 0;
				function setID(id)
				{
					selID = id;
					<?php
					if ($isreassign == NO)
					{
						if ($select == YES) { ?>document.setup.s0.disabled = false;document.setup.s1.disabled = false;<?php }
						?>
						document.setup.e0.disabled = false;document.setup.e1.disabled = false;
						document.setup.d0.disabled = <?=(count($show_values)==1?'true':'false');?>;document.setup.d1.disabled = <?=(count($show_values)==1?'true':'false');?>;
						<?php
					}
					else
					{
						?>
						document.setup.r0.disabled = false;document.setup.r1.disabled = false;
						<?php
					}
					?>
				}

				function go(where)
				{
					if (where == 'select' || where == 'add' || where == 'dodelete' || (selID && where == 'edit') || (selID && where == 'delete' && confirm('Are you SURE you want to delete the selected <?=$single;?>?')))
					{
						var frm = document.setup;
						frm.act.value = where;
						frm.submit();
					}
				}

				function noreassign()
				{
					if (confirm('Please note, EVERY item in this platform WILL BE DELETED!\n\nAre you ABSOLUTELY sure you want to do this?'))
					{
						var frm = document.setup;
						frm.act.value = 'dodelete';
						frm.noreassign.value = '<?=YES;?>';
						frm.submit();
					}
				}
			</script>

			<?php
			while (list($ord,$val) = each($letters))
			{
				echo " ";

				if ($val == YES) { ?><a href="#<?=chr($ord);?>" style="text-decoration:underline"><?php }
				else { ?><font color="#BBBBBB"><?php }

				echo chr($ord);

				if ($val == YES) { ?></a><?php }
				else { ?></font><?php }

				echo " ";
			}

			$buttons = "";
			if ($isreassign == NO)
			{
				if ($select == YES)
				{
					$buttons .= '<input type="button" value="Select &gt" class="btn" onclick="go(\'select\')" id="s%n" disabled="true"> ';
				}
				$buttons .= '<input type="button" value="Add &gt" class="btn" onclick="go(\'add\')"> ';
				$buttons .= '<input type="button" value="Edit &gt" class="btn" onclick="go(\'edit\')" id="e%n" disabled="true"> ';
				$buttons .= '<input type="button" value="Delete &gt" class="btn" onclick="go(\'delete\')" id="d%n" disabled="true"> ';
			}
			else
			{
				$buttons .= '<input type="button" value="Reassign &amp; Delete &gt" class="btn" onclick="go(\'dodelete\')" id="r%n" disabled="true">';
			}
			?>

			<p />

			<form method="post" action="platforms.php" name="setup">
			<input type="hidden" name="act" value="">
			<input type="hidden" name="platformID" value="<?=$this->platformID;?>">
			<input type="hidden" name="deleteID" value="<?=$this->selID;?>">
			<input type="hidden" name="item" value="<?=$this->item;?>">
			<input type="hidden" name="noreassign" value="<?=NO;?>">
			<?php
			echo ereg_replace("%n","0",$buttons)."<p />";

			$this->pg->outlineTableHead();

			?>
			<tr>
				<?php
				while (list($f,$t) = each($this->field_titles)) { ?><td bgcolor="<?=$this->pg->color('table-head');?>"><b><?=$t;?></b></td><?php }
				?>
			</tr>
			<?php

			if (!count($show_values)) { ?><tr><td colspan="<?=count($this->field_titles);?>" bgcolor="<?=$this->pg->color('table-cell');?>" align="center">--- No <?=$title;?> ---</td></tr><?php }

			while (list($i,$arr) = each($show_values))
			{
				$bg = (($i%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

				$id = $arr[0];

				if ($id == $highlightID) { $pb = '<b>'; $sb = '</b>'; } else { $pb = ''; $sb = ''; }

				?>
				<label for="<?=$id;?>">
				<tr><?php
					$let = strtoupper(substr($arr[1],0,1));
					if ($let != $lastlet)
					{
						?><a name="#<?=$let;?>"></a><?php
						$lastlet = $let;
					}

					for ($j=0; $j<count($arr); $j++)
					{
						$val = $arr[$j];
						if (!$j)
						{
							// radio column
							$display = '<input type="radio" name="selID" value="'.$id.'" id="'.$id.'" style="border-width:0" onclick="setID('.$id.')">';
						}
						else
						{
							// value column
							if ($this->item == 'sources' && $j == 2) { $val = ($val==RATING?'Rating':'Price'); }
							elseif ($this->item == 'platforms' && $j == 3) { $val = ($val==BOX?'Box':'No Box'); }
							$display = $val;

							if (NO&&$j == 1) // YES to prepend with ID
							{
								$display = "$id - ".$display;
							}
						}

						?><td bgcolor="<?=$bg;?>"><?=$pb.$display.$sb;?></td><?php
					}
				?></tr>
				</label>
				<?php
			}

			$this->pg->outlineTableFoot();

			echo "<p />".ereg_replace("%n","1",$buttons);
			?>
			</form>
			<?php

			if ($isreassign == NO && count($show_values) == 1) { ?><p /><font size="1"><b>Note:</b> There is only 1 <?=$single;?> - deleting has been disabled.</font><p /><?php }

			if ($this->item != 'platform' && (strlen($example) || strlen($note)))
			{
				$show = array();
				if (strlen($example)) { $show[] = '<font size="1"><b>Example:</b> '.$example.'</font>'; }
				if (strlen($note)) { $show[] = '<font size="1"><b>Note:</b> '.$note.'</font>'; }
				?>
				<p />
				<?=implode('<br />',$show);?>
				<?php
			}
		}
	}

	/**
	* Returns the information for the current item from $this->items
	*/
	function get_info()
	{
		$return = @$this->items[$this->item];
		if (!is_array($return)) { $return = array(); }

		return $return;
	}

	/**
	* Sets the current selID and retrieves it's information
	* @param integer $selID
	* @param integer $get_info Pull the information? YES/NO [optional, default YES]
	*/
	function set_selID($selID,$get_info=YES)
	{
		global $db;

		$this->selID = $selID;

		if ($get_info == YES)
		{
			$inf = $this->get_info();
			$fields = $inf['FIELDS'];
			$table = $inf['TABLE'][0];
			$prefix = $inf['TABLE'][1];
			$idfield = $inf['TABLE'][2];

			$pull_fields = array($prefix.'_'.$idfield);
			while (list($a,list($f,$t)) = each($fields))
			{
				$pull_fields[] = "${prefix}_$f";
			}

			// pull in the values
			$sql = "SELECT ".implode(",",$pull_fields)." FROM $table WHERE ${prefix}_$idfield=$this->selID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			if (mysql_num_rows($result))
			{
				$row = mysql_fetch_assoc($result);
				while (list($k,$v) = each($row))
				{
					$this->info[substr($k,4)] = $v;
				}
			}
			else { $this->info = array(); }
		}
	}

	/**
	* Displays the add/edit form
	*/
	function add_form()
	{
		if ($this->selID) { $word = "edit"; } else { $word = "add"; }

		$inf = $this->get_info();
		$title = $inf['TITLE'];
		$single = $inf['SINGLE'];
		$example = $inf['EXAMPLE'];
		$note = $inf['NOTE'];
		$fields = $inf['FIELDS'];

		$this->pg->pageHead(ucwords("$word $single"),3);
		$this->show_platform();

		if (count($this->status)) { $this->show_errors(); }

		// output the form/table
		$this->pg->outlineTableHead();
		?>
		<form method="post" action="platforms.php" name="infofrm">
		<input type="hidden" name="act" value="do<?=$word;?>">
		<input type="hidden" name="platformID" value="<?=$this->platformID;?>">
		<input type="hidden" name="item" value="<?=$this->item;?>">
		<input type="hidden" name="selID" value="<?=$this->selID;?>">
		<input type="hidden" name="popup" value="<?=$this->popup;?>">
		<input type="hidden" name="parentform" value="<?=$this->parentform;?>">
		<?php

		while (list($a,list($name,$desc,$type,$size)) = each($fields))
		{
			?>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b><?=$desc;?>:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell2');?>"><?php
					if ($type == "text")
					{
						$onkeypress = @$fields[$a][4];
						if (strlen($onkeypress)) { $ok = ' onkeypress="'.$onkeypress.'"'; } else { $ok = ''; }
						?><input type="text" name="info[<?=$name;?>]" size="<?=$size;?>" value="<?=htmlspecialchars(stripslashes(@$this->info[$name]));?>"<?=$ok;?>><?php
					}
					elseif ($type == "radio")
					{
						$options = @$fields[$a][4];
						if (!is_array($options)) { echo "<b>ERROR: NO OPTIONS FOR RADIO FIELD TYPE!</b>"; }
						else
						{
							while (list($val,$show) = each($options))
							{
								if ($val == @$this->info[$name]) { $s = ' checked="checked"'; } else { $s = ''; }
								?><input type="radio" name="info[<?=$name;?>]" value="<?=$val;?>" style="border-width:0;vertical-align:middle" id="<?=$val;?>"<?=$s;?>> <label style="cursor:hand" for="<?=$val;?>"><?=$show;?></label> <?php
							}
						}
					}
				?></td>
			</tr>
			<?php
		}

		$this->pg->outlineTableFoot();
		?>
		<p />
		<input type="submit" value="<?=ucwords("$word $single");?> &gt;" class="btn"> <input type="reset" value="Reset Form &gt;" class="btn">
		</form>
		<?php
		if (strlen($example) || strlen($note))
		{
			$show = array();
			if (strlen($example)) { $show[] = '<font size="1"><b>Example:</b> '.$example.'</font>'; }
			if (strlen($note)) { $show[] = '<font size="1"><b>Note:</b> '.$note.'</font>'; }
			?>
			<p />
			<?=implode('<br />',$show);?>
			<?php
		}

		$this->pg->addOnload("document.infofrm.elements['info[".$fields[0][0]."]'].focus()");
	}

	/**
	* Parses $_POST
	*/
	function pull_post()
	{
		$this->info = $_POST['info'];
		$this->selID = $_POST['selID'];
	}

	/**
	* Adds/edits the information stored in $this->item, $this->selID, and $this->info
	*/
	function add()
	{
		global $db;

		$inf = $this->get_info();
		$fields = $inf['FIELDS'];
		$table = $inf['TABLE'][0];
		$prefix = $inf['TABLE'][1];
		$idfield = $inf['TABLE'][2];

		$vals = array();
		while (list($a,list($f,$t)) = each($fields))
		{
			if ($this->item == 'sources' && $f == 'type' && !@$this->info[$f]) { $this->info[$f] = PRICE; }
			if (strlen(@$this->info[$f])) { $vals[] = "${prefix}_$f='".mysql_escape_string($this->info[$f])."'"; }
		}
		if ($this->item != 'platforms') { $vals[] = "${prefix}_platformID=$this->platformID"; }

		if ($this->selID) { $sql = "UPDATE $table SET "; }
		else { $sql = "INSERT INTO $table SET "; }

		$sql .= implode(",",$vals);

		if ($this->selID) { $sql .= " WHERE ${prefix}_$idfield=$this->selID".($this->item!='platforms'?" AND ${prefix}_platformID=$this->platformID":""); }

		//echo "$sql<p />";

		$result = mysql_query($sql,$db);
		$isdupe = $this->error->mysql(__FILE__,__LINE__,YES);

		if ($isdupe == YES) { $this->status[0] = DUPLICATE; }
		else
		{
			if ($this->item == 'platforms' && !$this->selID)
			{
				// a new platform was added - create an 'Unknown Type' type
				$platformID = mysql_insert_id();
				$sql = "INSERT INTO types VALUES (NULL,$platformID,'Unknown Type')";
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}

			$this->status[0] = ($this->selID?EDITED:ADDED);
		}

		if ($this->selID) { return $this->selID; } else { return mysql_insert_id(); }
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
  * Deletes $this->selID and reassigns any items to the provided ID
  * @param integer $assignID
  */
  function delete($assignID)
  {
		global $db;

		$inf = $this->get_info();
		$fields = $inf['FIELDS'];
		$table = $inf['TABLE'][0];
		$prefix = $inf['TABLE'][1];
		$idfield = $inf['TABLE'][2];

		$sql = "DELETE FROM $table WHERE ${prefix}_$idfield=$this->selID";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		$sqls = array();
		if ($this->item == 'features')
		{
			// delete the itm_fet_rel entries
			$sqls[] = "DELETE FROM itm_fet_rel WHERE ifr_featureID=$this->selID";
		}
		elseif ($this->item == 'sources')
		{
			// delete the item_source_values entries
			$sqls[] = "DELETE FROM item_source_values WHERE isv_sourceID=$this->selID";
		}
		elseif ($this->item == 'fields')
		{
			// delete the item_field_values entries
			$sqls[] = "DELETE FROM item_field_values WHERE ifv_fieldID=$this->selID";
		}
		else
		{
			$noreassign = @$_POST['noreassign'];

			$update = array();
			$delete = array();

			if ($assignID)
			{
				// reassign the entries in items
				$sqls[] = "UPDATE items SET itm_$idfield=$assignID WHERE itm_$idfield=$this->selID";

				if ($this->item == 'platforms')
				{
					// all features, sources, types, companies, and fields must be altered
					// box discounts, cash/credit percentages,
					$update = array(
						array('companies','com'),
						array('features','fet'),
						array('item_fields','itf'),
						array('request_items','rqi'),
						array('request_platform_options','rpo'),
						array('sources','sor'),
						array('types','typ')
					);

					$delete = array(
						array('item_images','iti'),
						array('max_copies','mac'),
						array('min_prices','mnp'),
						array('percopy_discounts','pcd')
					);
				}
			}
			elseif ($noreassign)
			{
				if ($this->item == 'platforms')
				{
					// delete every instance of the platform (items, features, sources, types, companies, prices, quantities, etc)
					$delete = array(
						array('box_discounts','bod'),
						array('ccpercs','ccp'),
						array('companies','com'),
						array('features','fet'),
						array('item_fields','itf'),
						array('item_images','iti'),
						array('items','itm'),
						array('max_copies','mac'),
						array('min_prices','mnp'),
						array('percopy_discounts','pcd'),
						array('platforms','pla'),
						array('request_items','rqi'),
						array('request_platform_options','rpo'),
						array('sources','sor'),
						array('types','typ')
					);
				}
			}
		}

		while (list($a,list($table,$pre)) = each($update))
		{
			$sqls[] = "UPDATE $table SET {$pre}_platformID=$assignID WHERE {$pre}_platformID=$this->selID";
		}

		while (list($a,list($table,$pre)) = each($delete))
		{
			$sqls[] = "DELETE FROM $table WHERE {$pre}_platformID=$this->selID";
		}

		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}

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
  * Generate and show any error message(s) according to the contents of $this->status
  */
  function show_errors()
  {
    $errors = array();

		$inf = $this->get_info();
		$single = $inf['SINGLE'];

    for ($i=0; $i<count($this->status); $i++)
    {
      if ($this->status[$i] == DUPLICATE) { $error = "That $single already exists!"; }

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
	* Display the menu for the current platform with checks/Xs next to each item
	*/
	function show_menu($hr=YES,$force=NO)
	{
		if ($this->item != 'platforms' || $force == YES)
		{
			if ($hr == NO) { $this->show_platform(YES); }
			else { ?><p /><hr width="75%" size="-1" color="#CCCCCC" /><?php }

			?>
			<form method="post" action="platforms.php" name="load">
				<input type="hidden" name="act" value="load">
				<input type="hidden" name="platformID" value="<?=$this->platformID;?>">
				<input type="hidden" name="item" value="">
			</form>

			<script type="text/javascript">
				function load(item)
				{
					if (item == 'pricing') { alert('Under Construction'); }
					else
					{
						var frm = document.load;
						frm.item.value = item;
						frm.submit();
					}
				}
			</script>
			<?php


			$pages = array();
			while (list($item,$arr) = each($this->items))
			{
				if ($item != "platforms")
				{
					$this->set_item($item);
					if (count($this->values)) { $done = YES; } else { $done = NO; }
					$pages[] = array($done,$arr['TITLE'],$item);
				}
			}

			?><div style="text-align:left;width:175px"><?php
			while (list($a,list($done,$title,$item)) = each($pages))
			{
				$url = "javascript:load('$item')";
				?><img src="/images/<?=($done==YES?'check.gif':'x.gif');?>"> <a href="<?=$url;?>"><?=$title;?></a><br /><?php
			}
			?></div><?php
		}
	}

	/**
	* Outputs "Platform: <platform name>"
	* @param integer $chg Show the "Change" link? [optional, default NO]
	* @param string $chgurl If $chg is YES, URL for change link [optional, default '']
	* @param integer $p Show <p /> at the end? If NO, shows <br /> [optional, default YES]
	*/
	function show_platform($chg=NO,$chgurl="",$p=YES)
	{
		if ($chg == YES && !strlen($chgurl)) { $chgurl = $_SESSION['root_admin'].'setup_items/platforms.php'; }

		?><font size="2"><b>Platform:</b> <?=$this->platform_name();?><!-- (<?=$this->platformID;?>)--></font><?php
		if ($chg == YES) { ?> <font size="1">(<a href="<?=$chgurl;?>">Change</a>)</font><?php }
		if ($p) { ?><p /><?php } else { ?><br /><?php }
	}

	/**
	* Returns the selected platform name
	* @return string platform name
	*/
	function platform_name()
	{
		global $db;

		if ($this->platformID && !strlen($this->platform_name))
		{
			$sql = "SELECT pla_name FROM platforms WHERE pla_platformID=$this->platformID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			if (mysql_num_rows($result))
			{
				$row = mysql_fetch_assoc($result);
				$this->platform_name = $row['pla_name'];
			}
		}

		return $this->platform_name;
	}

	/**
	* Returns the number of items that would be affected by deleting a type, company, or platform
	* @return integer number of affected items
	*/
	function affected()
	{
		global $db;

		$affected = 0;

		$inf = $this->get_info();
		$idfield = $inf['TABLE'][2];

		$sql = "SELECT COUNT(*) AS count FROM items WHERE itm_$idfield=$this->selID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		$row = mysql_fetch_assoc($result);

		$affected += $row['count'];

		if ($this->item == 'platforms')
		{
			$sqls = array();
			$sqls[] = "SELECT COUNT(*) AS count FROM features WHERE fet_platformID=$this->selID";
			$sqls[] = "SELECT COUNT(*) AS count FROM sources WHERE sor_platformID=$this->selID";
			$sqls[] = "SELECT COUNT(*) AS count FROM types WHERE typ_platformID=$this->selID";
			$sqls[] = "SELECT COUNT(*) AS count FROM companies WHERE com_platformID=$this->selID";

			while (list($a,$sql) = each($sqls))
			{
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
				$row = mysql_fetch_assoc($result);

				$affected += $row['count'];
			}
		}

		return $affected;
	}

	/**
	* Returns the distinct companies
	* @return	array
	* @access	public
	*/
	function getDistinctCompanies()
	{
		global $db;

		$companies = array();

		$sql = 'SELECT DISTINCT com_name FROM companies ORDER BY com_name';
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			$companies[] = $row['com_name'];
		}

		return $companies;
	} // end function getDistinctPlatforms

	/**
	* Returns the distinct types
	* @return	array
	* @access	public
	*/
	function getDistinctTypes()
	{
		global $db;

		$types = array();

		$sql = 'SELECT DISTINCT typ_type FROM types ORDER BY typ_type';
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			$types[] = $row['typ_type'];
		}

		return $types;
	} // end function getDistinctTypes
} // end class platforms

?>