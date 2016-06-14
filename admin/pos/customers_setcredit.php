<?php
include('../../include/include.inc');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$credit = @$_GET['credit'];

$pg = new admin_page();
$pg->setFull(NO);
$pg->head();

$error = new error('Customer Credit Set');

if (strlen($credit))
{
	if (!is_numeric($credit)) { $alert = 'Please enter a number!'; }
	else
	{
		$credit = sprintf('%0.2f',$credit);
		$sql = "UPDATE customers SET cus_creditamount=$credit WHERE cus_customerID={$_SESSION['customerID']}";
		mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);

		$alert = "Set customer credit to \$".number_format($credit,2);
		?>
		<script type="text/javascript">parent.change_credit(<?=$credit;?>);</script>
		<?php
	}

	?>
	<script type="text/javascript">
		alert('<?=$alert;?>');
		parent.orig_credit = <?=$credit;?>;
	</script>
	<?php
}

$pg->foot();
?>