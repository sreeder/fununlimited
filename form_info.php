<?php
/*
* Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
if (!isset($pg)) { die('INVALID CALL - FILE NOT INCLUDED CORRECTLY'); }

$form_buttons = array(
	'register' => 'Register &gt;',
	'edit'     => 'Save Information &gt;',
);
$form_actions = array(
	'register' => '/registerUpdate.php',
	'edit'     => '/accountUpdate.php',
);
if (!isset($form_action) || !in_array($form_action,array_keys($form_buttons))) { $form_action = 'register'; }
if (!isset($return)) { $return = ''; }

?>
<?=$pg->outlineTableHead();?>
	<form method="post" action="<?=$form_actions[$form_action];?>" name="register">
	<input type="hidden" name="act" value="<?=$form_action;?>" />
	<input type="hidden" name="return" value="<?=htmlspecialchars($return);?>" />
	<tr>
		<td class="tbl_label">First Name:</td>
		<td><input type="text" name="info[fname]" id="reg_first" size="25" value="<?=@$reginfo['fname'];?>" /></td>
	</tr>
	<tr>
		<td class="tbl_label">Last Name:</td>
		<td><input type="text" name="info[lname]" size="30" value="<?=@$reginfo['lname'];?>" /></td>
	</tr>
	<tr>
		<td class="tbl_label">Address:</td>
		<td><input type="text" name="info[address]" size="40" value="<?=@$reginfo['address'];?>" /></td>
	</tr>
	<tr>
		<td class="tbl_label">City:</td>
		<td>
			<input type="text" name="info[city]" size="20" value="<?=@$reginfo['city'];?>" />
		</td>
	</tr>
	<tr>
		<td class="tbl_label">State:</td>
		<td>
			<select name="info[state]" size="1"><?php
				$st = new states();
				while (list($abb,$name) = each($st->states))
				{
					if ($abb == @$reginfo['state']) { $s = ' selected="selected"'; } else { $s = ''; }
					?><option value="<?=$abb;?>"<?=$s;?>><?=$name;?></option><?php
				}
			?></select>
		</td>
	</tr>
	<tr>
		<td class="tbl_label">Zip Code:</td>
		<td>
			<input type="text" name="info[zip]" size="12" maxlength="10" value="<?=$pg->format('zip',validate::strip(@$reginfo['zip']));?>" />
		</td>
	</tr>
	<tr>
		<td class="tbl_label">Phone Number:</td>
		<td><input type="text" name="info[phone]" size="16" value="<?=$pg->format('phone',validate::strip(@$reginfo['phone']));?>" /></td>
	</tr>
	<tr>
		<td class="tbl_label">Cell Phone Number:</td>
		<td><input type="text" name="info[cellphone]" size="16" value="<?=$pg->format('phone',validate::strip(@$reginfo['cellphone']));?>" /></td>
	</tr>
	<tr>
		<td class="tbl_label">E-Mail:</td>
		<td><input type="text" name="info[email]" size="40" value="<?=@$reginfo['email'];?>" /></td>
	</tr>
	<?php
	if ($form_action == 'register')
	{
		?>
		<tr>
			<td class="tbl_label">Username:</td>
			<td><input type="text" name="info[user]" size="10" value="<?=@$reginfo['user'];?>" /></td>
		</tr>
		<tr>
			<td class="tbl_label">Password:</td>
			<td><input type="password" name="info[pass]" size="10" /></td>
		</tr>
		<tr>
			<td class="tbl_label">Password (again):</td>
			<td><input type="password" name="info[pass_verify]" size="10" /></td>
		</tr>
		<?php
	}
	elseif ($form_action == 'edit')
	{
		?>
		<tr>
			<td class="tbl_label">Username:</td>
			<td>
				<input type="hidden" name="info[user]" value="<?=@$reginfo['user'];?>" />
				<?=@$reginfo['user'];?>
			</td>
		</tr>
		<?php
	}
	?>
<?=$pg->outlineTableFoot();?>
<p />
<?php
if ($form_action == 'edit')
{
	?>
	<input type="reset" value="Reset Form" class="btn" />
	<?php
}
?>
<input type="submit" value="<?=$form_buttons[$form_action];?>" class="btn" />
</form>
