<?php
/**
* Customer handling class (adding/editing/removing/searching/etc)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class customers extends customer_notes
{
	/**
	* page class reference
	* @var reference $pg
	*/
	var $pg;

	/**
	* Error reporting class
	* @var identifier $error
	*/
	var $error;

	/**
	* Current storeID
	* @var integer $storeID
	*/
	var $storeID;

	/**
	* Current customerID
	* @var integer $customerID
	*/
	var $customerID;

	/**
	* Customer information
	* @var array $info
	*/
	var $info;

	/**
	* Customer search criteria
	* @var array $criteria
	*/
	var $criteria;

	/**
	* Customer search results
	* @var array $results
	*/
	var $results;

	/**
	* Return page
	* @var string $return
	*/
	var $return;

	/**
	* Status variable used for error reporting (bad email, bad zip code, success, etc)
	* @var array $status
	*/
	var $status;

	/**
	 * Lookup tables
	 * @var	array	$lookup_tables
	 */
	var $lookup_tables;

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	*/
	function customers(&$pg)
	{
		$this->pg =& $pg;
		$this->storeID = $_SESSION['storeID'];
		$this->customerID = 0;
		$this->info = array();
		$this->criteria = array();
		$this->results = array();
		$this->status = array();

		$this->customer_notes(); // call constructor of extended class

		$this->error = new error('Customers');

		$this->lookup_tables = array(
			'hair_color' => array(
				'XXX' => 'Unknown or completely Bald',
				'BLK' => 'Black',
				'BLN' => 'Blond or Strawberry',
				'BLU' => 'Blue',
				'BRO' => 'Brown',
				'GRN' => 'Green',
				'GRY' => 'Gray or Partially Gray',
				'ONG' => 'Orange',
				'PLE' => 'Purple',
				'PNK' => 'Pink',
				'RED' => 'Red or Auburn',
				'SDY' => 'Sandy',
				'WHI' => 'White',
			),

			'eye_color' => array(
				'XXX' => 'Unknown',
				'MUL' => 'Multicolored',
				'HAZ' => 'Hazel',
				'GRY' => 'Gray',
				'GRN' => 'Green',
				'BRO' => 'Brown',
				'BLU' => 'Blue',
				'BLK' => 'Black',
			),

			'ethnicity' => array(
				'U' => 'Unknown',
				'P' => 'Pacific Islander',
				'H' => 'Hispanic',
				'A' => 'Asian',
				'B' => 'Black',
				'I' => 'American Indian or Alaskan Native',
				'W' => 'White',
			),
		);
	} // end function customers

	/**
	* Pulls all customers into $this->customers
	* @param integer $active Active customers YES/NO [optional, default YES]
	*/
	function get_customers($active=YES)
	{
		global $db;

		$this->customers = array();

		$sql = "SELECT * FROM customers WHERE cus_active=".$active." ORDER BY cus_lname,cus_fname";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			$emplID = $row['cus_customerID'];

			// pull the sales/trades data for this customer
			$data = array(
				'month'=>array(
					'sale'=>array('total'=>rand(1,5),'items'=>rand(5,20),'amount'=>(rand(30,100)+(rand(0,99)/100))),
					'trade'=>array('total'=>rand(1,5),'items'=>rand(5,20),'amount'=>(rand(30,100)+(rand(0,99)/100)))
				),
				'ytd'=>array(
					'sale'=>array('total'=>rand(1,5),'items'=>rand(5,20),'amount'=>(rand(30,100)+(rand(0,99)/100))),
					'trade'=>array('total'=>rand(1,5),'items'=>rand(5,20),'amount'=>(rand(30,100)+(rand(0,99)/100)))
				)
			);

			$this->customers[] = array_merge($row,array('data'=>$data));
		}
	}

	/**
	* Sets the customerID and pulls the customer information into $this->info
	* @param integer $customerID
	* @param boolean $setsession save the customer info to the session? [optional, default true]
	* @param boolean $get_totals get the user's totals? [optional, default true]
	*/
	function set_customerID($customerID, $setsession=true, $get_totals=true)
	{
		global $db;

		$this->customerID = $customerID;
		$this->info = array();

		$sql = "SELECT * FROM customers WHERE cus_customerID=$customerID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			// remove 'cus_'
			while (list($key,$val) = each($row))
			{
				$this->info[substr($key,4)] = $val;
			}
		}

		if ($get_totals)
		{
			$get = array(
				'sales'        => 'total_sale',
				'cashtrades'   => 'total_trade_cash',
				'credittrades' => 'total_trade_credit',
				'returns'      => 'total_return'
			);
			$rnk = new rankings($this->pg);
			while (list($type, $key) = each($get))
			{
				$rnk->setRankings($type, NO, 0, $customerID);
				$rank = $rnk->getRankings($customerID);
				$total = (count($rank) ? $rank['rnk_' . $type . '_value'] : 0);
				$this->info[$key] = $total;
			}
		} // if get totals

		if ($setsession)
		{
			$_SESSION['customerID'] = $customerID;
			$_SESSION['cust_info'] = $this->info;
		}
	}

	/**
	* Output the customer information
	* @param integer $shownav Show navigation buttons? [optional, default NO]
	*/
	function show_info($shownav=NO)
	{
		if ($shownav == YES)
		{
			// output the navigation buttons
			$numsale = 0;
			while (list($a,$arr) = each($_SESSION['cust_items']))
			{
				if ($arr['ini_type'] == SALE) { $numsale++; }
			}
			reset($_SESSION['cust_items']);

			?>
			<script type="text/javascript">
				function go(where)
				{
					if ((where != 'close' && where != 'returns') || (where == 'close' && <?php echo $numsale;?>==0) && confirm('Are you sure you would like to close the current invoice?') == true)
					{
						frm = document.gofrm;
						frm.act.value = where;
						frm.submit();
					}
					else if (where == 'close' && <?php echo $numsale;?>)
					{
						document.location = '/admin/pos/invoice_payment.php';
					}
					else if (where == 'returns')
					{
						document.location = '/admin/pos/returns.php?act=new';
					}
				}
			</script>

			<form method="post" action="/admin/pos/invoice.php" name="gofrm">
			<input type="hidden" name="act" value="">
			<?php
			if (!@$_SESSION['cust_invoiceID'])
			{
				?>
				<input type="button" value="Create New Invoice &gt;" onclick="go('newinvoice')" class="btn">
				<input type="button" value="Re-open Existing Invoice &gt;" onclick="go('viewopen')" class="btn">
				<?php
			}
			else
			{
				?>
				<input type="button" value="Edit Current Invoice &gt;" onclick="go('view')" class="btn">
				<?php
			}
			?>
			</form>

			<p />
			<?php
		}

		?>
		<table border="0" cellspacing="0" cellpadding="5" width="90%">
			<tr>
				<td align="center" valign="top" width="50%">
					<!-- contact information -->
					<?php echo $this->show_info_table('Contact Information');?>
					<p />
					<!-- rankings -->
					<?php echo $this->show_info_table('Rankings',150);?>
				</td>
				<td align="center" valign="top" width="50%">
					<!-- in-store information -->
					<?php echo $this->show_info_table('In-Store Information');?>
					<p />
					<!-- other information -->
					<?php echo $this->show_info_table('Other Information');?>
					<p />
					<input type="button" value="Edit Customer Information &gt;" class="btn" onclick="document.location='/admin/pos/customers.php?act=edit&customerID=<?php echo $_SESSION['customerID'];?>'">
				</td>
			</tr>
		</table>
		<p />
		<?php
		// show any incomplete/saved invoices
		$inv = new invoice($pg);
		$inv->setSaved($this->customerID);
		$saved = $inv->getSaved();

		if (count($saved))
		{
			?>
			<p />
			The following <?php echo count($saved);?> invoice<?php echo (count($saved)!=1?'s':'');?> <?php echo (count($saved)!=1?'are':'is');?> incomplete/saved:
			<p />
			<?php echo $this->pg->outlineTableHead();?>
			<tr bgcolor="<?php echo $this->pg->color('table-head');?>">
				<td>&nbsp;</td>
				<td align="center"><b>Created</b></td>
				<td align="center"><b>Select</b></td>
			</tr>
			<?php
			while (list($a,$arr) = each($saved))
			{
				$bg = (($a/2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));

				$ago = ceil((strtotime(date('m/d/Y'))-strtotime(date('m/d/Y',$arr['inv_time'])))/(60*60*24));

				?>
				<tr bgcolor="<?php echo $bg;?>">
					<td valign="top">
						<?php echo "{$arr['cus_lname']}, {$arr['cus_fname']}";?><br />
						<?php echo $this->pg->format('phone',$arr['cus_phone']);?>
					</td>
					<td>
						<?php echo date('m/d/Y h:ia',$arr['inv_time']);?><br />
						<font color="#BBBBBB"><?php echo ($ago?"$ago day".($ago!=1?'s':'')." ago":'today');?></font>
					</td>
					<td>
						<a href="/admin/pos/pos.php?act=select&customerID=<?php echo $arr['inv_customerID'];?>&reopen_invoiceID=<?php echo $arr['inv_invoiceID'];?>">Reopen Invoice &gt;</a>
					</td>
				</tr>
				<?php
			}
			$this->pg->outlineTableFoot();
			?><p /><?php
		}
		?>
		<hr width="95%" size="-1" color="#000000" noshade="noshade" />

		<div id="noteHidden" style="display:none;width:90%;text-align:left"><input type="button" value="Show Customer Notes &gt;" onclick="showNotes('show')" class="btn" style="width:200px" /></div>
		<div id="noteShown" style="display:block">
			<?php

			// customer notes
			$this->setNotes($_SESSION['customerID']);
			$notes = $this->getNotes();

			// get note totals
			$note_totals = array(NOTE_POSITIVE=>0,NOTE_NEUTRAL=>0,NOTE_NEGATIVE=>0);
			while (list($a,$arr) = each($notes)) { $note_totals[$arr['cun_type']]++; }
			reset($notes);

			?>
			<table border="0" cellspacing="0" cellpadding="0" width="90%">
				<tr>
					<td width="20%"><input type="button" value="&lt; Hide Customer Notes" onclick="showNotes('hide')" class="btn" style="width:200px" /></td>
					<td width="60%" align="center">
						<?php
						while (list($type,$total) = each($note_totals))
						{
							$image = $this->getNoteTypeImage($type);
							$word = $this->getNoteTypeWord($type);

							echo "$image $total $word ";
						}
						?>
					</td>
					<td width="20%">&nbsp;</td>
				</tr>
			</table>
			<p />
			<?php

			// output the add-note form
			?>
			<script type="text/javascript">
				function verifyNote(frm)
				{
					if (frm.elements['data[note]'].value == '') { alert('Please enter a note.'); frm.elements['data[note]'].focus(); return false; }
					else { return true; }
				}
				function deleteNote(noteID)
				{
					if (confirm('Are you SURE you want to delete this note?')) { document.location = '/admin/pos/noteUpdate.php?act=delete&noteID='+noteID; }
				}
				function showNotes(sh)
				{
					document.getElementById('noteHidden').style.display = (sh=='show'?'none':'block');
					document.getElementById('noteShown').style.display = (sh=='show'?'block':'none');
				}
			</script>

			<table border="0" cellspacing="1" cellpadding="3">
				<form method="post" action="/admin/pos/noteUpdate.php" onsubmit="return verifyNote(this)">
				<input type="hidden" name="act" value="add" />
				<input type="hidden" name="return" value="/admin/pos/pos.php" />
				<input type="hidden" name="data[customerID]" value="<?php echo $_SESSION['customerID'];?>" />
				<tr><td colspan="2" align="center" bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Add Note to Customer</b></td></tr>
				<tr bgcolor="<?php echo $this->pg->color('table-cell');?>">
					<td rowspan="2"><?php
						$types = $this->getNoteTypes();
						while (list($a,$type) = each($types))
						{
							if ($type == NOTE_NEUTRAL) { $c = ' checked="checked"'; }
							else { $c = ''; }
							$image = $this->getNoteTypeImage($type);
							$word = $this->getNoteTypeWord($type);
							?><input type="radio" name="data[type]" value="<?php echo $type;?>" id="type<?php echo $type;?>" class="nb" onclick="this.form.elements['data[note]'].select()"<?php echo $c;?> /><label for="type<?php echo $type;?>"><?php echo $image;?> <?php echo $word;?></label><br /><?php
						}
					?></td>
					<td align="right">
						<input type="text" name="data[note]" size="75" />
					</td>
				</tr>
				<tr bgcolor="<?php echo $this->pg->color('table-cell');?>">
					<td align="right">
						Please remember that notes cannot be changed!
						<input type="submit" value="Add Note &gt;" class="btn" />
					</td>
				</tr>
				</form>
			</table>
			<p />
			<?php echo $this->pg->outlineTableHead('90%');?>
			<?php
			while (list($a,$arr) = each($notes))
			{
				if ($arr['cun_type'] == NOTE_POSITIVE) { $bg = '#DDDDFF'; }
				elseif ($arr['cun_type'] == NOTE_NEUTRAL) { $bg = '#EEEEEE'; }
				elseif ($arr['cun_type'] == NOTE_NEGATIVE) { $bg = '#FFDDDD'; }

				$image = $this->getNoteTypeImage($arr['cun_type']);
				$word = $this->getNoteTypeWord($arr['cun_type']);
				?>
				<tr>
					<td bgcolor="<?php echo $bg;?>">
						<table border="0" cellspacing="0" cellpadding="3" width="100%">
							<tr>
								<td width="100%" style="white-space:normal">
									<?php echo "$image $word";?><br />
									<b>Note:</b> <?php echo stripslashes($arr['cun_note']);?><br />
								</td>
								<td align="right" valign="bottom">
									<?php
									if ($arr['cun_userID'] == $_SESSION['userID'])
									{
										?><a href="javascript:deleteNote(<?php echo $arr['cun_noteID'];?>)">Delete Note &gt;</a><br /><?php
									}
									?>
									<?php echo date('m/d/Y h:ia',$arr['cun_time']);?> by <?php echo $arr['use_name'];?>
								</td>
							</tr>
						</table>
					</td>
				</tr>
				<?php
			}

			if (!count($notes)) { ?><tr><td align="center" bgcolor="<?php echo $this->pg->color('table-cell');?>">--- No Notes Found for the Current Customer ---</td></tr><?php }
			?>
			<?php echo $this->pg->outlineTableFoot();?>
		</div>
		<?php
	}

	/**
	* Outputs a table in the customer information screen
	* @param string $item Which item?
	* @param string $width table width
	*/
	function show_info_table($item,$width='100%')
	{
		global $db;

		$tables = array(
			'Contact Information' => array(
				'Name'            => $this->info['fname'].' '.$this->info['lname'],
				'Address'         => $this->info['address'],
				'City, State Zip' => trim($this->info['city'].', '.$this->info['state'].' '.$this->pg->format('zip',$this->info['zip']),' ,'),
				'Phone'           => $this ->pg->format('phone',$this->info['phone']),
				'Cell Phone'      => $this->pg->format('phone',$this->info['cellphone']),
				'E-Mail'          => (strlen($this->info['email'])?'<a href="mailto:'.$this->info['email'].'">'.$this->info['email'].'</a>':'')
			),
			'Rankings' => array(
				'Sales'               => '',
				'Trades - Cash'       => '',
				'Trades - Credit'     => '',
				'Returns'             => '',
				'YTD Sales'           => '',
				'YTD Trades - Cash'   => '',
				'YTD Trades - Credit' => '',
				'YTD Returns'         => '',
				'Update Rankings'     => '<a href="'.$_SESSION['root_admin'].'pos/pos.php?updateranks='.YES.'">Click to Update Rankings</a>',
				'View All Rankings'   => '<a href="'.$_SESSION['root_admin'].'reports/customer_rankings.php">Click to View All Rankings</a>'
			),
			'In-Store Information' => array(
				'Credit'          => $this->info['creditamount'],
				'Sales'           => '$'.number_format(@$this->info['total_sale'],2),
				'Trades - Cash'   => '$'.number_format(@$this->info['total_trade_cash'],2),
				'Trades - Credit' => '$'.number_format(@$this->info['total_trade_credit'],2),
				'Returns'         => '$'.number_format(@$this->info['total_return'],2)
			),
			'Other Information' => array(
				'Date Added' => date('m/d/Y h:ia',$this->info['timeadded']),
				'Height'     => $this->info['height'],
				'Weight'     => $this->info['weight'],
				'DOB'        => $this->info['dob'],
				'Gender'     => ($this->info['gender']==MALE?'Male':($this->info['gender']==FEMALE?'Female':'')),
				'Ethnicity'  => getA($this->lookup_tables['ethnicity'], $this->info['ethnicity'], 'Unknown'),
				'Hair Color' => getA($this->lookup_tables['hair_color'], $this->info['hair_color'], 'Unknown'),
				'Eye Color'  => getA($this->lookup_tables['eye_color'], $this->info['eye_color'], 'Unknown'),
				'Active'     => ($this->info['active']==YES?'YES':'No')
			)
		);

		// pull in the 'of' rankings (is/of)
		$total_rankings = array(0,0,0,0);
		$sql = "SELECT MAX(rnk_sales),MAX(rnk_cashtrades),MAX(rnk_credittrades),MAX(rnk_returns),MAX(rnk_ytd_sales),MAX(rnk_ytd_cashtrades),MAX(rnk_ytd_credittrades),MAX(rnk_ytd_returns) FROM rankings WHERE rnk_storeID={$_SESSION['storeID']} AND rnk_archived=" . NO;
		$result = mysql_query($sql,$db);
		if (mysql_num_rows($result)) { $total_rankings = mysql_fetch_row($result); }

		if (isset($tables[$item]))
		{
			$arr = $tables[$item];

			$this->pg->outlineTableHead($width);
			?><tr><td colspan="2" bgcolor="<?php echo $this->pg->color('table-label');?>" align="center"><b><?php echo $item;?></b></td></tr><?php

			if ($item == 'Rankings')
			{
				$fields = array_keys($arr);

				$sql = "SELECT rnk_sales,rnk_cashtrades,rnk_credittrades,rnk_returns,rnk_ytd_sales,rnk_ytd_cashtrades,rnk_ytd_credittrades,rnk_ytd_returns FROM rankings WHERE rnk_customerID={$_SESSION['customerID']} AND rnk_storeID={$_SESSION['storeID']} AND rnk_archived=" . NO;
				$result = mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);

				if (mysql_num_rows($result)) { $ranks = mysql_fetch_row($result); }
				else { $ranks = array(0,0,0,0,0,0,0,0); }

				function pad_ranks($is,$of)
				{
					$padto = 6;
					return '#'.str_repeat('&nbsp;',($padto-strlen($is))).$is.' / '.str_repeat('&nbsp;',($padto-strlen($of))).$of;
				}

				$f1 = '<font face="Courier New" size="2">'; $f2 = '</font>';
				$arr[$fields[0]] = $f1 . ($ranks[0] ? pad_ranks($ranks[0],$total_rankings[0]) : 'No Sales') . $f2;
				$arr[$fields[1]] = $f1 . ($ranks[1] ? pad_ranks($ranks[1],$total_rankings[1]) : 'No Cash Trades') . $f2;
				$arr[$fields[2]] = $f1 . ($ranks[2] ? pad_ranks($ranks[2],$total_rankings[2]) : 'No Credit Trades') . $f2;
				$arr[$fields[3]] = $f1 . ($ranks[3] ? pad_ranks($ranks[3],$total_rankings[3]) : 'No Returns') . $f2;

				$arr[$fields[4]] = $f1 . ($ranks[4] ? pad_ranks($ranks[4],$total_rankings[4]) : 'No YTD Sales') . $f2;
				$arr[$fields[5]] = $f1 . ($ranks[5] ? pad_ranks($ranks[5],$total_rankings[5]) : 'No YTD Cash Trades') . $f2;
				$arr[$fields[6]] = $f1 . ($ranks[6] ? pad_ranks($ranks[6],$total_rankings[6]) : 'No YTD Credit Trades') . $f2;
				$arr[$fields[7]] = $f1 . ($ranks[7] ? pad_ranks($ranks[7],$total_rankings[7]) : 'No YTD Returns') . $f2;
			}

			while (list($title,$val) = each($arr))
			{
				if (strlen($val))
				{
					$lh = '';
					$lf = '';

					if ($item == 'In-Store Information')
					{
						if ($title == 'Sales')
						{
							$lh = '<a href="'.$_SESSION['root_admin'].'pos/custhistory.php?show=sales" title="View Customer Sale History">';
							$lf = '</a>';
						}
						elseif (substr($title, 0, 6) == 'Trades')
						{
							$lh = '<a href="'.$_SESSION['root_admin'].'pos/custhistory.php?show=trades" title="View Customer Trade History">';
							$lf = '</a>';
						}
						elseif ($title == 'Returns')
						{
							$lh = '<a href="'.$_SESSION['root_admin'].'pos/custhistory.php?show=returns" title="View Customer Return History">';
							$lf = '</a>';
						}
					}
					if ($item == 'Rankings')
					{
						$ytd = (substr($title,0,3)=='YTD' ? YES : NO);
						$check_title = $title;
						if ($ytd) { $check_title = substr($title,4); }
						if ($check_title == 'Sales')
						{
							$lh = '<a href="'.$_SESSION['root_admin'].'reports/customer_rankings.php?type=sales&ytd=' . $ytd . '" title="View Sales Rankings">';
							$lf = '</a>';
						}
						elseif ($check_title == 'Trades - Cash')
						{
							$lh = '<a href="'.$_SESSION['root_admin'].'reports/customer_rankings.php?type=cashtrades&ytd=' . $ytd . '" title="View Cash Trade Rankings">';
							$lf = '</a>';
						}
						elseif ($check_title == 'Trades - Credit')
						{
							$lh = '<a href="'.$_SESSION['root_admin'].'reports/customer_rankings.php?type=credittrades&ytd=' . $ytd . '" title="View Credit Trade Rankings">';
							$lf = '</a>';
						}
						elseif ($check_title == 'Returns')
						{
							$lh = '<a href="'.$_SESSION['root_admin'].'reports/customer_rankings.php?type=returns&ytd=' . $ytd . '" title="View Return Rankings">';
							$lf = '</a>';
						}
					}

					?>
					<tr>
						<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><b><?php echo $title;?></b></td>
						<td width="100%" bgcolor="<?php echo $this->pg->color('table-cell2');?>"><?php
							if ($title == "Credit")
							{
								// show the 'set credit' button
								?>
								<script type="text/javascript">
									var orig_credit = -1;
									function set_credit(orig)
									{
										if (orig_credit == -1) { orig_credit = orig; }
										var newcredit = prompt("Please enter the customer's credit amount:",orig_credit);
										var credit = parseFloat(newcredit);
										if (newcredit != null && credit != orig_credit)
										{
											setcreditfrm.location = '/admin/pos/customers_setcredit.php?credit='+credit;
										}
									}

									function change_credit(credit)
									{
										document.getElementById('creditamount').innerHTML = '$'+format_price(credit);
									}
								</script>

								<span id="creditamount">$<?php echo $val;?></span>
								&nbsp;&nbsp;&nbsp;&nbsp;<a href="javascript:set_credit(<?php echo $val;?>)"><img src="/images/setcredit.gif" width="73" height="11" border="0" style="vertical-align:bottom" /></a>
								<iframe name="setcreditfrm" src="/admin/pos/customers_setcredit.php" width="1" height="1" frameborder="0" marginwidth="0" marginheight="0">
									Your browser does not support iframes. Please upgrade.
								</iframe>
								<?php
							}
							else { echo $lh.$val.$lf; }
						?></td>
					</tr>
					<?php
				}
			}

			$this->pg->outlineTableFoot();
		}
	}

	/**
	* Displays the add/edit customer form
	* @param integer $active
	* @param string $return page to return to after adding the customer [optional, default '']
	*/
	function add_form($active,$return="")
	{
		$val = new validate();

		if ($this->customerID) { $word = 'edit'; }
		else { $word = 'add'; }

		if (strlen($val->strip(@$this->info['phone'])) == 7)
		{
			$this->info['phone'] = $_SESSION['store_info']['sto_areacode'] . $this->info['phone'];
		}

		?>
		<font size="1">
			<b>Note:</b> Only first name, last name, and phone are required for sale customers.<br />
			Everything but cell phone and email are required for trade customers.
		</font>
		<p />

		<form method="post" action="customers.php" name="customer" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="doadd" />
		<input type="hidden" name="customerID" value="<?php echo $this->customerID;?>" />
		<input type="hidden" name="info[storeID]" value="<?php echo $this->storeID;?>" />
		<input type="hidden" name="info[active]" value="<?php echo $active;?>" />
		<input type="hidden" name="return" value="<?php echo $return;?>" />
		<input type="hidden" name="after" value="" />
		<?php echo $this->pg->outlineTableHead();?>
		<tr>
			<td colspan="2" bgcolor="<?php echo $this->pg->color('table-label');?>" align="center"><b><?php echo ucwords($word);?> Customer</b></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>First Name:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="info[fname]" size="25" value="<?php echo @$this->info['fname'];?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Last Name:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="info[lname]" size="30" value="<?php echo @$this->info['lname'];?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Address:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="info[address]" size="40" value="<?php echo @$this->info['address'];?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>City, State Zip:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>">
				<input type="text" name="info[city]" size="20" value="<?php echo @$this->info['city'];?>">
				<select name="info[state]" size="1">
					<?php

					$st = new states();

                                        foreach ($st->states as $abb => $name)
					{
						if ($abb == @$this->info['state']) { $s = ' selected="selected"'; }
						else { $s = ''; }

						?>
						<option value="<?php echo $abb;?>"<?php echo $s;?>><?php echo $name;?></option>
						<?php
					}

					?>
				</select>
				<input type="text" name="info[zip]" size="12" maxlength="10" value="<?php echo $this->pg->format('zip',$val->strip(@$this->info['zip']));?>">
			</td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Phone Number:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="info[phone]" size="16" value="<?php echo $this->pg->format('phone',$val->strip(@$this->info['phone']));?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Cell Phone Number:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="info[cellphone]" size="16" value="<?php echo $this->pg->format('phone',$val->strip(@$this->info['cellphone']));?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>E-Mail:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="info[email]" size="40" value="<?php echo @$this->info['email'];?>"></td>
		</tr>
		<tr>
			<td colspan="2" bgcolor="<?php echo $this->pg->color('table-label');?>" align="center"><b>Required Trade Information</b></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Ethnicity:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>">
				<select name="info[ethnicity]">
					<?php

					$select = getA($this->info, 'ethnicity');

					foreach ($this->lookup_tables['ethnicity'] as $value => $label)
					{
						$sel = getSelected($select==$value);

						?>
						<option value="<?php echo $value;?>"<?php echo $sel;?>><?php echo $label;?></option>
						<?php
					}

					?>
				</select>
			</td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Hair Color:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>">
				<select name="info[hair_color]">
					<?php

					$select = getA($this->info, 'hair_color');

					foreach ($this->lookup_tables['hair_color'] as $value => $label)
					{
						$sel = getSelected($select==$value);

						?>
						<option value="<?php echo $value;?>"<?php echo $sel;?>><?php echo $label;?></option>
						<?php
					}

					?>
				</select>
			</td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Eye Color:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>">
				<select name="info[eye_color]">
					<?php

					$select = getA($this->info, 'eye_color');

					foreach ($this->lookup_tables['eye_color'] as $value => $label)
					{
						$sel = getSelected($select==$value);

						?>
						<option value="<?php echo $value;?>"<?php echo $sel;?>><?php echo $label;?></option>
						<?php
					}

					?>
				</select>
			</td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Height:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="info[height]" size="8" value="<?php echo htmlspecialchars(@$this->info['height']);?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Weight:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="info[weight]" size="4" value="<?php echo htmlspecialchars(@$this->info['weight']);?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>DOB:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="info[dob]" size="14" value="<?php echo htmlspecialchars(@$this->info['dob']);?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Gender:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>">
				<select name="info[gender]" size="1">
					<option value="0"></option>
					<?php

					$vals = array(
						'Male'   => MALE,
						'Female' => FEMALE
					);

					foreach ($vals as $s => $v)
					{
						?>
						<option value="<?php echo $v;?>"<?php echo ($v==@$this->info['gender']?' selected="selected"':'');?>><?php echo $s;?></option>
						<?php
					}

					?>
				</select>
			</td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>ID #:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="info[idnumber]" size="15" value="<?php echo htmlspecialchars(@$this->info['idnumber']);?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>ID Expiration:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="info[idexpiration]" size="7" value="<?php echo htmlspecialchars(@$this->info['idexpiration']);?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>ID State:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>">
				<select name="info[idstate]">
					<?php

					foreach ($st->states as $abb => $name)
					{
						if ($abb == @$this->info['idstate']) { $s = ' selected="selected"'; }
						else { $s = ''; }

						?>
						<option value="<?php echo $abb;?>"<?php echo $s;?>><?php echo $name;?></option>
						<?php
					}

					?>
				</select>
			</td>
		</tr>
		<?php echo $this->pg->outlineTableFoot();?>
		<p />
		<?php
		if ($this->customerID && @$_SESSION['cust_invoiceID'])
		{
			?>
			<input type="submit" value="Edit Customer/View Invoice &gt;" class="btn" onclick="this.form.after.value='invoice'">
			<input type="submit" value="Edit Customer/View Info &gt;" class="btn" onclick="this.form.after.value='info'">
			<?php
		}
		else
		{
			?>
			<input type="submit" value="<?php echo ucwords($word);?> Customer &gt;" class="btn">
			<?php
		}
		?>
		<input type="reset" value="Reset Form &gt;" class="btn">
		</form>
		<?php echo $this->pg->cancel($_SESSION['root_admin'].'pos/customers.php');?>

		<script type="text/javascript">
			function verify(frm)
			{
				var foc = null;
				if (frm.elements['info[fname]'].value == '') { alert('You must enter a first name.'); foc = frm.elements['info[fname]']; }
				else if (frm.elements['info[lname]'].value == '') { alert('You must enter a last name.'); foc = frm.elements['info[lname]']; }
				//else if (frm.elements['info[address]'].value == '') { alert('You must enter an address.'); foc = frm.elements['info[address]']; }
				//else if (frm.elements['info[city]'].value == '') { alert('You must enter a city.'); foc = frm.elements['info[city]']; }
				//else if (frm.elements['info[state]'].options[frm.elements['info[state]'].selectedIndex].value == '') { alert('You must select a state.'); foc = frm.elements['info[state]']; }
				//else if (frm.elements['info[zip]'].value == '') { alert('You must enter a zip.'); foc = frm.elements['info[zip]']; }
				else if (frm.elements['info[phone]'].value == '' && frm.elements['info[cellphone]'].value == '') { alert('You must enter a phone number.'); foc = frm.elements['info[phone]']; }

				if (foc == null && frm.elements['info[dob]'].value.length && !validDate(frm.elements['info[dob]'].value))
				{
					// invalid DOB
					alert('Please enter a valid DOB (MM/DD/YYYY)');
					foc = frm.elements['info[dob]'];
				}

				if (foc) { foc.focus(); return false; }
				else { return true; }
			}
		</script>
		<?php

		$this->pg->addOnload("document.customer.elements['info[fname]'].focus()");
	}

	/**
	* Pulls the information from $_POST
	*/
	function pull_post()
	{
		$this->customerID = @$_POST['customerID'];
		if (isset($_POST['info']))
		{
			$this->info = $_POST['info'];
			while (list($key,$val) = each($this->info)) { $this->info[$key] = stripslashes(trim($val)); }
			reset($this->info);
		}
		if (isset($_POST['criteria']))
		{
			$this->criteria = $_POST['criteria'];
			while (list($key,$val) = each($this->criteria)) { $this->criteria[$key] = stripslashes(trim($val)); }
			reset($this->criteria);
		}

		$this->return = getGP('return');
	}

	/**
	* Adds/edits the customer stored in $this->info
	* @param boolean $online online store registration? [optional, default NO]
	* @return integer customerID
	* @access public
	*/
	function add($online=NO)
	{
		global $db;

		if ($online)
		{
			$_SESSION['register_info'] = $this->info;
		}

		$val = new validate();
		if (!$online && strlen($val->strip(@$this->info['phone'])) == 7)
		{
			$this->info['phone'] = $_SESSION['store_info']['sto_areacode'] . $this->info['phone'];
		}

		if ($this->_validateInfo($online))
		{
			// add the customer to the database

			if ($this->customerID) { $sql = "UPDATE customers SET "; }
			else { $sql = "INSERT INTO customers SET "; }

			$this->info['zip'] = $val->strip($this->info['zip']);
			$this->info['phone'] = $val->strip($this->info['phone']);
			$this->info['cellphone'] = $val->strip(@$this->info['cellphone']);
			if ($online)
			{
				$this->info['locale'] = ONLINE;
				unset($this->info['pass_verify']);
				unset($this->info['from_edit']);
			}
			else
			{
				$this->info['locale'] = INSTORE;
			}

			$vals = array();
			while (list($key,$val) = each($this->info))
			{
				if ($key == 'pass')
				{
					$vals[] = "cus_pass=ENCODE('" . mysql_real_escape_string($val) . "','" . ENCODEKEY . "')";
				}
				else
				{

					$testval = mysql_real_escape_string($val);
					$newval= !empty($testval)? "'$testval'" : "NULL";
					$vals[] = "cus_{$key}= $newval";

					//$vals[] = "cus_{$key}='" . mysql_real_escape_string($val) . "'";
				}
			}
			if (!$this->customerID) { $vals[] = "cus_timeadded='" . time() . "'"; }

			$sql .= implode(',',$vals);

			if ($this->customerID) { $sql .= " WHERE cus_customerID=$this->customerID"; }
			mysql_query($sql,$db);
			$dupe = $this->error->mysql(__FILE__,__LINE__,YES);

			if ($dupe == NO)
			{
				if (!$this->customerID) { $return = mysql_insert_id(); }
				else { $return = $this->customerID; }

				$this->status[0] = "ADDED";
				return $return;
			}
			else
			{
				$this->status[0] = "DUPLICATE";
				return -1;
			}
		}
	}

	/**
	* Delete the customer stored in $this->customerID (sets to inactive - doesn't delete)
	*/
	function delete()
	{
		global $db;

		if ($this->customerID)
		{
			// set the customer as inactive
			$sql = "UPDATE customers SET cus_active=".NO." WHERE cus_customerID=$this->customerID";

			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$_SESSION['customerID'] = 0;

			$this->status[0] = "DELETED";
		}
	}

	/**
	* Re/de-activates the customer stored in $this->customerID (sets to inactive - doesn't delete)
	* @param integer $active If YES, deactivate the customer. If no, reactivate the customer. [optional, default YES]
	*/
	function activate($active=YES)
	{
		global $db;

		if ($this->customerID)
		{
			// re/de-activate the customer
			$sql = "UPDATE customers SET cus_active=".($active==YES?NO:YES)." WHERE cus_customerID=$this->customerID";

			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$this->status[0] = "DELETED";
		}
	}

	/**
	* Permanently deletes the customer stored in $this->customerID and all traces of it (set customerID=0 where needed)
	*/
	function permdelete()
	{
		global $db;

		if ($this->customerID)
		{
			$sql = "DELETE FROM customers WHERE cus_customerID=$this->customerID";

			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$this->status[0] = DELETED;
		}
	}

	/**
	* Check the status of the customer addition/edit
	* @return integer YES/NO
	*/
	function was_added()
	{
		if ($this->status[0] == ADDED || $this->status[0] == EDITED) { return YES; }
		else { return NO; }
	}

	/**
	* Check the status of the customer re/de-activation/deletion
	* @return integer YES/NO
	*/
	function was_deleted()
	{
		if ($this->status[0] == DELETED) { return YES; }
		else { return NO; }
	}

	/**
	* Validates information in $this->info (phone numbers, zip codes, etc) before $this->add() can be completed
	* @param boolean $online online store registration? [optional, default NO]
	* @return integer YES/NO
	* @access private
	*/
	function _validateInfo($online=NO)
	{
		$val = new validate();

		$this->status = array();

		if (($online && !strlen(@$this->info['zip'])) || (strlen(@$this->info['zip']) && !$val->zip_code($this->info['zip']))) { $this->status[] = BADZIP; }
		if (($online && !strlen(@$this->info['phone'])) || (strlen(@$this->info['phone']) && !$val->phone($this->info['phone']))) { $this->status[] = BADPHONE; }
		if (strlen(@$this->info['cellphone']) && !$val->phone($this->info['cellphone'])) { $this->status[] = BADPHONE; }
		if (($online && !strlen(@$this->info['email'])) || (strlen(@$this->info['email']) && !$val->email($this->info['email']))) { $this->status[] = BADEMAIL; }
		if ($online)
		{
			// validate the online store registration info
			if (!@$this->info['from_edit'])
			{
				if (strlen(@$this->info['user']) && !$this->uniqueUsername($this->info['user'])) { $this->status[] = DUPLICATE; }
				if (!strlen(@$this->info['user']) || !strlen(@$this->info['pass'])) { $this->status[] = NO_USERPASS; }
				if (@$this->info['pass'] != @$this->info['pass_verify']) { $this->status[] = PASS_NO_MATCH; }
			}
		}

		return (count($this->status) ? NO : YES);
	}

	/**
	* Return YES/NO if the username is unique
	* @param string $user
	* @return boolean
	* @access public
	*/
	function uniqueUsername($user)
	{
		global $db;

		$sql = "SELECT * FROM customers WHERE cus_user='" . mysql_real_escape_string($user) . "'";
		$result = mysql_query($sql,$db);
		return (mysql_num_rows($result) ? NO : YES);
	}

	/**
	* Generate and show any error message(s) according to the contents of $this->status
	* @param boolean $return return the errors? [optional, default NO]
	* @access public
	*/
	function show_errors($return=NO)
	{
		$errors = array();

		$strings = array(
			DUPLICATE     => ($return ? 'Username already in use' : 'A customer with that name/phone number already exists'),
			BADZIP        => 'Invalid zip code',
			BADPHONE      => 'Invalid phone number',
			BADEMAIL      => 'Invalid email address',
			NO_USERPASS   => 'No username/password entered',
			PASS_NO_MATCH => 'Passwords do not match',
		);

		for ($i=0; $i<count($this->status); $i++)
		{
			$errors[] = @$strings[$this->status[$i]];
		}

		if (!$return)
		{
			if ($this->customerID) { $word = 'Edit'; }
			else { $word = 'Add'; }
			$this->pg->error($errors);
			echo "<p />Please correct the error".(count($errors)==1?'':'s')." before pressing <b>$word Customer</b> again.<p />";
		}
		else
		{
			return $errors;
		}
	}

	/**
	* Fill $this->info with values in $this->criteria
	*/
	function criteria_to_info()
	{
		$crit= (isset($_SESSION['customer_criteria']) ? $_SESSION['customer_criteria'] : null);
		$this->info = $crit;
	}

	/**
	* Output the customer search form
	* @param string $action Form action
	* @param integer $small only show phone/email/first name/last name [optional, default NO]
	* @param integer $setfocus set the focus to the phone field on load? [optional, default YES]
	* @param array $hidden hidden form elements [optional, default array()]
	*/
	function search_form($action,$small=NO,$setfocus=YES,$hidden=array())
	{
		$val = new validate();

		?>
		<form method="post" action="<?php echo $action;?>" name="custsrch" id="custsrch" onsubmit="return verify(this)">
		<input type="hidden" name="act" value="search">
		<?php
		while (list($k,$v) = each($hidden)) { ?><input type="hidden" name="<?php echo $k;?>" value="<?php echo $v;?>"><?php }
		$this->pg->outlineTableHead();
		?>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Phone Number:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="criteria[phone]" size="16" value="<?php echo $this->pg->format('phone',$val->strip(@$this->criteria['phone']));?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Email Address:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="criteria[email]" size="30" value="<?php echo @$this->criteria['email'];?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>First Name:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="criteria[fname]" size="25" value="<?php echo @$this->criteria['fname'];?>"></td>
		</tr>
		<tr>
			<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Last Name:</b></td>
			<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="criteria[lname]" size="30" value="<?php echo @$this->criteria['lname'];?>"></td>
		</tr>
		<?php
		if ($small == NO)
		{
			?>
			<tr>
				<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Address:</b></td>
				<td bgcolor="<?php echo $this->pg->color('table-cell');?>"><input type="text" name="criteria[address]" size="40" value="<?php echo @$this->criteria['address'];?>"></td>
			</tr>
			<tr>
				<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>City, State Zip:</b></td>
				<td bgcolor="<?php echo $this->pg->color('table-cell');?>">
					<input type="text" name="criteria[city]" size="20" value="<?php echo @$this->criteria['city'];?>">
					<select name="criteria[state]" size="1">
						<?php

						$st = new states();

						foreach ($st->states as $abb => $name)
						{
							if ($abb == @$this->criteria['state']) { $s = ' selected="selected"'; }
							else { $s = ''; }

							?>
							<option value="<?php echo $abb;?>"<?php echo $s;?>><?php echo $name;?></option>
							<?php
						}

						?>
					</select>
					<input type="text" name="criteria[zip]" size="12" maxlength="10" value="<?php echo $this->pg->format('zip',$val->strip(@$this->criteria['zip']));?>">
				</td>
			</tr>
			<tr>
				<td bgcolor="<?php echo $this->pg->color('table-label');?>"><b>Search:</b></td>
				<td bgcolor="<?php echo $this->pg->color('table-cell');?>">
					<?php
					if (!isset($this->criteria['search'])) { $this->criteria['search'] = 'all'; }
					$c1 = (@$this->criteria['search']=='all'?' checked="checked"':'');
					$c2 = (@$this->criteria['search']=='onlystore'?' checked="checked"':'');
					$c3 = (@$this->criteria['search']=='onlybiz'?' checked="checked"':'');
					?>
					<input type="radio" name="criteria[search]" id="a" value="all" style="border-width:0;vertical-align:middle"<?php echo $c1;?>> <label style="cursor:hand" for="a">All customers</label><br />
					<input type="radio" name="criteria[search]" id="os" value="onlystore" style="border-width:0;vertical-align:middle"<?php echo $c2;?>> <label style="cursor:hand" for="os">Only customers from this store</label><br />
					<input type="radio" name="criteria[search]" id="ob" value="onlybiz" style="border-width:0;vertical-align:middle"<?php echo $c3;?>> <label style="cursor:hand" for="ob">Customers with business with this store</label>
				</td>
			</tr>
			<?php
		}

		$this->pg->outlineTableFoot();
		?>
		<p />
		<input type="submit" value="Find Customer &gt;" class="btn"> <input type="button" value="Clear Form &gt;" class="btn" onclick="clearfrm(this.form)">
		</form>

		<script type="text/javascript">
			function verify(frm)
			{
				var totsearch = 0;
				for (var i=0; i<frm.elements.length; i++)
				{
					if (frm.elements[i].type == 'text' && frm.elements[i].value.length) { totsearch++; }
					else if (frm.elements[i].type == 'select-one' && frm.elements[i].options[frm.elements[i].selectedIndex].value.length) { totsearch++; }
				}

				if (!totsearch) { alert('Please enter at least one criteria.'); document.custsrch.elements['criteria[phone]'].focus(); return false; }
				else { return true; }
			}

			function clearfrm(frm)
			{
				firstIDX = -1;
				for (var i=0; i<frm.elements.length; i++)
				{
					if (frm.elements[i].type != 'hidden' && firstIDX == -1) { firstIDX = i; }

					if (frm.elements[i].type == 'text') { frm.elements[i].value= ''; }
					else if (frm.elements[i].type == 'select-one') { frm.elements[i].selectedIndex = 0; }
				}

				frm.elements[firstIDX].focus();
			}
		</script>
		<?php

		if ($setfocus == YES) { $this->pg->addOnload("document.custsrch.elements['criteria[phone]'].focus()"); }
	}

	/**
	* Perform the customer search
	*/
	function search()
	{
		global $db;

		$_SESSION['customer_criteria'] = $this->criteria;

		$val = new validate();
		$this->criteria['phone'] = $val->strip(@$this->criteria['phone']);
		$this->criteria['zip'] = $val->strip(@$this->criteria['zip']);

		$where = array('cus_active='.YES);
		if (@$this->criteria['search'] == 'onlybiz') { $from = 'customers,invoices'; }
		else { $from = 'customers'; }

		$sql = "SELECT * FROM $from WHERE ";

		if (strlen(@$this->criteria['phone'])) { $where[] = "(cus_phone LIKE '%".$this->criteria['phone']."%' OR cus_cellphone LIKE '%".$this->criteria['phone']."%')"; }
		if (strlen(@$this->criteria['email'])) { $where[] = "cus_email='".mysql_real_escape_string($this->criteria['email'])."'"; }
		if (strlen(@$this->criteria['fname'])) { $where[] = "cus_fname LIKE '%".mysql_real_escape_string($this->criteria['fname'])."%'"; }
		if (strlen(@$this->criteria['lname'])) { $where[] = "cus_lname LIKE '%".mysql_real_escape_string($this->criteria['lname'])."%'"; }
		if (strlen(@$this->criteria['address'])) { $where[] = "cus_address LIKE '%".mysql_real_escape_string($this->criteria['address'])."%'"; }
		if (strlen(@$this->criteria['city'])) { $where[] = "cus_city LIKE '%".mysql_real_escape_string($this->criteria['city'])."%'"; }
		if (strlen(@$this->criteria['state'])) { $where[] = "cus_state='%".mysql_real_escape_string($this->criteria['state'])."'"; }
		if (strlen(@$this->criteria['zip'])) { $where[] = "cus_zip LIKE '%".mysql_real_escape_string($this->criteria['zip'])."%'"; }
		if (@$this->criteria['search'] == 'onlystore') { $where[] = "cus_storeID=".$_SESSION['storeID']; }
		elseif (@$this->criteria['search'] == 'onlybiz') { $where[] = "cus_customerID=inv_customerID"; $where[] = "inv_storeID=".$_SESSION['storeID']; }

		$sql .= implode(' AND ',$where);

		if (@$this->criteria['search'] == 'onlybiz') { $sql .= ' GROUP BY cus_customerID'; }

		$sql .= ' ORDER BY cus_lname,cus_fname LIMIT '.MAX_CUSTOMER_SEARCH_RESULTS;

		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		$this->results = array();
		while ($row = mysql_fetch_assoc($result)) { $this->results[] = $row; }
	}

	/**
	* Output the customer search results
	* @param string $action Form action
	* @param string $ifnoresult action to take if there are no results
	* @param string $return page to return to after the customer has been added (if they add one) [optional, default '']
	* @param array $hidden hidden form elements [optional, default array()]
	* @param boolean $isforpos is this for the POS customer selection? [optional, default NO]
	*/
	function search_results($action,$ifnoresult,$return='',$hidden=array(),$isforpos=NO)
	{
		if (!count($this->results))
		{
			if ($ifnoresult == 'addlinksearch')
			{
				?>
				Your search returned no results.
				<p />
				Click <a href="/admin/pos/customers.php?act=add&fill=<?php echo YES;?>&return=<?php echo $return;?>">here</a> to add a new customer, or search again below.
				<p />
				<?php
				$this->search_form($action);
			}
			elseif ($ifnoresult == 'addform')
			{
				?>
				Your search returned no results.
				<p />
				<input type="button" value="&lt; Search Again" onclick="document.location='/admin/pos/pos.php'" class="btn" />
				<p />
				<?php

				$this->criteria_to_info();
				$this->add_form(YES);
			}
			elseif ($ifnoresult == 'wishlist')
			{
				?>
				Your search returned no results.
				<p />
				Click <a href="/admin/pos/customers.php?act=add&fill=<?php echo YES;?>&return=<?php echo $return;?>">here</a> to add a new customer, or search again below.
				<p />
				<?php
				$this->search_form('/admin/utilities/wishlist.php','wishlist',urlencode("{$_SESSION['root_admin']}utilities/wishlist.php?customerID=%customerID%"),array('use_itemID'=>@$_POST['use_itemID']));
			}
		}
		else
		{
			// output the results
			$val = new validate();

			?>
			<input type="button" value="&lt; Search Again" onclick="document.location='<?php echo $action;?>'" class="btn" />
			<input type="button" value="Add New Customer &gt;" onclick="document.location='/admin/pos/customers.php?act=add&fill=<?php echo YES;?>'" class="btn" />
			<p />

			<?php
			if (count($this->results) == MAX_CUSTOMER_SEARCH_RESULTS)
			{
				?>
				<font size="1"><b>Note:</b> Your search returned <?php echo MAX_CUSTOMER_SEARCH_RESULTS;?>+ results. Only the first <?php echo MAX_CUSTOMER_SEARCH_RESULTS;?> are shown. Please narrow your criteria.</font>
				<p />
				<?php
			}
			?>

			<script type="text/javascript">
				var selID = 0;
				function sel(id)
				{
					selID = id;

					var obj = eval('document.cusresults.cus'+id);
					obj.checked = true;

					<?php
					if (!$isforpos) { ?>document.cusresults.s.disabled = false;<?php }
					else { ?>document.cusresults.sni.disabled = false;document.cusresults.sci.disabled = false;<?php }
					?>
					document.cusresults.d.disabled = false;
				}

				function go(where,posfunc)
				{
					if (!selID) { alert('You must select a customer.'); }
					else
					{
						if (where != 'delete' || (where == 'delete' && confirm('Are you SURE you want to delete the selected customer?\n\nPlease note: deleting a customer sets them as inactive - their invoice history will not be removed.') == true))
						{
							var frm = document.cusresults;
							frm.act.value = where;
							frm.posfunc.value = posfunc;
							frm.submit();
						}
					}
				}
			</script>

			<form method="post" action="<?php echo $action;?>" name="cusresults">
			<input type="hidden" name="act" value="select" />
			<input type="hidden" name="posfunc" value="" />
			<?php
			while (list($k,$v) = each($hidden)) { ?><input type="hidden" name="<?php echo $k;?>" value="<?php echo $v;?>" /><?php }
			$this->pg->outlineTableHead();
			?>
			<tr>
				<td bgcolor="<?php echo $this->pg->color('table-head');?>">&nbsp;</td>
				<td bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Name</b></td>
				<td bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Address</b></td>
				<td bgcolor="<?php echo $this->pg->color('table-head');?>"><b>Phone</b></td>
			</tr>

			<?php
			$selcustomerID = 0;
			while (list($a,$arr) = each($this->results))
			{
				if (!$a) { $selcustomerID = $arr['cus_customerID']; }

				$bg = (($a%2)?$this->pg->color('table-cell'):$this->pg->color('table-cell2'));
				?>
				<label for="cus<?php echo $arr['cus_customerID'];?>">
				<tr style="cursor:hand">
					<td bgcolor="<?php echo $bg;?>"><input type="radio" name="customerID" value="<?php echo $arr['cus_customerID'];?>" id="cus<?php echo $arr['cus_customerID'];?>" class="nb" onclick="sel(<?php echo $arr['cus_customerID'];?>)"></td>
					<td bgcolor="<?php echo $bg;?>"><?php echo "{$arr['cus_lname']}, {$arr['cus_fname']}";?></td>
					<td bgcolor="<?php echo $bg;?>"><?php echo $arr['cus_address'];?><br /><?php echo trim("{$arr['cus_city']}, {$arr['cus_state']} ".$this->pg->format('zip',$val->strip(@$arr['cus_zip'])),' ,');?><?php echo (strlen($arr['cus_email'])?"<br /><a href=\"mailto:{$arr['cus_email']}\">{$arr['cus_email']}</a>":'');?></td>
					<td bgcolor="<?php echo $bg;?>"><?php
						if (strlen(@$arr['cus_phone'])) { echo $this->pg->format('phone',$val->strip(@$arr['cus_phone'])); }
						if (strlen(@$arr['cus_cellphone'])) { echo '<br />Cell: '.$this->pg->format('phone',$val->strip(@$arr['cus_cellphone'])); }
					?></td>
				</tr>
				</label>
				<?php
			}

			if ($selcustomerID) { $this->pg->addOnload("sel($selcustomerID)"); }
			?>

			<?php echo $this->pg->outlineTableFoot();?>

			<p />

			<?php
			if (!$isforpos)
			{
				?><input type="button" value="Select Customer &gt;" id="s"<?php echo (!$selcustomerID?' disabled="disabled"':'');?> onclick="go('select')" class="btn"><?php
			}
			else
			{
				?>
				<input type="button" value="Select Customer/New Invoice &gt;" id="sni"<?php echo (!$selcustomerID?' disabled="disabled"':'');?> onclick="go('select','newinvoice')" class="btn">
				<input type="button" value="Select Customer/Customer Info &gt;" id="sci"<?php echo (!$selcustomerID?' disabled="disabled"':'');?> onclick="go('select','custinfo')" class="btn">
				<?php
			}
			?>
			<input type="button" value="Delete Customer &gt;" id="d"<?php echo (!$selcustomerID?' disabled="disabled"':'');?> onclick="go('delete')" class="btn">
			</form>
			<?php

		}
	}

	/**
	* Returns an array of required fields for the trade information screen
	*/
	function check_trade_info()
	{
		global $db;

		$st = new states();
		$states = array();

		foreach ($st->states as $abb => $name)
		{
			$states[$name] = $abb;
		}

		$fields = array(
			'address'      =>array('Address','address',40),
			'city'         =>array('City','city',20),
			'state'        =>array('State','state',1,$states),
			'zip'          =>array('Zip Code','zip',12),
			'ethnicity'    =>array('Ethnicity','ethnicity',1, $this->lookup_tables['ethnicity'], true),
			'hair_color'   =>array('Hair Color','hair_color',1, $this->lookup_tables['hair_color'], true),
			'eye_color'    =>array('Eye Color','eye_color',1, $this->lookup_tables['eye_color'], true),
			'height'       =>array('Height','height',8),
			'weight'       =>array('Weight','weight',4),
			'dob'          =>array('DOB','dob',14),
			'gender'       =>array('Gender','gender',1,array(''=>0,'Male'=>MALE,'Female'=>FEMALE)),
			'idnumber'     =>array('ID #','idnumber',15),
			'idexpiration' =>array('ID Expiration','idexpiration',7),
			'idstate'      =>array('ID State','idstate',1,$states),
		);

		$unknown_values = array('', 'XXX', 'U');

		$req = array();

		$sql = "SELECT * FROM customers WHERE cus_customerID=".$_SESSION['customerID'];
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		if (mysql_num_rows($result))
		{
			$row = mysql_fetch_assoc($result);
			if (!strlen(trim($row['cus_address']))) { $req[] = $fields['address']; }
			if (!strlen(trim($row['cus_city']))) { $req[] = $fields['city']; }
			if (!strlen(trim($row['cus_state']))) { $req[] = $fields['state']; }
			if (!strlen(trim($row['cus_zip']))) { $req[] = $fields['zip']; }

			if (in_array($row['cus_ethnicity'], $unknown_values) || !isset($this->lookup_tables['ethnicity'][$row['cus_ethnicity']]))
			{
				$req[] = $fields['ethnicity'];
			}

			if (in_array($row['cus_hair_color'], $unknown_values) || !isset($this->lookup_tables['hair_color'][$row['cus_hair_color']]))
			{
				$req[] = $fields['hair_color'];
			}

			if (in_array($row['cus_eye_color'], $unknown_values) || !isset($this->lookup_tables['eye_color'][$row['cus_eye_color']]))
			{
				$req[] = $fields['eye_color'];
			}

			if (!strlen(trim($row['cus_height']))) { $req[] = $fields['height']; }
			if (!strlen(trim($row['cus_weight']))) { $req[] = $fields['weight']; }
			if (!strlen(trim($row['cus_dob']))) { $req[] = $fields['dob']; }
			if (!$row['cus_gender']) { $req[] = $fields['gender']; }
			if (!strlen(trim($row['cus_idnumber']))) { $req[] = $fields['idnumber']; }
			if (!strlen(trim($row['cus_idexpiration']))) { $req[] = $fields['idexpiration']; }
			if (!strlen(trim($row['cus_idstate']))) { $req[] = $fields['idstate']; }
		}

		return $req;
	}

	/**
	* Return the password for the given customerID
	* @param integer $customerID
	* @return string
	* @access public
	*/
	function getCurrentPassword($customerID)
	{
		global $db;

		$sql = "SELECT DECODE(cus_pass,'FunUnlmtd') AS pass FROM customers WHERE cus_customerID=$customerID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		if (mysql_num_rows($result))
		{
			$row = mysql_fetch_assoc($result);
			return $row['pass'];
		}
		else { return ''; }
	}

	/**
	* Save the password for the given customerID
	* @param integer $customerID
	* @param string $password
	* @access public
	*/
	function savePassword($customerID,$password)
	{
		global $db;

		$sql = "UPDATE customers SET cus_pass=ENCODE('" . mysql_real_escape_string($password) . "','FunUnlmtd') WHERE cus_customerID=$customerID";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
	}
}

?>
