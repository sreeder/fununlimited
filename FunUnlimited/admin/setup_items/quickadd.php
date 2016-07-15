<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$pg = new admin_page();
$pg->setTitle('Quick Item Add');
$pg->head('Quick Item Add');

$itm = new items($pg);
$error = new error('Quick Item Add');

$act = (isset($_GET['act'])?$_GET['act']:@$_POST['act']);
$numitems = (isset($_GET['numitems'])?$_GET['numitems']:@$_POST['numitems']);
if (!$numitems) { $numitems = 20; } // number of new item lines to show

if ($act == '')
{
	// display the select platform form

	if (strlen(@$_SESSION['quickadd_status']))
	{
		$pg->status($_SESSION['quickadd_status']);
		unset($_SESSION['quickadd_status']);
	}
	if (count(@$_SESSION['quickadd_errors']))
	{
		$pg->error($_SESSION['quickadd_errors'], 'Add Item');
		unset($_SESSION['quickadd_errors']);
	}

	if (getG('errors') && @$_SESSION['do_quickadd'])
	{
		?>
		<p>
			Please make note of the above errors.<br />
			The items you selected to add to an invoice will now be added...
		</p>

		<input type="button" value="Continue &gt;" onclick="go('/admin/pos/invoice.php?act=view')" class="btn" />
		<?php
	}
	else
	{
		$itm->platform_form("{$_SESSION['root_admin']}setup_items/quickadd.php?numitems=$numitems", '',@$_GET['platformID']);
		?>
		<p />
		<hr width="50%" size="-1" color="#000000" noshade="noshade" />
		<p />
		<script type="text/javascript">
		function submitNumItems()
		{
			var frm = $('numitemsfrm');
			var platformID = $('itemfrm').platformID.value;
			frm.platformID.value = platformID;
			frm.submit();
		}
		</script>

		Number of new items to add:
		<form method="get" action="quickadd.php" id="numitemsfrm">
			<input type="hidden" name="platformID" value="<?php echo @$_GET['platformID'];?>" />
			<select name="numitems" size="1" onchange="submitNumItems()"><?php
			for ($i=1; $i<=30; $i++)
			{
				if ($i == $numitems) { $s = ' selected="selected"'; } else { $s = ''; }
				?><option value="<?php echo $i;?>"<?php echo $s;?>><?php echo $i;?></option><?php
			}
			?></select>
			<input type="button" value="Change &gt;" onclick="submitNumItems()" class="btn" />
		</form>
		<?php
	}
}
elseif ($act == 'selplatform')
{
	// platform selected, show the add form
	$platformID = getGP('platformID');

	// select the 'Unknown Type' typeID
	$sql = "SELECT typ_typeID FROM types WHERE typ_platformID=$platformID AND typ_type='Unknown Type'";
	$result = mysql_query($sql, $db);
	$error->mysql(__FILE__,__LINE__);
	if (!mysql_num_rows($result))
	{
		$pg->error('Type <b>Unknown Type</b> does not exist for selected platform - please add it!');
	}
	else
	{
		// is there an open invoice?
		$open_invoice = (@$_SESSION['cust_invoiceID'] ? YES : NO);

		$row = mysql_fetch_assoc($result);
		$typeID = $row['typ_typeID'];

		$pla = new platforms($pg, $platformID);
		$pla->show_platform(YES,"{$_SESSION['root_admin']}setup_items/quickadd.php?numitems=$numitems");
		$pla->set_item('types'); // pull in the types
		$yr = new years(); // pull in the years

		// get the invoice add options
		ob_start();
		?>
		New/Used: <select name="options[%i][newused]">
			<option value="<?php echo ITEM_NEW;?>" selected="selected">New</option>
			<option value="<?php echo ITEM_USED;?>">Used</option>
		</select>
		 Box Type: <select name="options[%i][box]">
			<option value="<?php echo BOX;?>" selected="selected">Box</option>
			<option value="<?php echo NOBOX;?>">No Box</option>
			<option value="<?php echo STOREBOX;?>">Store Box</option>
		</select>
		<?php
		$options_sale = str_replace(array("\n", "\t"), '', ob_get_clean());

		ob_start();
		?>
		Price Type: <select name="options[%i][pricetype]">
			<option value="<?php echo CASH;?>">Cash</option>
			<option value="<?php echo CREDIT;?>" selected="selected">Credit</option>
			<option value="<?php echo NEITHER;?>">Neither</option>
		</select>
		 Box Type: <select name="options[%i][box]">
			<option value="<?php echo BOX;?>" selected="selected">Box</option>
			<option value="<?php echo NOBOX;?>">No Box</option>
		</select>
		 Condition: <select name="options[%i][condition]">
			<option value="<?php echo ITEM_NEW;?>">New</option>
			<option value="<?php echo GOOD;?>" selected="selected">Good</option>
			<option value="<?php echo FAIR;?>">Fair</option>
			<option value="<?php echo POOR;?>">Poor</option>
		</select>
		<?php
		$options_trade = str_replace(array("\n", "\t"), '', ob_get_clean());

		// output the add item form
		?>
		<script type="text/javascript">
			var typeIDs = new Array(<?php echo count($pla->values);?>);
			var type_text = new Array(<?php echo count($pla->values);?>);
			var yearIDs = new Array(<?php echo count($yr->years);?>);
			var year_text = new Array(<?php echo count($yr->years);?>);
			var allow_submit = false;
			var numitems = <?php echo $numitems;?>;

			<?php
			// populate the type variables
			while (list($a, $arr) = each($pla->values))
			{
				?>typeIDs[<?php echo $a;?>]=<?php echo $arr[0];?>;type_text[<?php echo $a;?>]='<?php echo mysql_real_escape_string($arr[1]);?>';<?php
			}
			?>
			<?php
			// populate the year variables
			while (list($a, $arr) = each($yr->years))
			{
				?>yearIDs[<?php echo $a;?>]=<?php echo $arr['yer_yearID'];?>;year_text[<?php echo $a;?>]='<?php echo mysql_real_escape_string($arr['yer_year']);?>';<?php
			}
			?>

			// populate all select boxes
			function populate_selects()
			{
				var tpobj,yrobj;
				var typeID,yearID,type,year;

				for (var i=0; i<numitems; i++)
				{
					tpobj = $('type'+i);
					for (var j=0; j<typeIDs.length; j++)
					{
						if (type_text[j] == 'Unknown Type') { var selIDX = j; }
						tpobj.options[j] = new Option(type_text[j],typeIDs[j]);
					}
					tpobj.selectedIndex = selIDX;

					yrobj = $('year'+i);
					for (var j=0; j<yearIDs.length; j++)
					{
						yrobj.options[j] = new Option(year_text[j],yearIDs[j]);
					}
					yrobj.selectedIndex = (yrobj.options.length-1);
				}

				$('holdlyr').style.display = 'none';
				allow_submit = true;

				setAddType(<?php echo TRADE;?>);
			}

			/**
			* Set the quick add to invoice type
			* @param integer add_type
			*/
			function setAddType(add_type)
			{
				options_sale = '<?php echo $options_sale;?>';
				options_trade = '<?php echo $options_trade;?>';

				for (var i=0; i<numitems; i++)
				{
					$('options' + i).innerHTML = (add_type==<?php echo SALE;?> ? options_sale : options_trade).replace(/%i/g, i);
				}
			}

			function verify(frm)
			{
				if (!allow_submit) { alert('Please wait until the options have been updated!'); return false; }
				else
				{
					var totadd = 0;

					for (var i=0; i<frm.elements.length; i++)
					{
						if (frm.elements[i].name.indexOf('title') != -1 && frm.elements[i].value != '') { totadd++; }
					}

					if (!totadd)
					{
						alert("You didn't enter any titles - no items to add!");
						return false;
					}
					else
					{
						return (numitems==1 ? true : confirm("Are you sure you want to add these items?\nNumber of items: " + totadd));
					}
				}
			}

			function checkAll(chk)
			{
				for (var i=0; i<numitems; i++)
				{
					$('ti' + i).checked = chk;
				}
			}
		</script>

		<div id="holdlyr" style="display:block">
			<font size="5" color="red">Please Hold - Populating Options...<br />&nbsp;</font>
		</div>

		<span class="note">
			Only <b>Title</b> is required - a default value will be used for any blank fields
			<p />
			Check the <b>+Invoice</b> box to also add this item to the current invoice.
		</span>
		<p />
		<form method="post" action="/admin/setup_items/quickaddUpdate.php" name="itmfrm" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="add" />
		<input type="hidden" name="platformID" value="<?php echo $platformID;?>" />
		<input type="hidden" name="numitems" value="<?php echo $numitems;?>" />

		<input type="submit" value="Add Items &gt;" class="btn" />
		<p />
		<?php
		$pg->addOnload('populate_selects()');

		$pg->outlineTableHead();

		$rowspan = ($open_invoice ? 2 : 1);
		?>
		<tr bgcolor="<?php echo $pg->color('table-head');?>">
			<td align="center" rowspan="<?php echo $rowspan;?>">&nbsp;</td>
			<td align="center"><b>UPC</b></td>
			<td align="center"><b>Title</b></td>
			<td align="center"><b>Type</b></td>
			<td align="center"><b>Year</b></td>
			<td colspan="2" align="center"><b>Price (N/U)</b></td>
			<td colspan="2" align="center"><b>Qty (N/U)</b></td>
		</tr>
		<?php
		if ($open_invoice)
		{
			?>
			<tr bgcolor="<?php echo $pg->color('table-head');?>">
				<td colspan="8">
					<input type="checkbox" onclick="checkAll(this.checked)" title="Check/Uncheck All" class="nb" checked="checked" />
					<b>+Invoice</b>
				</td>
			</tr>
			<?php
		}

		for ($i=0; $i<$numitems; $i++)
		{
			$bg = (($i%2)?$pg->color('table-cell'):$pg->color('table-cell2'));

			?>
			<tr bgcolor="<?php echo $bg;?>">
				<td align="right" rowspan="<?php echo $rowspan;?>"><font color="#BBBBBB"><?php echo ($i+1);?></font></td>
				<td>
					<input type="text" name="items[<?php echo $i;?>][upc]" size="25" onkeypress="return checkenter(this,event)" />
				</td>
				<td>
					<input type="text" name="items[<?php echo $i;?>][title]" size="30" onkeypress="return checkenter(this,event)" />
				</td>
				<td>
					<select name="items[<?php echo $i;?>][typeID]" id="type<?php echo $i;?>" size="1" onkeydown="return checkenter(this,event) && checkbackspace(this,event)"></select>
				</td>
				<td>
					<select name="items[<?php echo $i;?>][yearID]" id="year<?php echo $i;?>" size="1" onkeydown="return checkenter(this,event) && checkbackspace(this,event)"></select>
				</td>
				<td>
					$<input type="text" name="items[<?php echo $i;?>][price][<?php echo ITEM_NEW;?>]" size="5" onkeypress="return onlynumbers(this.value,event,true,true)" onblur="this.value=format_price(this.value,false)" />
				</td>
				<td>
					$<input type="text" name="items[<?php echo $i;?>][price][<?php echo ITEM_USED;?>]" size="5" onkeypress="return onlynumbers(this.value,event,true,true)" onblur="this.value=format_price(this.value,false)" />
				</td>
				<td>
					<input type="text" name="items[<?php echo $i;?>][qty_new]" size="2" onkeypress="return onlynumbers(this.value,event,true,true)" />
				</td>
				<td>
					<input type="text" name="items[<?php echo $i;?>][qty_used]" size="2" onkeypress="return onlynumbers(this.value,event,true,true)" />
				</td>
			</tr>
			<?php
			if ($open_invoice)
			{
				?>
				<tr bgcolor="<?php echo $bg;?>">
					<td colspan="8">
						<input type="checkbox" name="toinvoice[<?php echo $i;?>]" id="ti<?php echo $i;?>" value="<?php echo YES;?>" class="nb" checked="checked" />
						+Invoice / Options:
						<span id="options<?php echo $i;?>"></span>
					</td>
				</tr>
				<?php
			}
		}

		$pg->outlineTableFoot();

		if ($open_invoice)
		{
			?>
			<p class="note">
				<b>Add Checked Items to Section of Invoice:</b>
				<input type="radio" name="quickadd_type" id="qtt" value="<?php echo TRADE;?>" class="nb" onclick="setAddType(this.value)" checked="checked" />
				<label for="qtt">Trade</label>

				<input type="radio" name="quickadd_type" id="qts" value="<?php echo SALE;?>" class="nb" onclick="setAddType(this.value)" />
				<label for="qts">Sale</label>
			</p>
			<?php
		} // if open invoice

		?>
		<p>
			<input type="submit" value="Add Items &gt;" class="btn" />
		</p>
		</form>
		<?php
		if ($open_invoice)
		{
			?>
			<p />
			<span class="note">
				<b>Note:</b> If you select items to add to an invoice, but your items generate an error (IE: duplicate UPCs),<br />
				you will be returned to this screen and your invoice item selections will be forgotten!
			</span>
			<?php
		} // if open invoice

		$pg->addOnload('document.itmfrm.elements[4].focus()');
	}
}

$pg->foot();
?>