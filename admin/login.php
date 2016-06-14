<?php
include('../include/include.inc');

$pg = new admin_page();

if (isset($_POST['storeID']))
{
	$return = @$_POST['return'];
	$storeID = @$_POST['storeID'];
	$user = @$_POST['user'];
	$pass = @$_POST['pass'];

	$log = new login();
	$valid = $log->checkLogin($storeID,$user,$pass);

	if ($valid)
	{
		$_SESSION['loggedin'] = YES;
		$_SESSION['userID'] = $log->userinfo['use_userID'];
		$_SESSION['storeID'] = $log->userinfo['use_storeID'];
		$_SESSION['name'] = $log->userinfo['use_name'];
		$_SESSION['usertype'] = $log->userinfo['use_type'];

		head();
		?>
		You have successfully been logged in <b><?=$log->userinfo['use_name'];?></b>!
		<p />
		<?php

		if (strlen($return) && basename($return) != 'index.php')
		{
			?>
			Please hold while you are redirected...
			<p />
			Browser doesn't redirect? <a href="<?=$return;?>">Click Here...</a>

			<script type="text/javascript">setTimeout("document.location='<?=$return;?>'",1000)</script>
			<?php
		}
		else
		{
			?>Please select a function from the menu.<?php
		}
	}
	else
	{
		head();
		$pg->error('Invalid store/username/password combination. Please try again.');
		$log->form($storeID,$user,$return);
	}
}
else
{
	head();
	if (@$_SESSION['loggedin']) { ?>You are already logged in!<?php }
	else
	{
		$log = new login();
		$log->form(1);
	}
}

function head()
{
	global $pg;

	$pg->setTitle('Log In');
	$pg->head('Log In');
}

$pg->foot();
?>