<?php
include('include/include.inc');

if (@$_SESSION['store_loggedin']) { headerLocation('/catalog.php'); }

$act = getGP('act');
$invalid = getG('invalid');
$action = getG('action');
$user = getG('user');
$return = getG('return');

$pg = new page();
$pg->setShowBrowse(NO);
$log = new login(ONLINE);
$error = new error('Login');

$pg->addOnload("document.getElementById('" . ($action=='register' ? 'reg_first' : 'log_' . (strlen($user) ? 'pass' : 'user')) . "').focus()");

$show_form = NO;

if ($act == '')
{
	// output the login form
	head();
	if ($invalid)
	{
		if (!strlen($action) || $action == 'login') { $pg->error('Invalid username/password combination. Please try again.'); }
		elseif ($action == 'register')
		{
			echo 'Please fix the following errors before attempting to register again:<p />';
			$pg->error(@$_SESSION['register_errors']);
		}
	}

	$show_form = YES;
}
else
{
	head();
	if (isset($_SESSION['store_loggedin']))
	{
		// already logged in - go to the catalog
		header('Location: /catalog.php');
	}
	else { $show_form = YES; }
}

// show the form, if necessary
if ($show_form)
{
	$reginfo = @$_SESSION['register_info'];

	if (strlen($return) && !$invalid)
	{
		?>
		<b>To continue, please log in or register:</b>
		<p />
		<?php
	}

	?>
	<table border="0" cellspacing="20" cellpadding="0">
		<tr>
			<td align="center" valign="top">
				<font size="4"><b>Log In</b></font>
				<p />
				<?=$log->form(NONE,$user,$return);?>
			</td>
			<td bgcolor="#000000" width="1"><img src="/images/blank.gif" width="1" height="1" /></td>
			<td align="center" valign="top">
				<font size="4"><b>Register</b></font>
				<p />
				Enter your information below to register for an account.
				<p />
				<?php
				include('form_info.php');
				?>
			</td>
		</tr>
	</table>
	<?php
}

function head()
{
	global $pg;

	$pg->setTitle('Log In/Register');
	$pg->head('Log In/Register');
}

$pg->foot();
?>