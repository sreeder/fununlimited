<?php
include('../../include/include.inc');

$cl = new check_login(STORE);
$error = new error('Point-of-Sale');
$pg = new admin_page();

$cust = new customers($pg);

$act = getGP('act');
$customerID = getGP('customerID');

if ($act == 'select')
{
	$_SESSION['customerID'] = $customerID;
	$cust->set_customerID($_SESSION['customerID'], true, false);
	$_SESSION['last_customer'] = $cust->info;

	if (@$_GET['reopen_invoiceID'])
	{
		$inv = new invoice($pg);
		$opened = $inv->set_invoiceID($_GET['reopen_invoiceID'],YES,$customerID);
		if ($opened) { header('Location: /admin/pos/invoice.php?act=view'); }
	}
	else
	{
		$posfunc = @$_POST['posfunc'];
		if ($posfunc == 'newinvoice') { header("Location: {$_SESSION['root_admin']}pos/invoice.php?act=newinvoice"); }
		else { header("Location: {$_SESSION['root_admin']}pos/pos.php"); }
	}
} // if act is 'select'
if ($act == 'YESsure')
{
	if (@$_SESSION['cust_invoiceID'])
	{
		// set the open invoice as inactive and not-completed
		$sql = 'UPDATE invoices SET inv_active='.NO.',inv_completed='.NO." WHERE inv_invoiceID={$_SESSION['cust_invoiceID']}";
		mysql_query($sql,$db);
		$error->mysql(__FILE__,__LINE__);
		$_SESSION['cust_invoiceID'] = 0;
	}
	$act = 'new';
} // if act is 'YESsure'
if ($act == 'new')
{
	if (@$_SESSION['customerID'])
	{
		if (@$_SESSION['cust_invoiceID']) { $act = 'sure'; }
		else
		{
			// UNSET VARS, ETC
			$_SESSION['customerID'] = 0; // just in case...
			unset($_SESSION['customerID']);

			while (list($var,$val) = each($_SESSION))
			{
				if (substr($var,0,5) == 'cust_') { unset($_SESSION[$var]); }
			}
		}
	}
	if ($act != 'sure') { $act = ''; }
} // if act is 'new'

if (@$_SESSION['customerID'] && $act != 'sure')
{
	$act = 'currentinfo';
	$cust->set_customerID($_SESSION['customerID']);
	$_SESSION['last_customer'] = $cust->info;
}

$titles = array(
	''            => 'Select Customer',
	'new'         => 'Select Customer',
	'search'      => 'Select Customer',
	'select'      => 'Customer Information',
	'currentinfo' => 'Customer Information',
	'sure'        => 'Close Current Customer?'
);

if ($act != 'delete')
{
	$pg->setTitle(@$titles[$act]);
	$pg->head(@$titles[$act],YES);
}

// should we print the last trade?
$print_tradeID = getG('print_tradeID');

if ($print_tradeID)
{
	?>
	<script type="text/javascript">
		function printTradeDocs()
		{
			var obj = null;

			if (tradedocs)
			{
				obj = tradedocs;
			}
			else if (window.frames)
			{
				obj = window.frames['tradedocs'];
			}
			else if (document.all)
			{
				obj = document.all['tradedocs'];
			}

			if (obj.focus)
			{
				obj.focus();
			}

			try
			{
				// this prevents IE7+ from auto-scaling the printout
				// bless you Google...
				obj.document.execCommand('print', false, null);
			}
			catch(e)
			{
				obj.print();
			}
		}
	</script>

	<p>
		<input type="button" value="Print Trade Documentation" class="btn" onclick="printTradeDocs()" />
		<iframe
			name="tradedocs" id="tradedocs" src="/admin/pos/trade_documentation.php?invoiceID=<?php echo $print_tradeID;?>"
			style="width:1px;height:1px" frameborder="0"
		></iframe>
	</p>
	<?php
} // if print the last trade

