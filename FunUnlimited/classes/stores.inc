<?php
/**
* Store handling class (adding/editing/removing)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class stores
{
	/**
	* page class reference
	* @var reference $pg
	*/
	var $pg;

	/**
	* Error reporting class
	* @var identifier $error
	*/
	var $error;

	/**
	* Current storeID
	* @var integer $storeID
	*/
	var $storeID;

	/**
	* Store information
	* @var array $info
	*/
	var $info;

  /**
  * Status variable used for error reporting (bad email, bad zip code, success, etc)
  * @var array $status
  */
  var $status;

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	*/
	function stores(&$pg)
	{
		$this->pg =& $pg;
		$this->storeID = 0;
		$this->info = array();
		$this->status = array();

		$this->error = new error('Store');
	}

	/**
	* Outputs the listing of stores with buttons to add/edit/remove
	* @param integer $active Active stores YES/NO [optional, default YES]
	*/
	function show_stores($active=YES)
	{
		global $db;

		// pull in the stores
		$this->get_stores($active);

		// output the table
		$this->pg->outlineTableHead();

		?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-head');?>">&nbsp;</td>
			<td bgcolor="<?=$this->pg->color('table-head');?>" align="center"><b>Name</b></td>
			<td bgcolor="<?=$this->pg->color('table-head');?>" align="center"><b>Location</b></td>
			<td bgcolor="<?=$this->pg->color('table-head');?>" align="center"><b>Phone</b></td>
		</tr>
		<?php

		while (list($a,$arr) = each($this->stores))
		{
			$bg = (($a%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

			$phone = array();
			if (strlen($arr['sto_phone'])) { $phone[] = "Phone: ".$this->pg->format('phone',$arr['sto_phone']); }
			if (strlen($arr['sto_fax'])) { $phone[] = "Fax: ".$this->pg->format('phone',$arr['sto_fax']); }
			?>
			<label for="<?=$arr['sto_storeID'];?>">
			<tr style="cursor:hand">
				<td bgcolor="<?=$bg;?>"><input type="radio" name="storeID" id="<?=$arr['sto_storeID'];?>" style="border-width:0" onclick="set_storeID(<?=$arr['sto_storeID'];?>)"></td>
				<td bgcolor="<?=$bg;?>"><b><?=$arr['sto_name'];?></b></td>
				<td bgcolor="<?=$bg;?>" valign="top"><?=$arr['sto_address'];?><br /><?=$arr['sto_city'];?>, <?=$arr['sto_state'];?> <?=$this->pg->format('zip',$arr['sto_zip']);?></td>
				<td bgcolor="<?=$bg;?>" valign="top"><?=implode("<br />",$phone);?></td>
			</tr>
			</label>
			<?php
		}
		if (!count($this->stores)) { ?><tr><td colspan="4" bgcolor="<?=$this->pg->color('table-cell');?>" align="center">--- No <?=($active==YES?'Active':'Inactive');?> Stores ---</td></tr><?php }

		$this->pg->outlineTableFoot();

		?>
		<p />
		<?php

		$this->get_stores(($active==YES?NO:YES));
		$total = count($this->stores);
		?>

		<script type="text/javascript">
			var storeID=0;
			function set_storeID(id)
			{
				storeID = id;
				stores.e.disabled = false;
				stores.d.disabled = false;
				<?=($active==NO?'stores.pd.disabled = false;':'');?>
			}

			function go(where)
			{
				if (where == 'add' || (storeID && where == 'edit') || (storeID && where == 'activate' && confirm('Are you SURE you want to <?=($active==NO?'re':'de');?>activate the selected store?')) || (storeID && where == 'permdelete' && confirm('Are you ABSOLUTELY SURE you want to permanently delete the selected store?\n\nPlease note that this is NOT REVERSIBLE!')))
				{
					if (where == 'add') { storeID = 0; }

					var frm = document.stores;
					frm.act.value = where;
					frm.storeID.value = storeID;
					frm.submit();
				}
			}
		</script>

		<form method="post" action="/admin/setup_misc/stores.php" name="stores">
		<input type="hidden" name="act" value="">
		<input type="hidden" name="storeID" value="">
		<input type="hidden" name="active" value="<?=$active;?>">
		<input type="button" value="Add New Store &gt;" onclick="go('add')" class="btn"> <input type="button" value="Edit Store &gt;" id="e" onclick="go('edit')" disabled="true" class="btn">
		<?php
		if ($active == YES) { ?><input type="button" value="Deactivate Store &gt;" id="d" onclick="go('activate')" disabled="true" class="btn"><?php }
		else
		{
			?>
			<input type="button" value="Reactivate Store &gt;" id="d" onclick="go('activate')" disabled="true" class="btn">
			<p />
			<input type="button" value="Permanently Delete Store &gt;" id="pd" onclick="go('permdelete')" disabled="true" class="btn">
			<?php
		}
		?>
		</form>
		<?php

		if ($active == NO || $total)
		{
			?>
			<p />
			<b>Viewing:</b> <?=($active==YES?'Active':'Inactive');?> Stores (<a href="/admin/setup_misc/stores.php?active=<?=($active==YES?NO:YES);?>">View <?=$total;?> <?=($active==YES?'Inactive':'Active');?> Store<?=($total==1?'':'s');?></a>)
			<?php
		}
	}

	/**
	* Pulls all stores into $this->stores
	* @param integer $active Active stores YES/NO [optional, default YES]
	*/
	function get_stores($active=YES)
	{
		global $db;

		$this->stores = array();

		$sql = "SELECT * FROM stores WHERE sto_active=".$active." ORDER BY sto_name";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result)) { $this->stores[] = $row; }
	}

	/**
	* Pulls all stores into $this->stores (calls $this->get_stores())
	* @param integer $active Active stores YES/NO [optional, default YES]
	*/
	function setStores($active=YES)
	{
		$this->get_stores($active);
	}

	/**
	* Returns all stores
	* @return array
	*/
	function getStores()
	{
		return $this->stores;
	}

	/**
	* Sets the storeID and pulls the store information into $this->info
	* @param integer $storeID
	*/
	function set_storeID($storeID)
	{
		global $db;

		$this->storeID = $storeID;
		$this->info = array();

		$sql = "SELECT * FROM stores WHERE sto_storeID=$storeID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			// remove 'sto_'
			while (list($key,$val) = each($row))
			{
				$this->info[substr($key,4)] = $val;
			}
		}
	}

	/**
	* Displays the add/edit store form
	* @param integer $active
	*/
	function add_form($active)
	{
		$val = new validate();

		if ($this->storeID) { $word = "edit"; } else { $word = "add"; }

		?>
		<form method="post" action="/admin/setup_misc/stores.php" name="store" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="doadd">
		<input type="hidden" name="storeID" value="<?=$this->storeID;?>">
		<input type="hidden" name="info[active]" value="<?=$active;?>">
		<?php
		$this->pg->outlineTableHead();
		?>
		<tr>
			<td colspan="2" bgcolor="<?=$this->pg->color('table-label');?>" align="center"><b><?=ucwords($word);?> Store</b></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Name:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[name]" size="50" value="<?=@$this->info['name'];?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Address:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[address]" size="40" value="<?=@$this->info['address'];?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>City, State Zip:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>">
				<input type="text" name="info[city]" size="20" value="<?=@$this->info['city'];?>">
				<select name="info[state]" size="1"><?php
					$st = new states();

					while (list($abb,$name) = each($st->states))
					{
						if ($abb == @$this->info['state']) { $s = ' selected="selected"'; } else { $s = ''; }
						?><option value="<?=$abb;?>"<?=$s;?>><?=$name;?></option><?php
					}
				?></select>
				<input type="text" name="info[zip]" size="12" maxlength="10" value="<?=$this->pg->format('zip',$val->strip(@$this->info['zip']));?>">
			</td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Phone Number:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[phone]" size="16" value="<?=$this->pg->format('phone',$val->strip(@$this->info['phone']));?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>">Fax Number:</td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[fax]" size="16" value="<?=$this->pg->format('phone',$val->strip(@$this->info['fax']));?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>">E-Mail Address:</td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[email]" size="35" value="<?=@$this->info['email'];?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Sales Tax Percentage:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[taxperc]" size="10" value="<?=trim(@$this->info['taxperc'],'0');?>" style="text-align:right">%</td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Default Area Code:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[areacode]" size="3" maxlength="3" value="<?=@$this->info['areacode'];?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Restocking Fee %:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[charge_perc]" size="10" value="<?=trim(@$this->info['charge_perc'],'0');?>" style="text-align:right">%</td>
		</tr>
		<?php
		if (!$this->storeID)
		{
			?>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Username:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[username]" size="10" value="<?=@$this->info['username'];?>"></td>
			</tr>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Password:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="password" name="info[password]" size="8" maxlength="8"></td>
			</tr>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Confirm Password:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="password" name="info[password2]" size="8" maxlength="8"></td>
			</tr>
			<?php
		}

		$this->pg->outlineTableFoot();
		?>
		<p />
		<input type="submit" value="<?=ucwords($word);?> Store &gt;" class="btn"> <input type="reset" value="Reset Form &gt;" class="btn">
		</form>
		<?=$this->pg->cancel($_SESSION['root_admin'].'setup_misc/stores.php');?>

		<script type="text/javascript">
			function verify(frm)
			{
				var foc;
				if (frm.elements['info[name]'].value == '') { alert('You must enter a name.'); foc = frm.elements['info[name]']; }
				else if (frm.elements['info[address]'].value == '') { alert('You must enter an address.'); foc = frm.elements['info[address]']; }
				else if (frm.elements['info[city]'].value == '') { alert('You must enter a city.'); foc = frm.elements['info[city]']; }
				else if (frm.elements['info[state]'].options[frm.elements['info[state]'].selectedIndex].value == '') { alert('You must select a state.'); foc = frm.elements['info[state]']; }
				else if (frm.elements['info[zip]'].value == '') { alert('You must enter a zip.'); foc = frm.elements['info[zip]']; }
				else if (frm.elements['info[phone]'].value == '') { alert('You must enter a phone number.'); foc = frm.elements['info[phone]']; }
				else if (frm.elements['info[taxperc]'].value == '') { alert('You must enter a sales tax percentage.'); foc = frm.elements['info[taxperc]']; }
				else if (frm.elements['info[areacode]'].value == '') { alert('You must enter an area code.'); foc = frm.elements['info[areacode]']; }
				else if (frm.elements['info[charge_perc]'].value == '') { alert('You must enter a credit card charge percentage.'); foc = frm.elements['info[charge_perc]']; }
				<?php
				if (!$this->storeID)
				{
					?>
					else if (frm.elements['info[username]'].value == '') { alert('You must enter a username.'); foc = frm.elements['info[username]']; }
					else if (frm.elements['info[password]'].value == '') { alert('You must enter a password.'); foc = frm.elements['info[password]']; }
					else if (frm.elements['info[password]'].value != frm.elements['info[password2]'].value) { alert('Passwords do not match.'); foc = frm.elements['info[password]']; }
					<?php
				}
				?>

				if (foc) { foc.focus(); return false; }
				else { return true; }
			}
		</script>
		<?php

		$this->pg->addOnload("document.store.elements['info[name]'].focus()");
	}

	/**
	* Pulls the information from $_POST
	*/
	function pull_post()
	{
		$this->storeID = $_POST['storeID'];
		$this->info = $_POST['info'];
		while (list($key,$val) = each($this->info)) { $this->info[$key] = stripslashes($val); }
		reset($this->info);
	}

	/**
	* Adds/edits the store stored in $this->info
	*/
	function add()
	{
		global $db;

		if ($this->validate())
		{
			// add the store to the database

			if ($this->storeID) { $sql = "UPDATE stores SET "; }
			else { $sql = "INSERT INTO stores SET "; }

			$val = new validate();
			$this->info['zip'] = $val->strip($this->info['zip']);
			$this->info['phone'] = $val->strip($this->info['phone']);
			$this->info['fax'] = $val->strip($this->info['fax']);

			$vals = array();
			while (list($key,$val) = each($this->info))
			{
				if ($key != 'username' && $key != 'password' && $key != 'password2') { $vals[] = "sto_".$key."='".mysql_real_escape_string($val)."'"; }
			}
			$sql .= implode(",",$vals);

			if ($this->storeID) { $sql .= " WHERE sto_storeID=$this->storeID"; }

			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$storeID = mysql_insert_id();

			if (!$this->storeID)
			{
				// set the store's priority to the lowest
				$sql = "UPDATE stores SET sto_priority=$storeID WHERE sto_storeID=$storeID";
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);

				// add the default user
				$sql = "INSERT INTO users VALUES (NULL,$storeID,'Store Administrator - ".mysql_real_escape_string($this->info['name'])."','".$this->info['username']."',ENCODE('".$this->info['password']."','".ENCODEKEY."'),".STORE.")";
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}

			$this->status[0] = ADDED;
		}
	}

	/**
	* Re/de-activates the store stored in $this->storeID (sets to inactive - doesn't delete)
	* @param integer $active If YES, deactivate the store. If no, reactivate the store. [optional, default YES]
	*/
	function activate($active=YES)
	{
		global $db;

		if ($this->storeID)
		{
			// re/de-activate the store
			$sql = "UPDATE stores SET sto_active=".($active==YES?NO:YES)." WHERE sto_storeID=$this->storeID";

			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$this->status[0] = DELETED;
		}
	}

	/**
	* Permanently deletes the store stored in $this->storeID and all traces of it (items, quantity, pricing, etc)
	*/
	function permdelete()
	{
		global $db;

		if ($this->storeID)
		{
			$sqls = array(
				"DELETE FROM stores WHERE sto_storeID=$this->storeID",
				"DELETE FROM users WHERE use_storeID=$this->storeID"
			);

			while (list($a,$sql) = each($sqls))
			{
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}

			$this->status[0] = DELETED;
		}
	}

  /**
  * Check the status of the store addition/edit
  * @return integer YES/NO
  */
  function was_added()
  {
    if ($this->status[0] == ADDED || $this->status[0] == EDITED) { return YES; }
    else { return NO; }
  }

  /**
  * Check the status of the store re/de-activation/deletion
  * @return integer YES/NO
  */
  function was_deleted()
  {
    if ($this->status[0] == DELETED) { return YES; }
    else { return NO; }
  }

  /**
  * Validates information in $this->info (phone numbers, zip codes, emails, etc) before $this->add() can be completed
  * @return integer YES/NO
  */
  function validate()
  {
    $val = new validate();

    $this->status = array();

    if (!$val->zip_code($this->info['zip'])) { $this->status[] = BADZIP; }
    if (!$val->phone($this->info['phone'])) { $this->status[] = BADPHONE; }
    if (strlen($this->info['fax']) && !$val->phone($this->info['fax'])) { $this->status[] = BADFAXPHONE; }
    if (strlen($this->info['email']) && !$val->email($this->info['email'])) { $this->status[] = BADEMAIL; }

    if (count($this->status)) { return NO; } else { return YES; }
  }

  /**
  * Generate and show any error message(s) according to the contents of $this->status
  */
  function show_errors()
  {
    $errors = array();

    for ($i=0; $i<count($this->status); $i++)
    {
      if ($this->status[$i] == BADEMAIL) { $error = "Invalid e-mail address."; }
      elseif ($this->status[$i] == BADZIP) { $error = "Invalid zip code."; }
      elseif ($this->status[$i] == BADPHONE) { $error = "Invalid phone number."; }
      elseif ($this->status[$i] == BADFAXPHONE) { $error = "Invalid fax phone number."; }

      $errors[] = $error;
    }

    if ($this->storeID) { $word = "Edit"; } else { $word = "Add"; }
    $this->pg->error($errors);
    echo "<p />Please correct the error".(count($errors)==1?'':'s')." before pressing <b>$word Store</b> again.<p />";
  }

	/**
	* Obtain a store's name
	* @param integer $storeID
	* @return string the store's name
	*/
	function getStoreName($storeID)
	{
		global $db;

		$sql = "SELECT sto_name FROM stores WHERE sto_storeID=$storeID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		if (mysql_num_rows($result))
		{
			$row = mysql_fetch_assoc($result);
			$name = $row['sto_name'];
		}

		return @$name;
	}
}

?>