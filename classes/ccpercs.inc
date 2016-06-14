<?php
/**
* Cash/Credit Percentages class (adding/editing/removing)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class ccpercs
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
	* Percentages
	* @var array $percs
	*/
	var $percs;

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
	function ccpercs(&$pg)
	{
		$this->pg =& $pg;
		$this->storeID = $_SESSION['storeID'];
		$this->percs = array();
		$this->status = array();

		$this->error = new error('Cash/Credit %');

		$this->get_percs();
	}

	/**
	* Pull in the current percentages
	*/
	function get_percs()
	{
		global $db;

		$this->percs = array();

		$sql = "SELECT * FROM ccpercs WHERE ccp_storeID=$this->storeID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			$crv = (strlen($row['ccp_credit'])?($row['ccp_credit']*100):'');
			$this->percs[$row['ccp_platformID']] = array($crv);
		}

		if (!isset($this->percs[0])) { $this->percs[0] = array(''); }
	}

	/**
	* Outputs the listing of percentages with textboxes to change for each platform
	*/
	function percs_form()
	{
		?>
		Please enter the percentage of the used price that is given for credit.
		<p />
		Any values may be left blank; the <b>Default</b> values will be used.
		<p />
		<b>NOTE:</b> The values are percentages.  Enter &quot;75&quot;, not &quot;.75&quot;.<br />
		1/2 credit will always be given for cash.
		<p />

		<script type="text/javascript">
			function verify(frm)
			{
				if (frm.credit0.value.length == 0) { alert('You must enter a default credit percentage.'); frm.credit0.focus(); return false; }
				else { return true; }
			}
		</script>
		<?php

		$this->pg->outlineTableHead();
		?>

		<form method="post" action="/admin/setup_invoice/ccpercs.php" name="ccpfrm" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="set">
		<tr>
			<td bgcolor="<?=$this->pg->color('table-head');?>"><b>Platform</b></td>
			<td align="center" bgcolor="<?=$this->pg->color('table-head');?>"><b>Credit</b></td>
		</tr>
		<?php
		$pla = new platforms($this->pg,0);
		$pla->set_item('platforms');
		$pla->values = array_merge(array(0=>array(0,'<b>Default</b>')),$pla->values);
		while (list($a,$arr) = each($pla->values))
		{
			$id = $arr[0];
			$name = $arr[1];

			$credit = @$this->percs[$id][0];

			?>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-cell');?>"><?=$name;?></td>
				<td align="center" bgcolor="<?=$this->pg->color('table-cell2');?>">
					<input type="text" name="credit[<?=$id;?>]" size="5" value="<?=$credit;?>" id="credit<?=$id;?>" style="vertical-align:middle" onkeypress="return onlynumbers(this.value,event,false)" style="text-align:right">%
				</td>
			</tr>
			<?php
		}
		$this->pg->outlineTableFoot();

		?>
		<p />
		<input type="submit" value="Update Percentages &gt;" class="btn">
		</form>
		<?php

		$this->pg->addOnload('document.ccpfrm.credit0.focus()');
	}

	/**
	* Pulls the information from $_POST
	*/
	function pull_post()
	{
		$credit = $_POST['credit'];
		$platformIDs = array_keys($credit);

		$this->percs = array();

		while (list($a,$platformID) = each($platformIDs))
		{
			//$this->percs[$platformID] = array((strlen(@$cash[$platformID])?$cash[$platformID]:$defs[0]),(strlen(@$credit[$platformID])?$credit[$platformID]:$defs[1]));
			$this->percs[$platformID] = array(@$credit[$platformID]);
		}

		ksort($this->percs);
	}

	/**
	* Set the percentages for the platforms in $this->percs
	*/
	function set()
	{
		global $db;

		$sqls = array("DELETE FROM ccpercs WHERE ccp_storeID={$_SESSION['storeID']}");

		$vals = array();

		while (list($platformID,list($credit)) = each($this->percs))
		{
			$crv = (strlen($credit)?($credit/100):'');
			$vals[] = "($platformID,$this->storeID,'$crv')";
		}
		$sqls[] = "INSERT INTO ccpercs VALUES ".implode(',',$vals);

		while (list($a,$sql) = each($sqls))
		{
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}

		$this->status[0] = SET;
	}

  /**
  * Check the status of the cash/credit percentage set
  * @return integer YES/NO
  */
  function was_set()
  {
    if ($this->status[0] == SET) { return YES; }
    else { return NO; }
  }
}

?>