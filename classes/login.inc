<?php
/**
* Handles user login/logout and form display for both administration and the online store
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class login
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
	* Locale of the login (INSTORE/ONLINE)
	* @var integer $locale
	*/
	var $locale;

	/**
	* User information (set after valid login)
	* @var array $userinfo
	*/
	var $userinfo;

	/**
	* Class constructor
	* Initializes variables
	* @param integer $locale
	*/
	function login($locale="INSTORE")
	{
		$this->pg =& $GLOBALS['pg'];
		$this->_setLocale($locale);
		$this->error = new error('Login');
	}

	/**
	* Set the locale
	* @param integer $locale
	* @access private
	*/
	function _setLocale($locale) { $this->locale = $locale; }

	/**
	* Return the locale value
	* @return integer
	* @access private
	*/
	function _getLocale() { return $this->locale; }

	/**
	* Displays the login form
	* @param integer $storeID ID of store to select [optional, default '']
	* @param string $user Username to put in the username box [optional, default '']
	* @param string $return Page to redirect to after login [optional, default '']
	*/
	function form($storeID=-1,$user='',$return='')
	{
		$locale = $this->_getLocale();

		if ($locale == "INSTORE")
		{
			// administration login
			if ($return == '/index.php') { $return = ''; }
			if ($storeID == -1 && isset($_SESSION['storeID'])) { $storeID = $_SESSION['storeID']; }

			$sto = new stores($pg);
			$sto->get_stores(YES);

			if (!count($sto->stores))
			{
				$this->error->show('No store information - database incomplete! Please contact the administrator!');
			}

			?>
			<script type="text/javascript">
				function verify(frm)
				{
					errfound = false;

					if (!frm.user.value.length) { err('You must enter a username',frm.user); }
					else if (!frm.pass.value.length) { err('You must enter a password',frm.pass); }

					return !errfound;
				}
			</script>

			<?=$this->pg->outlineTableHead();?>
			<form method="post" action="/admin/login.php" name="login" onsubmit="return verify(this)">
			<input type="hidden" name="return" value="<?=$return;?>">
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Store:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>"><select name="storeID" size="1"><?php
					$sto->stores = array_merge(array(array('sto_storeID'=>0,'sto_name'=>'')),$sto->stores);
					while (list($a,$arr) = each($sto->stores))
					{
						if ($arr['sto_storeID'] == $storeID) { $s = ' selected="selected"'; } else { $s = ""; }
						?><option value="<?=$arr['sto_storeID'];?>"<?=$s;?>><?=$arr['sto_name'];?></option><?php
					}
				?></select></td>
			</tr>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Username:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="text" name="user" size="10" value="<?=$user;?>"></td>
			</tr>
			<tr>
				<td bgcolor="<?=$this->pg->color('table-label');?>"><b>Password:</b></td>
				<td bgcolor="<?=$this->pg->color('table-cell');?>"><input type="password" name="pass" size="10"></td>
			</tr>
			<?=$this->pg->outlineTableFoot();?>
			<p />
			<input type="submit" value="Log In &gt;" class="btn">
			</form>
			<?php

			if (strlen($user)) { $this->pg->addOnload('document.login.pass.focus()'); }
			elseif ($storeID != -1) { $this->pg->addOnload('document.login.user.focus()'); }
			else { $this->pg->addOnload('document.login.storeID.focus()'); }
		}
		else
		{
			// online store login
			?>
			<script type="text/javascript">
				function verify(frm)
				{
					errfound = false;

					if (!frm.user.value.length) { err('You must enter a username',frm.user); }
					else if (!frm.pass.value.length) { err('You must enter a password',frm.pass); }

					return !errfound;
				}
			</script>

			Enter your username and password below to login.
			<p />
			Not yet a customer?<br />
			Enter your information to register for an account &rarr;
			<p />
			<?=$this->pg->outlineTableHead();?>
			<form method="post" action="/loginUpdate.php" name="login" onsubmit="return verify(this)">
			<input type="hidden" name="act" value="login">
			<input type="hidden" name="return" value="<?=$return;?>">
			<tr>
				<td class="tbl_label"><b>Username:</b></td>
				<td><input type="text" name="user" id="log_user" size="10" value="<?=$user;?>"></td>
			</tr>
			<tr>
				<td class="tbl_label"><b>Password:</b></td>
				<td><input type="password" name="pass" id="log_pass" size="10"></td>
			</tr>
			<?=$this->pg->outlineTableFoot();?>
			<p />
			<input type="submit" value="Log In &gt;" class="btn">
			</form>
			<?php

			//if (strlen($user)) { $this->pg->addOnload('document.login.pass.focus()'); }
			//elseif (strlen($storeID)) { $this->pg->addOnload('document.login.user.focus()'); }
			//else { $this->pg->addOnload('document.login.storeID.focus()'); }
		}
	}

	/**
	* Validate a storeID/user/pass
	* @param integer $storeID
	* @param string $user
	* @param string $pass
	* @param integer $customerID if passed, user/pass is ignored and this is used [optional, default 0]
	* @return boolean
	*/
	function checkLogin($storeID,$user,$pass,$customerID=0)
	{
		global $db;

		$locale = $this->_getLocale();
		if ($locale == "INSTORE")
		{
			$sql = "SELECT * FROM users WHERE use_storeID=$storeID AND use_user='$user' AND use_pass=ENCODE('$pass','".ENCODEKEY."')";
		}
		else
		{
			if ($customerID) { $where = "cus_customerID=$customerID"; }
			else { $where = "cus_user='$user' AND cus_pass=ENCODE('$pass','" . ENCODEKEY . "')"; }
			$sql = "SELECT * FROM customers WHERE $where";
		}
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		if (mysql_num_rows($result))
		{
			// it's a valid login - log the user in
			$row = mysql_fetch_assoc($result);
			$this->userinfo = $row;

			if ($locale == "INSTORE")
			{
				$_SESSION['store_userinfo'] = $row;

				// load the store information
				$sql = "SELECT * FROM stores WHERE sto_storeID=$storeID";
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
				if (mysql_num_rows($result))
				{
					$row = mysql_fetch_assoc($result);
					$_SESSION['store_info'] = $row;
				}
				else { $_SESSION['store_info'] = array(); }
			}
			else
			{
				$_SESSION['userinfo'] = $row;
			}

			return YES;
		}
		else
		{
			// it's an invalid login
			return NO;
		}
	}
}

?>