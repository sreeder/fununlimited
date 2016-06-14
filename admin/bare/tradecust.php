<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$error = new error('Inventory Movement');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);

$pg = new admin_page();
$pg->setFull(NO);
$pg->setTitle('Trade Sheet Customer Entry');
$pg->head('Trade Sheet Customer Entry');

?>
<input type="button" value="&lt; Return to Barebones Utilities" onclick="document.location='index.php'" class="btn" />
<input type="button" value="Open Customer List &gt;" onclick="customer_list()" class="btn" />
<p />
<?php

$cust = new customers($pg);

if ($act == "add")
{
	// add the customer(s)
	$info = $_POST['info'];
	$added = array();
	$not_added = array();
	$repopulateinfo = array();

	while (list($a,$arr) = each($info))
	{
		if (strlen($arr['fname']) && strlen($arr['lname']) && strlen($arr['phone']))
		{
			$_POST['info'] = $arr;
			$cust->pull_post();

			$customerID = $cust->add();

			if ($cust->was_added())
			{
				$added[] = "<b>{$cust->info['lname']}, {$cust->info['fname']}</b>";
			}
			else
			{
		    $errors = array();
		    for ($i=0; $i<count($cust->status); $i++)
		    {
		      if ($cust->status[$i] == DUPLICATE) { $errors[] = "A customer with that name/phone number already exists"; }
		      elseif ($cust->status[$i] == BADZIP) { $errors[] = "Invalid zip code"; }
		      elseif ($cust->status[$i] == BADPHONE) { $errors[] = "Invalid phone number"; }
		      elseif ($cust->status[$i] == BADEMAIL) { $errors[] = "Invalid email address"; }
		    }

				$not_added[] = array("<b>{$cust->info['lname']}, {$cust->info['fname']}</b>",implode('; ',$errors));

				$repopulateinfo[] = $arr;
			}
		}
	}

	$pg->outlineTableHead();
	?>
	<tr><td align="center" bgcolor="<?=$pg->color('table-head');?>"><b>Customer(s) Added</b></td></tr>
	<tr>
		<td bgcolor="<?=$pg->color('table-cell');?>"><?php
			if (count($added)) { echo implode('<br />',$added); }
			else { echo '<center>- None -</center>'; }
		?></td>
	</tr>
	<?php
	$pg->outlineTableFoot();

	?><p /><?php

	$pg->outlineTableHead((count($not_added)?500:''));
	?>
	<tr><td colspan="2" align="center" bgcolor="<?=$pg->color('table-head');?>"><b>Customer(s) Not Added/Reason</b></td></tr>
	<?php
	if (count($not_added))
	{
		while (list($a,list($name,$errors)) = each($not_added))
		{
			?>
			<tr>
				<td bgcolor="<?=$pg->color('table-cell');?>"><?=$name;?></td>
				<td width="100%" bgcolor="<?=$pg->color('table-cell2');?>"><?=$errors?></td>
			</tr>
			<?php
		}
	}
	else { ?><tr><td colspan="2" align="center" bgcolor="<?=$pg->color('table-cell');?>">- None -</td></tr><?php }
	$pg->outlineTableFoot();

	?><p /><?php

	form($repopulateinfo);
}
else { form(); }

// show the add customer form
function form($repopulateinfo=array())
{
	global $cus,$pg;

	$perpage = 25; // number of customer entry fields to show

	$st = new states();
	$states = array();
	while (list($abb,$name) = each($st->states)) { $states[$name] = $abb; }

	$fields = array(
		'lname'=>array('Last Name','lname',20),
		'fname'=>array('First Name','fname',20),
		'address'=>array('Address','address',20),
		'city'=>array('City','city',20),
		'state'=>array('State','state',1,$states),
		'zip'=>array('Zip Code','zip',5),
		'phone'=>array('Phone','phone',15),
		'dob'=>array('DOB','dob',8),
		'height'=>array('Height','height',5),
		'weight'=>array('Weight','weight',4),
		'gender'=>array('Gender','gender',1,array(''=>'','Male'=>MALE,'Female'=>FEMALE)),
		'ethnicity'=>array('Ethnicity','ethnicity',5),
		'email'=>array('Email','email',15),
		'idnumber'=>array('ID #','idnumber',15),
		'idexpiration'=>array('ID Expiration','idexpiration',7)
	);

	?>
	<script type="text/javascript">
		function customer_list()
		{
			open_window('/admin/reports/customer_list.php?popup=<?=YES;?>','custlist',725,500,'YES',false);
		}
		function clear_row(num)
		{
			var frm = document.getElementById('custfrm');
			var cleared = false;
			for (var i=0; i<frm.elements.length; i++)
			{
				if (frm.elements[i].name.indexOf('['+num+']') > -1)
				{
					if (frm.elements[i].type == 'text') { frm.elements[i].value = ''; }
					else if (frm.elements[i].type == 'select-one') { frm.elements[i].selectedIndex = 0; }

					cleared = true;
				}
				else if (cleared) { break; } // done clearing that line; stop execution
			}
		}
	</script>

	<b>Note:</b> Only the last name, first name, and phone number are required.
	<p />
	<?php
	$pg->outlineTableHead();
	?>
	<form method="post" action="/admin/bare/tradecust.php" id="custfrm">
	<input type="hidden" name="act" value="add" />
	<tr>
		<td align="right" bgcolor="<?=$pg->color('table-head');?>"><b>#</b></td>
		<td bgcolor="<?=$pg->color('table-head');?>"><b>Clear</b></td>
		<?php
		while (list($a,$arr) = each($fields))
		{
			?><td bgcolor="<?=$pg->color('table-head');?>"><b><?=$arr[0];?></b></td><?php
		}
		reset($fields);
		?>
	</tr>
	<?php
	for ($i=0; $i<$perpage; $i++)
	{
		$bg = (($i%2)?$pg->color('table-cell'):$pg->color('table-cell2'));

		$info = @$repopulateinfo[$i];

		?>
		<tr>
			<td align="right" bgcolor="<?=$bg;?>"><b><?=($i+1);?></b></td>
			<td bgcolor="<?=$bg;?>"><input type="button" value="Clear Row &gt;" onclick="clear_row(<?=$i;?>)" class="btn" /></td>
			<?php
			while (list($a,$arr) = each($fields))
			{
				$title = $arr[0];
				$field = $arr[1];
				$size = $arr[2];
				$vals = @$arr[3];
				$value = @$info[$field];

				if (!count($vals))
				{
					?>
					<td align="center" bgcolor="<?=$bg;?>"><input type="text" name="info[<?=$i;?>][<?=$field;?>]" size="<?=$size;?>" value="<?=$value;?>" /></td>
					<?php
				}
				else
				{
					?>
					<td align="center" bgcolor="<?=$bg;?>"><select name="info[<?=$i;?>][<?=$field;?>]" size="1"><?php
						while (list($show,$v) = each($vals))
						{
							if ($v == $value) { $s = ' selected="selected"'; } else { $s = ''; }
							?><option value="<?=$v;?>"<?=$s;?>><?=$show;?><?php
						}
					?></td>
					<?php
				}
			}
			reset($fields);
			?>
		</tr>
		<?php
	}
	$pg->outlineTableFoot();

	?>
	<p />
	<input type="submit" value="Add Trade Sheet Customers &gt;" class="btn" />
	</form>
	<?php
}

$pg->foot();
?>