<?php
/**
* Online store - store priorities
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class store_priorities
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
	* All stores
	* @var array $stores
	*/
	var $stores;

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	*/
	function store_priorities(&$pg)
	{
		$this->pg =& $pg;
		$this->stores = array();

		$this->error = new error('Store Inventory Priorities class');

		$this->get_stores();
	}

	/**
	* Pull in all of the stores and order by priority
	*/
	function get_stores()
	{
		$this->stores = array();

		$sto = new stores($this->pg);
		$sto->get_stores(YES);

		$stores = $sto->stores;
		$prios = array(); // format: $prios[storeID] = #
		while (list($a,$arr) = each($stores)) { $prios[$arr['sto_storeID']] = $arr['sto_priority']; }
		reset($stores);
		asort($prios);

		$newstores = array();
		while (list($storeID,$priority) = each($prios))
		{
			while (list($a,$arr) = each($stores))
			{
				if ($arr['sto_storeID'] == $storeID) { $newstores[] = $arr; break; }
			}
			reset($stores);
		}
		$this->stores = $newstores;
	}

	/**
	* Output the priority form
	*/
	function show_stores()
	{
		?>
		Select the store order in which inventory will be sent from<br />
		when an order is placed in the online store:
		<p />
		<script language="javascript" src="/scripts/listbox.js"></script>
		<form method="post" action="/admin/setup_misc/priorities.php" name="prio" onsubmit="selectAllOptions(this.elements['priority[]'])">
		<input type="hidden" name="act" value="set">
		<table border="0" cellspacing="0" cellpadding="10">
			<tr>
				<td>
					<select name="priority[]" size="<?=count($this->stores);?>" multiple="multiple">
						<?php
						while (list($a,$arr) = each($this->stores))
						{
							?><option value="<?=$arr['sto_storeID'];?>"><?=$arr['sto_name'];?></option><?php
						}
						?>
					</select>
				</td>
				<td valign="middle">
					<input type="button" value="Move Up" onclick="moveOptionUp(this.form.elements['priority[]'])" class="btn">
					<p />
					<input type="button" value="Move Down" onclick="moveOptionDown(this.form.elements['priority[]'])" class="btn">
				</td>
			</tr>
		</table>
		<p />
		<input type="submit" value="Set Priorities &gt;" class="btn"> <input type="button" value="Reset Priorities &gt;" onclick="document.location='/admin/setup_misc/priorities.php'" class="btn">
		</form>
		<?php
	}

	/**
	* Sets the store priorities
	*/
	function set_priorities()
	{
		global $db;

		$prios = @$_POST['priority'];

		if (is_array($prios))
		{
			$sqls = array();

			while (list($priority,$storeID) = each($prios))
			{
				$sqls[] = "UPDATE stores SET sto_priority=$priority WHERE sto_storeID=$storeID";
			}

			while (list($a,$sql) = each($sqls))
			{
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}
		}
	}
}

?>