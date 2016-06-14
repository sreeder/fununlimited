<?php
include('../include/include.inc');

$pg = new admin_page();
$pg->setTitle('Site Development Progress');
$pg->head('Site Development Progress');

$todo = array();

?><b>October-November</b><p /><?php

/*
add(NO,'Add all possible criteria to item search (w/repost to change fields when platform selected [features, types, etc])');
add(NO,'If adding an item and UPC is duplicate, link to info page for duplicate item');
add(YES,'When completing invoice, if there are trade items, check/prompt for any missing required info');
add(NO,"On invoice item add, show 'mini' results for multiple matches");
add(NO,'Change item search to one-function-per-criterion format');
*/
add(NO,'Customer management (add(YES)/edit(YES)/remove(NO)/search(50%)/etc)');
add(NO,'Finalize Invoice System');
add(NO,'Online store');

// WHEN SEARCHING, IF ANY ARE MISSING COMPANY/YEAR, ALERT WITH LINKS TO ITEM EDIT PAGE

$pg->outlineTableHead();
?>
<tr>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		<?php
		while (list($a,list($yn,$td)) = each($todo))
		{
			?><img src="/images/<?=($yn==YES?'check.gif':'x.gif');?>" style="vertical-align:middle"> <?=$td;?><br /><?php
		}
		?>
	</td>
</tr>
<?php
$pg->outlineTableFoot();

?><p /><?php

$pg->outlineTableHead();
?>
<tr>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		Is there anything you would like to have added to this list? Any problems with the site? Suggestions? Tell me!<br />
		Try my cell phone first; if I don't answer, call my house...
		<p />
		<center>
			<table border="0">
				<tr>
					<td>
						<b>Scott Carpenter</b><br />
						<b>Cell:</b> 435.512.9323<br />
						<b>Home:</b> 435.752.3047<br />
						<b>E-Mail:</b> <a href="mailto:s-carp@comcast.net">s-carp@comcast.net</a>
					</td>
				</tr>
			</table>
		</center>
	</td>
</tr>
<?php
$pg->outlineTableFoot();

?><p /><hr width="100%" size="-1" color="#CCCCCC" /><p /><?php

$totalchecks = 34;
$checkamt = 250;
$bidamt = 8750;

$pg->outlineTableHead();
?>
<tr>
	<td bgcolor="<?=$pg->color('table-cell');?>">
		<b>Total Paid:</b> <?=$totalchecks;?> checks x $<?=$checkamt;?> = $<?=($totalchecks*$checkamt);?><br />
		<b>Current Bid:</b> $<?=$bidamt;?><br />
		<b>Balance:</b> $<?=($bidamt-($totalchecks*$checkamt));?>
		<p />
		<center>
			<table border="0" cellspacing="3" cellpadding="0">
				<tr><td colspan="3" align="center"><b><u>Bid Breakdown</u></b></td></tr>
				<tr>
					<td>
						<!-- descriptions -->
						Inventory System<br />
						Online Store<br />
						Point-of-Sale<br />
						Reporting/Year-End Functions<br />
						Pay For Time In Store
					</td>
					<td>&nbsp;</td>
					<td align="right">
						<!-- amounts -->
						$3500<br />
						$1750<br />
						$2250<br />
						$750<br />
						$500+
					</td>
				</tr>
				<tr><td colspan="3" bgcolor="#000000"></td></tr>
				<tr><td colspan="3" align="right"><b>$<?=$bidamt;?></b></td></tr>
			</table>
		</center>
	</td>
</tr>
<?php
$pg->outlineTableFoot();

?>
<p />
<font size="1">
	<b>Please note:</b> the above amounts may go up or down depending<br />
	on any additions/subtractions made to/from the site and it's content.
</font>
<?php

function add($yn,$td) { global $todo; $todo[] = array($yn,$td); }

$pg->foot();

/*

5-30-03 to 6-6-03
add(YES,'Ask for verification when deleting an item');
add(YES,'Option to search active or inactive items');
add(YES,'show_results function (in place of hard-coded search results)');
add(YES,'Multiple pages on search results (w/selection on search page for qty-per-page)');
add(YES,'If searching inactive items, button to reactivate the item');
add(YES,'Dynamic field values on setup page (platform-dependant)');
add(YES,'Dynamic fields on item info page');
add(YES,'Links to return to the search results (cancel buttons)');

6-7-03 to 6-13-03
add(YES,'Image upload/display on item add/edit/results screens');
add(YES,'Track <b>fromselect</b> var if errors during image upload');
add(YES,'When deleting values in setup, if no items/values will be affected, delete, don\'t reassign');
add(YES,'Fix letters on price/rating sources page (&lt;a name="letter"&gt; defined twice for some letters)');
add(YES,'"Add New Form Field" link on item add screen');

September
add(YES,'Employees');
add(YES,'Cash/Credit percentages');
add(YES,'Customer database');
add(YES,'Invoice database');
add(YES,'Sales/trades (new/history/add items/etc)');

*/
?>