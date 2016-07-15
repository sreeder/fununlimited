<?php
include('../../include/include.inc');

$user = @$_POST['user'];
$pass = @$_POST['pass'];

if (md5($user) == '25b50d8a2b0d5858e8052a520a54cefa' && md5($pass) == 'e4db02ddaea7892e931008d726747775')
{
	$_SESSION['scottc_loggedin'] = YES;
	?>
	Scott Carpenter has been logged in.
	<?php
	if (strlen(@$_SERVER['HTTP_REFERER']))
	{
		?>
		<p />
		Click <a href="<?=$_SERVER['HTTP_REFERER'];?>">here</a> to return to where you came from (<?=$_SERVER['HTTP_REFERER'];?>)
		<?php
	}
}
else
{
	?>
	Invalid username and password. Please try again.
	<p />
	<?php
	check_scottc_loggedin(NO);
}

?>
