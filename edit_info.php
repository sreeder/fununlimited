<?php
/*
* Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
$check_login = true;
include('include/include.inc');

$invalid = getG('invalid');
$action = getG('action');

$pg = new page();
$pg->setTitle('Edit Account Information');
$pg->head('Edit Account Information');

$form = BOTH;
if ($invalid)
{
	if ($action == 'info')
	{
		echo 'Please fix the following errors before attempting to update your info again:<p />';
		$pg->error(@$_SESSION['register_errors']);
		$form = 'info';
	}
	elseif ($action == 'password')
	{
		echo 'Please fix the following error before attempting to change your password again:<p />';
		$pg->error(@$_SESSION['register_errors']);
		$form = 'password';
	}
	else { $invalid = NO; }
}

if ($form == BOTH || $form == 'info')
{
	// output the registration information edit form
	?>
	Update your information and press <b>Save Information</b>:
	<p />
	<?php
	if ($invalid)
	{
		$reginfo = $_SESSION['register_info'];
	}
	else
	{
		$reginfo = $_SESSION['userinfo'];
		fixInfoArray($reginfo);
	}
	$form_action = 'edit';
	include('form_info.php');
}

if ($form == BOTH)
{
	?>
	<p />
	<hr width="75%" size="1" color="#000000" />
	<p />
	<?php
}

if ($form == BOTH || $form == 'password')
{
	// output the password change form
	?>
	Need to change your password? Enter your current password<br />
	and a new password and press <b>Save Password</b>:
	<p />
	<?=$pg->outlineTableHead();?>
		<form method="post" action="/accountUpdate.php" name="passForm">
		<input type="hidden" name="act" value="set_password" />
		<tr>
			<td class="tbl_label">Current Password:</td>
			<td><input type="password" name="currentpass" size="12" /></td>
		</tr>
		<tr>
			<td class="tbl_label">New Password:</td>
			<td><input type="password" name="newpass" size="12" /></td>
		</tr>
		<tr>
			<td class="tbl_label">New Password (again):</td>
			<td><input type="password" name="newpass_verify" size="12" /></td>
		</tr>
	<?=$pg->outlineTableFoot();?>
	<p />
	<input type="submit" value="Save Password &gt;" class="btn" />
	</form>
	<?php
}

$pg->foot();
?>