<?php
/**
* Maximum copies class (adding/editing/removing)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class max_copies
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
	* Max copies
	* @var array $copies
	*/
	var $copies;

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
	function max_copies(&$pg)
	{
		$this->pg =& $pg;
		$this->storeID = $_SESSION['storeID'];
		$this->copies = array();
		$this->status = array();

		$this->error = new error('Maximum Copies');

		$this->get_maxcopies();
	}

	/**
	* Pull in the current maximum copies
	*/
	function get_maxcopies()
	{
		global $db;

		$this->copies = array();

		$sql = "SELECT * FROM max_copies WHERE mac_storeID=$this->storeID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			$this->copies[$row['mac_platformID']] = array($row['mac_new'],$row['mac_used']);
		}
	}

	/**
	* Outputs the listing of maximum copies with textboxes to change for each platform
	*/
	function maxcopies_form()
	{
		?>
		Please enter the maximum number of copies per item allowed for each platform.
		<p />
		If any values are left blank, an infinite number of copies will be allowed.
		<p />
		<?php

		$this->pg->outlineTableHead();
		?>

		<form method="post" action="/admin/setup_invoice/max_copies.php" name="mcfrm">
		<input type="hidden" name="act" value="set">
		<tr>
			<td bgcolor="<?=$this->pg->color('table-head');?>"><b>Platform</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b>New</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b>Used</b></td>
		</tr>
		<?php
		$pla = new platforms($this->pg,0);
		$pla->set_item('platforms');
		while (list($a,$arr) = each($pla->values))
		{
			$id = $arr[0];
			$name = $arr[1];

			$nc = @$this->copies[$id][0];
			$uc = @$this->copies[$id][1];

			?>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-cell');?>"><?=$name;?></td>
				<td align="center" bgcolor="<?=$this->pg->color('table-cell2');?>">
					<input type="text" name="new[<?=$id;?>]" size="3" value="<?=$nc;?>" id="nc<?=$id;?>" style="vertical-align:middle" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right">
				</td>
				<td align="center" bgcolor="<?=$this->pg->color('table-cell2');?>">
					<input type="text" name="used[<?=$id;?>]" size="3" value="<?=$uc;?>" id="uc<?=$id;?>" style="vertical-align:middle" onkeypress="return onlynumbers(this.value,event,true)" style="text-align:right">
				</td>
			</tr>
			<?php
		}
		$this->pg->outlineTableFoot();

		?>
		<p />
		<input type="submit" value="Update Maximum Copies &gt;" class="btn">
		</form>
		<?php

		//$this->pg->addOnload('document.mcfrm.mc0.focus()');
	}

	/**
	* Pulls the information from $_POST
	*/
	function pull_post()
	{
		$new = $_POST['new'];
		$used = $_POST['used'];
		$platformIDs = array_keys($new);

		$this->copies = array();

		while (list($a,$platformID) = each($platformIDs))
		{
			$this->copies[$platformID] = array(@$new[$platformID],@$used[$platformID]);
		}

		ksort($this->copies);
	}

	/**
	* Set the maximum copies for the platforms in $this->copies
	*/
	function set()
	{
		global $db;

		$sqls = array("DELETE FROM max_copies WHERE mac_storeID={$_SESSION['storeID']}");

		$vals = array();

		while (list($platformID,list($nc,$uc)) = each($this->copies))
		{
			$vals[] = "($platformID,$this->storeID,'$nc','$uc')";
		}
		$sqls[] = "INSERT INTO max_copies VALUES ".implode(',',$vals);

		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}

		$this->status[0] = SET;
	}

  /**
  * Check the status of the maximum copy set
  * @return integer YES/NO
  */
  function was_set()
  {
    if ($this->status[0] == SET) { return YES; }
    else { return NO; }
  }
}

?>