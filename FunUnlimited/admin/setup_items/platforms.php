<?php
include('../../include/include.inc');

$popup = (isset($_GET['popup'])?$_GET['popup']:@$_POST['popup']);
$parentform = (isset($_GET['parentform'])?$_GET['parentform']:@$_POST['parentform']);

$pg = new admin_page();
$pg->setFull(($popup ? NO : YES));
$pg->setTitle('Manage Platforms');
$pg->head('Manage Platforms');

$act = @$_POST['act'];
$item = @$_POST['item'];
if (isset($_GET['item']) && $_GET['item'] != 'platforms') { $act = (!isset($_GET['act'])?'load':$_GET['act']); $item = $_GET['item']; }
$selID = @$_POST['selID'];
$platformID = (isset($_GET['platformID'])?$_GET['platformID']:@$_POST['platformID']);

$pla = new platforms($pg,$platformID,$item);
$pla->popup = $popup;
$pla->parentform = $parentform;

if ($popup == YES) { $cbp = 'javascript:window.close()'; } else { $cbp = "platforms.php?item=$item&platformID=$platformID"; }

if ($act == "")
{
	// select the platform
	$pla->set_item('platforms');
	$pla->show_values(YES);
}
elseif ($act == "select")
{
	$pla->platformID = $selID;
	$pla->show_menu(NO,YES);
}
elseif ($act == "load")
{
	$pla->show_values();
	$pla->show_menu();
}
elseif ($act == "add" || $act == "edit")
{
	// display the add/edit form
	if ($act == "edit" && $selID) { $pla->set_selID($selID); }

	$pla->add_form();
	$pg->cancel($cbp);
}
elseif ($act == "doadd" || $act == "doedit")
{
	// complete the add/edit
	$pla->pull_post();

	$modID = $pla->add();

	if ($pla->was_added())
	{
		if ($popup == YES)
		{
			?>
			<script type="text/javascript">
				window.opener.document.<?=$parentform;?>.refresh.value = <?=YES;?>;
				window.opener.document.<?=$parentform;?>.refreshto.value = '<?=$item;?>';
				window.opener.document.<?=$parentform;?>.submit();
				window.close();
			</script>
			<?php
		}
		else
		{
			$pla->set_item($item);
			$pla->show_values(($pla->item=='platforms'?YES:NO),$modID);
			$pla->show_menu();
		}
	}
	else
	{
		$pla->add_form();
		$pg->cancel($cbp);
	}
}
elseif ($act == "delete")
{
	$pla->set_item($item);
	$pla->set_selID($selID);

	// display the re-assignment form if needed
	$reassign = NO;
	if ($item != 'features' && $item != 'sources' && $item != 'fields')
	{
		// if it's a type, company, and platform, see if there are any
		// associated values (if so, display the re-assignment form)
		if ($pla->affected()) { $reassign = YES; }
	}

	if (!$reassign || $item == 'features' || $item == 'sources' || $item == 'fields')
	{
		// no need to reassign features/sources/fields - just delete them
		?>
		Deleting, please hold...
		<p />
		If you are not automatically forwarded, <a href="javascript:document.dd.submit()">click here</a>.

		<form method="post" action="platforms.php" name="dd">
			<input type="hidden" name="act" value="dodelete">
			<input type="hidden" name="platformID" value="<?=$platformID;?>">
			<input type="hidden" name="item" value="<?=$item;?>">
			<input type="hidden" name="deleteID" value="<?=$selID;?>">
		</form>
		<?php
		$pg->addOnload('document.dd.submit()');
	}
	else
	{
		// types, companies, and platforms need to be reassigned
		$pla->set_item($item);
		$pla->set_selID($selID);
		$pla->show_values(NO,0,YES);
		$pg->cancel($cbp);
	}
}
elseif ($act == "dodelete")
{
	// complete the deletion/re-assignment
	$deleteID = $_POST['deleteID'];
	$pla->set_selID($deleteID);
	$pla->delete($selID);
	$pla->set_item($item);

	$pla->show_values(($item=='platforms'?YES:NO));
	$pla->show_menu();
}

$pg->foot();
?>