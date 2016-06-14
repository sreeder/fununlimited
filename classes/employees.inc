<?php
/**
* Employee handling class (adding/editing/removing)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class employees
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
	* Current employeeID
	* @var integer $employeeID
	*/
	var $employeeID;

	/**
	* Employee information
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
	function employees(&$pg)
	{
		$this->pg =& $pg;
		$this->storeID = $_SESSION['storeID'];
		$this->employeeID = 0;
		$this->info = array();
		$this->status = array();

		$this->error = new error('Employees');
	}

	/**
	* Outputs the listing of employees with buttons to add/edit/remove
	* @param integer $active Active employees YES/NO [optional, default YES]
	*/
	function show_employees($active=YES)
	{
		global $db;

		// pull in the employees
		$this->get_employees($active);

		// output the table
		$this->pg->outlineTableHead();

		?>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-head');?>">&nbsp;</td>
			<td bgcolor="<?=$this->pg->color('table-head');?>" align="center"><b>Name</b></td>
			<td bgcolor="<?=$this->pg->color('table-head');?>" align="center"><b>Address</b></td>
			<td bgcolor="<?=$this->pg->color('table-head');?>" align="center"><b>Phone</b></td>
			<!--
			<td colspan="2" bgcolor="<?=$this->pg->color('table-head');?>" align="center"><b>Sales</b></td>
			<td colspan="2" bgcolor="<?=$this->pg->color('table-head');?>" align="center"><b>Trades</b></td>
			-->
		</tr>
		<?php

		while (list($a,$arr) = each($this->employees))
		{
			$bg = (($a%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));
			$bg2 = $this->pg->color('table-head-lighter');

			?>
			<label for="<?=$arr['emp_employeeID'];?>">
			<tr style="cursor:hand">
				<td rowspan="1" bgcolor="<?=$bg;?>"><input type="radio" name="employeeID" id="<?=$arr['emp_employeeID'];?>" style="border-width:0" onclick="set_employeeID(<?=$arr['emp_employeeID'];?>)"></td>
				<td rowspan="1" bgcolor="<?=$bg;?>"><b><?=$arr['emp_lname'];?>, <?=$arr['emp_fname'];?></b></td>
				<td rowspan="1" bgcolor="<?=$bg;?>" valign="middle"><?=$arr['emp_address'];?><br /><?=$arr['emp_city'];?>, <?=$arr['emp_state'];?> <?=$this->pg->format('zip',$arr['emp_zip']);?></td>
				<td rowspan="1" bgcolor="<?=$bg;?>" valign="middle"><?=$this->pg->format('phone',$arr['emp_phone']);?></td>
				<!--
				<td bgcolor="<?=$bg2;?>" class="nine" align="center" valign="top"><b>Month</b></td>
				<td bgcolor="<?=$bg2;?>" class="nine" align="center" valign="top"><b>YTD</b></td>
				<td bgcolor="<?=$bg2;?>" class="nine" align="center" valign="top"><b>Month</b></td>
				<td bgcolor="<?=$bg2;?>" class="nine" align="center" valign="top"><b>YTD</b></td>
				-->
			</tr>
			<!--
			<tr>
				<td bgcolor="<?=$bg;?>" class="nine" valign="top">
					<?=$arr['data']['month']['sale']['total'];?> sale<?=($arr['data']['month']['sale']['total']==1?'':'s');?><br />
					<?=$arr['data']['month']['sale']['items'];?> item<?=($arr['data']['month']['sale']['items']==1?'':'s');?><br />
					$<?=number_format($arr['data']['month']['sale']['amount'],2);?>
				</td>
				<td bgcolor="<?=$bg;?>" class="nine" valign="top">
					<?=$arr['data']['ytd']['sale']['total'];?> sale<?=($arr['data']['ytd']['sale']['total']==1?'':'s');?><br />
					<?=$arr['data']['ytd']['sale']['items'];?> item<?=($arr['data']['ytd']['sale']['items']==1?'':'s');?><br />
					$<?=number_format($arr['data']['ytd']['sale']['amount'],2);?>
				</td>
				<td bgcolor="<?=$bg;?>" class="nine" valign="top">
					<?=$arr['data']['month']['trade']['total'];?> sale<?=($arr['data']['month']['trade']['total']==1?'':'s');?><br />
					<?=$arr['data']['month']['trade']['items'];?> item<?=($arr['data']['month']['trade']['items']==1?'':'s');?><br />
					$<?=number_format($arr['data']['month']['trade']['amount'],2);?>
				</td>
				<td bgcolor="<?=$bg;?>" class="nine" valign="top">
					<?=$arr['data']['ytd']['trade']['total'];?> sale<?=($arr['data']['ytd']['trade']['total']==1?'':'s');?><br />
					<?=$arr['data']['ytd']['trade']['items'];?> item<?=($arr['data']['ytd']['trade']['items']==1?'':'s');?><br />
					$<?=number_format($arr['data']['ytd']['trade']['amount'],2);?>
				</td>
			</tr>
			-->
			</label>
			<?php
		}
		reset($this->employees);
		if (!count($this->employees)) { ?><tr><td colspan="8" bgcolor="<?=$this->pg->color('table-cell');?>" align="center">--- No <?=($active==YES?'Active':'Inactive');?> Employees ---</td></tr><?php }

		$this->pg->outlineTableFoot();

		?>
		<p />
		<?php

		$this->get_employees(($active==YES?NO:YES));
		$total = count($this->employees);
		?>

		<script type="text/javascript">
			var employeeID=0;
			function set_employeeID(id)
			{
				employeeID = id;
				employees.e.disabled = false;
				employees.d.disabled = false;
				//employees.h.disabled = false;
				<?=($active==NO?'employees.pd.disabled = false;':'');?>
			}

			function go(where)
			{
				if (where == 'history' || where == 'add' || (employeeID && where == 'edit') || (employeeID && where == 'activate' && confirm('Are you SURE you want to <?=($active==NO?'re':'de');?>activate the selected employee?')) || (employeeID && where == 'permdelete' && confirm('Are you ABSOLUTELY SURE you want to permanently delete the selected employee?\n\nPlease note that this is NOT REVERSIBLE and that you will lose any invoice history related to this employee!')))
				{
					var frm = document.employees;
					frm.act.value = where;
					frm.employeeID.value = employeeID;
					frm.submit();
				}
			}
		</script>

		<form method="post" action="/admin/setup_misc/employees.php" name="employees">
		<input type="hidden" name="act" value="">
		<input type="hidden" name="employeeID" value="">
		<input type="hidden" name="active" value="<?=$active;?>">
		<input type="button" value="Add New Employee &gt;" onclick="go('add')" class="btn"> <input type="button" value="Edit Employee &gt;" id="e" onclick="go('edit')" disabled="true" class="btn">
		<?php
		if ($active == YES) { ?><input type="button" value="Deactivate Employee &gt;" id="d" onclick="go('activate')" disabled="true" class="btn"><?php }
		else
		{
			?>
			<input type="button" value="Reactivate Employee &gt;" id="d" onclick="go('activate')" disabled="true" class="btn">
			<p />
			<input type="button" value="Permanently Delete Employee &gt;" id="pd" onclick="go('permdelete')" disabled="true" class="btn">
			<?php
		}
		?>
		<!--
		<p />
		<input type="button" value="Employee History &gt;" onclick="go('history')" id="h" disabled="true" class="btn">
		-->
		</form>
		<?php

		if ($active == NO || $total)
		{
			?>
			<p />
			<b>Viewing:</b> <?=($active==YES?'Active':'Inactive');?> Employees (<a href="/admin/setup_misc/employees.php?active=<?=($active==YES?NO:YES);?>">View <?=$total;?> <?=($active==YES?'Inactive':'Active');?> Employee<?=($total==1?'':'s');?></a>)
			<?php
		}
	}

	/**
	* Pulls all employees into $this->employees
	* @param integer $active Active employees YES/NO/BOTH [optional, default YES]
	*/
	function get_employees($active=YES)
	{
		global $db;

		$this->employees = array();

		if ($active == BOTH) { $active = '0,1'; }

		$sql = "SELECT * FROM employees WHERE emp_storeID=$this->storeID AND emp_active IN (".$active.") ORDER BY emp_lname,emp_fname";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			$emplID = $row['emp_employeeID'];

			// pull the sales/trades data for this employee
			$data = array(
				'month'=>array(
					'sale'=>array('total'=>rand(1,5),'items'=>rand(5,20),'amount'=>(rand(30,100)+(rand(0,99)/100))),
					'trade'=>array('total'=>rand(1,5),'items'=>rand(5,20),'amount'=>(rand(30,100)+(rand(0,99)/100)))
				),
				'ytd'=>array(
					'sale'=>array('total'=>rand(1,5),'items'=>rand(5,20),'amount'=>(rand(30,100)+(rand(0,99)/100))),
					'trade'=>array('total'=>rand(1,5),'items'=>rand(5,20),'amount'=>(rand(30,100)+(rand(0,99)/100)))
				)
			);

			$this->employees[$emplID] = array_merge($row,array('data'=>$data));
		}
	}

	/**
	* Sets the employeeID and pulls the employee information into $this->info
	* @param integer $employeeID
	*/
	function set_employeeID($employeeID)
	{
		global $db;

		$this->employeeID = $employeeID;
		$this->info = array();

		$sql = "SELECT * FROM employees WHERE emp_employeeID=$employeeID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			// remove 'emp_'
			while (list($key,$val) = each($row))
			{
				$this->info[substr($key,4)] = $val;
			}
		}
	}

	/**
	* Returns the information for an employee
	* @param integer $employeeID
	*/
	function get_employee($employeeID)
	{
		return (isset($this->employees[$employeeID]) ? $this->employees[$employeeID] : array());
	}

	/**
	* Displays the add/edit employee form
	* @param integer $active
	*/
	function add_form($active)
	{
		$val = new validate();

		if ($this->employeeID) { $word = "edit"; } else { $word = "add"; }

		?>
		<form method="post" action="/admin/setup_misc/employees.php" name="employee" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="doadd">
		<input type="hidden" name="employeeID" value="<?=$this->employeeID;?>">
		<input type="hidden" name="info[storeID]" value="<?=$this->storeID;?>">
		<input type="hidden" name="info[active]" value="<?=$active;?>">
		<?php
		$this->pg->outlineTableHead();
		?>
		<tr>
			<td colspan="2" bgcolor="<?=$this->pg->color('table-label');?>" align="center"><b><?=ucwords($word);?> Employee</b></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>First Name:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[fname]" size="25" value="<?=@$this->info['fname'];?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Last Name:</b></td>
			<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="info[lname]" size="30" value="<?=@$this->info['lname'];?>"></td>
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
		<?php

		$this->pg->outlineTableFoot();
		?>
		<p />
		<input type="submit" value="<?=ucwords($word);?> Employee &gt;" class="btn"> <input type="reset" value="Reset Form &gt;" class="btn">
		</form>
		<?=$this->pg->cancel($_SESSION['root_admin'].'setup_misc/employees.php');?>

		<script type="text/javascript">
			function verify(frm)
			{
				var foc;
				if (frm.elements['info[fname]'].value == '') { alert('You must enter a first name.'); foc = frm.elements['info[fname]']; }
				else if (frm.elements['info[lname]'].value == '') { alert('You must enter a last name.'); foc = frm.elements['info[lname]']; }
				else if (frm.elements['info[address]'].value == '') { alert('You must enter an address.'); foc = frm.elements['info[address]']; }
				else if (frm.elements['info[city]'].value == '') { alert('You must enter a city.'); foc = frm.elements['info[city]']; }
				else if (frm.elements['info[state]'].options[frm.elements['info[state]'].selectedIndex].value == '') { alert('You must select a state.'); foc = frm.elements['info[state]']; }
				else if (frm.elements['info[zip]'].value == '') { alert('You must enter a zip.'); foc = frm.elements['info[zip]']; }
				else if (frm.elements['info[phone]'].value == '') { alert('You must enter a phone number.'); foc = frm.elements['info[phone]']; }

				if (foc) { foc.focus(); return false; }
				else { return true; }
			}
		</script>
		<?php

		$this->pg->addOnload("document.employee.elements['info[fname]'].focus()");
	}

	/**
	* Pulls the information from $_POST
	*/
	function pull_post()
	{
		$this->employeeID = $_POST['employeeID'];
		$this->info = $_POST['info'];
		while (list($key,$val) = each($this->info)) { $this->info[$key] = stripslashes($val); }
		reset($this->info);
	}

	/**
	* Adds/edits the employee stored in $this->info
	*/
	function add()
	{
		global $db;

		if ($this->validate())
		{
			// add the employee to the database

			if ($this->employeeID) { $sql = "UPDATE employees SET "; }
			else { $sql = "INSERT INTO employees SET "; }

			$val = new validate();
			$this->info['zip'] = $val->strip($this->info['zip']);
			$this->info['phone'] = $val->strip($this->info['phone']);

			$vals = array();
			while (list($key,$val) = each($this->info))
			{
				$vals[] = "emp_".$key."='".mysql_escape_string($val)."'";
			}
			$sql .= implode(",",$vals);

			if ($this->employeeID) { $sql .= " WHERE emp_employeeID=$this->employeeID"; }

			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$employeeID = mysql_insert_id();

			$this->status[0] = ADDED;
		}
	}

	/**
	* Re/de-activates the employee stored in $this->employeeID (sets to inactive - doesn't delete)
	* @param integer $active If YES, deactivate the employee. If no, reactivate the employee. [optional, default YES]
	*/
	function activate($active=YES)
	{
		global $db;

		if ($this->employeeID)
		{
			// re/de-activate the employee
			$sql = "UPDATE employees SET emp_active=".($active==YES?NO:YES)." WHERE emp_employeeID=$this->employeeID";

			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$this->status[0] = DELETED;
		}
	}

	/**
	* Permanently deletes the employee stored in $this->employeeID and all traces of it (set employeeID=0 where needed)
	*/
	function permdelete()
	{
		global $db;

		if ($this->employeeID)
		{
			$sql = "DELETE FROM employees WHERE emp_employeeID=$this->employeeID";

			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$this->status[0] = DELETED;
		}
	}

  /**
  * Check the status of the employee addition/edit
  * @return integer YES/NO
  */
  function was_added()
  {
    if ($this->status[0] == ADDED || $this->status[0] == EDITED) { return YES; }
    else { return NO; }
  }

  /**
  * Check the status of the employee re/de-activation/deletion
  * @return integer YES/NO
  */
  function was_deleted()
  {
    if ($this->status[0] == DELETED) { return YES; }
    else { return NO; }
  }

  /**
  * Validates information in $this->info (phone numbers, zip codes, etc) before $this->add() can be completed
  * @return integer YES/NO
  */
  function validate()
  {
    $val = new validate();

    $this->status = array();

    if (!$val->zip_code($this->info['zip'])) { $this->status[] = BADZIP; }
    if (!$val->phone($this->info['phone'])) { $this->status[] = BADPHONE; }

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
      if ($this->status[$i] == BADZIP) { $error = "Invalid zip code."; }
      elseif ($this->status[$i] == BADPHONE) { $error = "Invalid phone number."; }

      $errors[] = $error;
    }

    if ($this->employeeID) { $word = "Edit"; } else { $word = "Add"; }
    $this->pg->error($errors);
    echo "<p />Please correct the error".(count($errors)==1?'':'s')." before pressing <b>$word Employee</b> again.<p />";
  }
}

?>