<?php
include('../../include/include.inc');
check_scottc_loggedin();

// allows Scott Carpenter to send messages to Fun Unlimited users

$message = stripslashes(trim(@$_POST['message']));

if (strlen($message))
{
	// save the message in the database
	$sql = "INSERT INTO messages_from_scott VALUES (NULL,'".mysql_real_escape_string($message)."',".time().",".NO.")";
	mysql_query($sql,$db);
	if (mysql_errno()) { echo "MySQL Error when saving message: ".mysql_error()."<p />"; }
	?>
	Message saved
	<p />
	<b>Message:</b> <?=$message;?><br />
	<p />
	<hr width="100%" size="-1" color="#000000" noshade="noshade" />
	<p />
	<?php
}

?>
<form method="post" action="scott_message.php">
	Message: <input type="text" name="message" size="100" />
	<input type="submit" value="Send Message &gt;" />
</form>

<p />
<hr width="100%" size="-1" color="#000000" noshade="noshade" />
<p />
Messages:
<p />

<table border="1" cellspacing="0" cellpadding="3">
	<tr>
		<td><b>Time</b></td>
		<td><b>Shown?</b></td>
		<td><b>Message</b></td>
	</tr>
	<?php
	$sql = "SELECT * FROM messages_from_scott ORDER BY mfs_time";
	$result = mysql_query($sql,$db);
	while ($row = mysql_fetch_assoc($result))
	{
		?>
		<tr>
			<td><?=date('m/d/Y h:ia',$row['mfs_time']);?></td>
			<td><?=($row['mfs_shown']?'YES':'No');?></td>
			<td><?=$row['mfs_message'];?></td>
		</tr>
		<?php
	}
	?>
</table>