if ($act == '')
{
	// output the customer selection page
	if (strlen(@$_GET['status'])) { $pg->status($_GET['status']); }

	?>
	To find a customer, enter as much criteria below as you would like.
	<p />
	<table border="0" cellspacing="0" cellpadding="5">
		<tr>
			<td align="center" valign="top">
				<?php
				// output the last point-of-sale customer, if set
				if (isset($_SESSION['last_customer']))
				{
					$customerID = $_SESSION['last_customer']['customerID'];
					$rnk = new rankings($pg);
					$cust_rank = $rnk->getCustomerRankings($customerID);

					?>
					<?php echo $pg->outlineTableHead();?>
					<tr bgcolor="<?php echo $pg->color('table-head-darker');?>">
						<td colspan="2" align="center"><b>Last Point-of-Sale Customer</b></td>
					</tr>
					<tr>
						<td align="center" colspan="2" bgcolor="<?php echo $pg->color('table-head');?>">
							<?php echo "{$_SESSION['last_customer']['fname']} {$_SESSION['last_customer']['lname']}";?><br />
							<a href="/admin/pos/pos.php?act=select&customerID=<?php echo $_SESSION['last_customer']['customerID'];?>">Click to Reopen</a>
						</td>
					</tr>
					<tr>
						<td bgcolor="<?php echo $pg->color('table-label');?>"><b>Rank - Sales:</b></td>
						<td bgcolor="<?php echo $pg->color('table-cell2');?>" align="right"><?php echo $cust_rank['rnk_sales'];?></td>
					</tr>
					<tr>
						<td bgcolor="<?php echo $pg->color('table-label');?>"><b>Rank - Cash Trades:</b></td>
						<td bgcolor="<?php echo $pg->color('table-cell2');?>" align="right"><?php echo $cust_rank['rnk_cashtrades'];?></td>
					</tr>
					<tr>
						<td bgcolor="<?php echo $pg->color('table-label');?>"><b>Rank - Credit Trades:</b></td>
						<td bgcolor="<?php echo $pg->color('table-cell2');?>" align="right"><?php echo $cust_rank['rnk_credittrades'];?></td>
					</tr>
					<tr>
						<td bgcolor="<?php echo $pg->color('table-label');?>"><b>Rank - Returns:</b></td>
						<td bgcolor="<?php echo $pg->color('table-cell2');?>" align="right"><?php echo $cust_rank['rnk_returns'];?></td>
					</tr>
					<tr>
						<td bgcolor="<?php echo $pg->color('table-label');?>"><b>YTD Rank - Sales:</b></td>
						<td bgcolor="<?php echo $pg->color('table-cell2');?>" align="right"><?php echo $cust_rank['rnk_ytd_sales'];?></td>
					</tr>
					<tr>
						<td bgcolor="<?php echo $pg->color('table-label');?>"><b>YTD Rank - Cash Trades:</b></td>
						<td bgcolor="<?php echo $pg->color('table-cell2');?>" align="right"><?php echo $cust_rank['rnk_ytd_cashtrades'];?></td>
					</tr>
					<tr>
						<td bgcolor="<?php echo $pg->color('table-label');?>"><b>YTD Rank - Credit Trades:</b></td>
						<td bgcolor="<?php echo $pg->color('table-cell2');?>" align="right"><?php echo $cust_rank['rnk_ytd_credittrades'];?></td>
					</tr>
					<tr>
						<td bgcolor="<?php echo $pg->color('table-label');?>"><b>YTD Rank - Returns:</b></td>
						<td bgcolor="<?php echo $pg->color('table-cell2');?>" align="right"><?php echo $cust_rank['rnk_ytd_returns'];?></td>
					</tr>
					<?php echo $pg->outlineTableFoot();?>
					<p />
					<?php
				} // if there is a last-selected customer
				else
				{
					echo '&nbsp;';
				}
				?>
			</td>
			<td align="center" valign="top">
				<?php echo $cust->search_form('pos.php');?>
			</td>
		</tr>
	</table>
	<?php

	// output any customers with saved invoices
	$inv = new invoice($pg);
	list($del_invoices,$del_items) = $inv->deleteOldSavedInvoices();
	$inv->setSaved();
	$saved = $inv->getSaved();

	if (count($saved) || $del_invoices)
	{
		$half = ceil(count($saved)/2);

		?>
		<p />
		<hr width="75%" size="-1" color="#000000" noshade="noshade" />
		<?php
		if ($del_invoices)
		{
			?>
			<p />
			<?php
			$pg->status(
				"$del_invoices invoice" . getS($del_invoices)
				. " (containing $del_items item" . getS($del_items) . ') '
				. ($del_items!=1 ? 'were' : 'was') . ' just deleted because '
				. ($del_items!=1 ? 'they were' : 'it was') . ' 15 or more days old'
			);
		}

		if (count($saved))
		{
			?>
			<p />
			The following <?php echo count($saved);?> invoice<?php echo getS(count($saved));?> <?php echo (count($saved)!=1 ? 'have' : 'has');?> been saved in the past two weeks:
			<p />
			<font size="1"><b>Note:</b> This list is sorted most recently created to oldest</font>
			<p />
			<table border="0" cellspacing="0" cellpadding="0" width="75%">
				<tr>
					<td align="center" valign="top" width="50%">
						<?php
						while (list($a,$arr) = each($saved))
						{
							if (!$a || $a == $half)
							{
								if ($a == $half)
								{
									$pg->outlineTableFoot();
									?>
									</td>
									<td align="center" valign="top" width="50%">
									<?php
								}

								?>
								<?php echo $pg->outlineTableHead();?>
								<tr bgcolor="<?php echo $pg->color('table-head');?>">
									<td>&nbsp;</td>
									<td align="center"><b>Customer</b></td>
									<td align="center"><b>Created</b></td>
									<td align="center"><b>Select</b></td>
								</tr>
								<?php
							} // if first invoice or at the halfway point

							$bg = (($a/2) ? $pg->color('table-cell') : $pg->color('table-cell2'));

							$ago = ceil((strtotime(date('m/d/Y')) - strtotime(date('m/d/Y',$arr['inv_time']))) / (60 * 60 * 24));

							?>
							<tr bgcolor="<?php echo $bg;?>">
								<td align="right"><font color="#CCCCCC"><?php echo ($a+1);?></font></td>
								<td valign="top">
									<?php echo "{$arr['cus_lname']}, {$arr['cus_fname']}";?><br />
									<?php echo $pg->format('phone',$arr['cus_phone']);?>
								</td>
								<td>
									<?php echo date('m/d/Y h:ia',$arr['inv_time']);?><br />
									<font color="#BBBBBB"><?php echo ($ago ? "$ago day" . getS($ago) . ' ago' : 'today');?></font>
								</td>
								<td>
									<a href="/admin/pos/pos.php?act=select&customerID=<?php echo $arr['inv_customerID'];?>&reopen_invoiceID=<?php echo $arr['inv_invoiceID'];?>">Reopen Invoice &gt;</a>
								</td>
							</tr>
							<?php
						} // each saved invoice
						?>
						<?php echo $pg->outlineTableFoot();?>
					</td>
				</tr>
			</table>
			<?php
		} // if saved
	} // if there are invoices to delete
} // if no act
elseif ($act == 'sure')
{
	// an invoice is currently open - ask if they are sure they want to close the customer
	?>
	There is currently an open invoice; are you sure you want to close the current customer?
	<p />
	<b>Note:</b> If you choose to close the customer, the open invoice will be set as incomplete and <b>can be completed later</b>.
	<p />
	<form method="post" action="pos.php">
		<input type="hidden" name="act" value="">
		<input type="button" value="YES, Close Customer &gt;" onclick="this.form.act.value='YESsure';this.form.submit()" class="btn">
		<input type="button" value="No, Do Not Close Customer &gt;" onclick="this.form.act.value='customerinfo';this.form.submit()" class="btn">
	</form>
	<?php
} // elseif act is 'sure'
elseif ($act == 'search')
{
	// perform/output the customer search results
	$cust->pull_post();
	$cust->search();
	$cust->search_results('pos.php','addform','',array(),YES);
} // elseif act is 'search'
elseif ($act == 'currentinfo')
{
	// output the information for the current customer
	if (getG('updateranks'))
	{
		$rnk = new rankings($pg);
		$rnk->updateRankings();
		$pg->status('Updated Rankings');
	}

	$cust->show_info(YES);
} // elseif act is 'currentinfo'
elseif ($act == 'delete')
{
	// delete a customer (set them as inactive)
	$cust->set_customerID(getP('customerID'));
	$cust->delete();

	$pg->showUpdating(
		'Customer deleted...',
		"/admin/pos/pos.php?status=Deleted customer: <b>{$cust->info['fname']} {$cust->info['lname']}</b>"
	);
} // elseif act is 'delete'

if ($act != 'delete') { $pg->foot(); }
?>