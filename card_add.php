<?php
/*
* Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
*/
$check_login = true;
include('include/include.inc');

$pg = new page();
$cc = new credit_card($pg);
$cc->setTypes();
$types = $cc->getTypes();

if (getG('error_code'))
{
	// pull the info from the POSTed data
	$cardID = $_SESSION['cc_post_info']['cardID'];
	$info = $_SESSION['cc_post_info'];
	$newinfo = array();
	while (list($k,$v) = each($info)) { $newinfo["crc_$k"] = $v; }
	$info = $newinfo;
	$word = ($cardID ? 'Edit' : 'Add');
}
else
{
	$cardID = getG('cardID');
	if ($cardID)
	{
		// get the credit cards
		$cc->setCards($cardID);
		$info = $cc->getCards($cardID);

		if (!count($info)) { $cardID = 0; }
	}
	$word = ($cardID ? 'Edit' : 'Add');
}

$pg->setTitle("$word Credit/Debit Card");
$pg->head('Manage Payment Methods');

?>
<form method="post" action="creditcardUpdate.php" id="ccfrm">
<input type="hidden" name="act" value="add" />
<input type="hidden" name="cardID" value="<?=$cardID;?>" />
<table border="0" cellspacing="1" cellpadding="3" width="95%">
	<tr>
		<td colspan="2">
			<span class="orange_label">&raquo; <?=$word;?> Credit/Debit Card</span>
			<p />
			<?php
			// if applicable, show an error
			if (getG('error_code')) { $cc->showError(getG('error_code')); }
			?>
			Please enter the following information exactly as it appears on your billing statement:<br />
			&nbsp;
		</td>
	</tr>
	<tr>
		<td class="tbl_label">First/Last Name:</td>
		<td>
			<input type="text" name="fname" size="20" value="<?=@$info['crc_fname'];?>" />
			<input type="text" name="lname" size="30" value="<?=@$info['crc_lname'];?>" />
		</td>
	</tr>
	<tr>
		<td class="tbl_label">Company Name:</td>
		<td><input type="text" name="companyname" size="35" value="<?=@$info['crc_companyname'];?>" /></td>
	</tr>
	<tr>
		<td class="tbl_label">Card Number:</td>
		<td><input type="text" name="number" size="25" value="<?=@$info['crc_number'];?>" /></td>
	</tr>
	<tr>
		<td class="tbl_label">Card Type:</td>
		<td>
			<select name="typeID" size="1">
				<?php
				while (list($typeID,$arr) = each($types))
				{
					$s = ($typeID==@$info['crc_typeID'] ? ' selected="selected"' : '');
					?><option value="<?=$typeID;?>"<?=$s;?>><?=$arr['cct_name'];?></option><?php
				}
				?>
			</select>
		</td>
	</tr>
	<tr>
		<td class="tbl_label">Expiration Date:</td>
		<td>
	    <select name="expmonth" size="1">
	    	<?php
	      $months = array('January','February','March','April','May','June','July','August','September','October','November','December');
	      for ($i=0; $i<count($months); $i++)
	      {
	        $s = (($i+1)==@$info['crc_expmonth'] ? ' selected="selected"' : '');
	        ?><option value="<?=($i+1);?>"<?=$s;?>><?=str_pad(($i+1),2,0,STR_PAD_LEFT);?> - <?=$months[$i];?></option><?php
	      }
		    ?>
		  </select>
	    /
	    <select name="expyear" size="1">
	    	<?php
	      $start = date('Y');
	      $show = 10;

	      for ($i=$start; $i<=($start+$show); $i++)
	      {
	        $s = ($i==@$info['crc_expyear'] ? ' selected="selected"' : '');
	        ?><option value="<?=$i;?>"<?=$s;?>><?=$i;?></option><?php
	      }
	    	?>
	    </select>
	  </td>
	</tr>
	<tr>
		<td class="tbl_label">Billing Address:</td>
		<td><input type="text" name="address" size="35" value="<?=@$info['crc_address'];?>" /></td>
	</tr>
	<tr>
		<td class="tbl_label">City, State Zip:</td>
		<td>
			<input type="text" name="city" size="20" value="<?=@$info['crc_city'];?>" />
			<select name="state" size="1">
				<?php
				$st = new states();
				while (list($abb,$name) = each($st->states))
				{
					$s = ($abb==@$info['crc_state'] ? ' selected="selected"' : '');
					?><option value="<?=$abb;?>"<?=$s;?>><?=$name;?></option><?php
				}
				?>
			</select>
			<input type="text" name="zip" size="12" maxlength="10" value="<?=@$info['crc_zip'];?>" />
		</td>
	</tr>
	<tr>
		<td class="tbl_label">Billing Phone:</td>
		<td><input type="text" name="phone" size="16" value="<?=page::format('phone',@$info['crc_phone']);?>" /></td>
	</tr>
</table>
<p />
<input type="reset" value="Reset Form" class="btn" />
<input type="submit" value="<?=$word;?> Credit/Debit Card &gt;" class="btn" />

<p />
<img src="/images/blank_black.gif" width="550" height="1" />
<p />

<div class="left">
	<a href="/pay_methods.php"><img src="/images/btn_cancel.gif" width="69" height="18" border="0" alt="Cancel" /></a>
</div>
<?php

$pg->foot();
?>