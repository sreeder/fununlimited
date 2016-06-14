<?php
/**
* Administration page display class (header, footer, background colors, etc)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 10/05/2012
* @package Fun Unlimited
*/

class admin_page
{
	/**
	* Site colors
	* @var array $colors
	*/
	var $colors;

	/**
	* Page title
	* @var string $title
	*/
	var $title;

	/**
	* Onload event(s)
	* @var array $onload
	*/
	var $onload;

	/**
	* Start time of page
	* @var float $start
	*/
	var $start;

	/**
	* Output the full header? (images, menu, etc)
	* @var integer $full
	*/
	var $full;

	/**
	* Center the page contents?
	* @var integer $center
	*/
	var $center;

	/**
	* Error reporting class
	* @var identifier $error
	*/
	var $error;

	/**
	* Class constructor
	* Initializes variables
	*/
	function admin_page()
	{
		$this->colors = array(
			'table-head'            => '#CFCFCF', // table header background
			'table-head-darker'     => '#BFBFBF', // darker table header background
			'table-head-lighter'    => '#DFDFDF', // lighter table header background
			'table-label'           => '#CFCFCF', // table label (IE: for tables with left column = label, right column = textbox)
			'table-cell'            => '#E7E7E7', // table cell background (single cell or the first cell of alternating color cells)
			'table-cell2'           => '#EEEEEE', // table cell background (second cell of alternating color cells)
			'body'                  => '#FFFFFF', // body background color
			'text'                  => '#000000', // text color
			'link'                  => '#1E86DD', // link color (non-hover/visited)
			'link-hover'            => '#1E86DD', // link color (hover)
			'head-bg'               => '#0064B9', // header background
			'menu-bg'               => '#0064B9', // menu background color
			'menu-bg-mouseover'     => '#0073D4', // menu background color (mouseover)
			'menu-bg-sub'           => '#999999', // submenu background color
			'menu-bg-sub-mouseover' => '#AAAAAA', // submenu background color
			'menu-bg-sub-around'    => '#EEEEEE', // "around the submenu" background color
			'menu-bg-pos'           => '#DDDDDD', // point-of-sale menu background color
			'menu-bg-pos-mouseover' => '#E5E5E5', // point-of-sale menu background color (mouseover)
			'menu-text'             => '#FFFFFF', // menu text
			'menu-link'             => '#FFFFFF', // menu link color (non-hover/visited)
			'menu-link-hover'       => '#FFFFFF'  // menu link color (hover)
		);
		// set constants for all colors (_ instead of - though; IE: TABLE_HEAD)
		while (list($name,$color) = each($this->colors))
		{
			$constant = strtoupper(str_replace('-','_',$name));
			if (!defined($constant))
			{
				define($constant,$color);
			}
		}
		$this->onload = array();
		$this->setFull(YES);
		$this->setCenter(YES);

		$this->start = $_SESSION['start_time'];

		$this->error = new error('Page');
	}

	/**
	* Sets the page title
	* @param string $title
	*/
	function setTitle($title) { $this->title = $title; }

	/**
	* Returns the page title
	* @return string $title
	* @access private
	*/
	function _getTitle() { return $this->title; }

	/**
	* Set $this->full
	* @param boolean $full
	* @access public
	*/
	function setFull($full) { $this->full = $full; }

	/**
	* Return $this->full
	* @return boolean
	* @access private
	*/
	function _getFull() { return $this->full; }

	/**
	* Set $this->center
	* @param boolean $center
	* @access public
	*/
	function setCenter($center) { $this->center = $center; }

	/**
	* Return $this->center
	* @return boolean
	* @access private
	*/
	function _getCenter() { return $this->center; }

	/**
	* Prints the page header
	* @param string $pagehead Page header [optional, default '']
	* @param integer $posmenu Output the point-of-sale menu? [optional, default NO]
	*/
	function head($pagehead='',$posmenu=NO)
	{
		$full   = $this->_getFull();
		$center = $this->_getCenter();

		if ($full)
		{
			$this->addOnload('showDocSize()');
			$this->addOnload('showDisplayTime(' . sprintf('%0.0f',($this->micro_time() * 1000)) . ')'); // start time is the start of output
		}

		// if a full page is being shown, there is no open customer, no POST variables, and the page isn't update.php or keepalive.php, check for software updates
		/*
		if ($full && !@$_SESSION['customerID'] && !count($_POST) && !in_array(basename($_SERVER['PHP_SELF']),array('update.php','keepalive.php')) && !@$_GET['ispopup'])
		{
			// check for unapplied software updates
			$upd = new update();
			$upd->setUpdates(NO);
			$updateIDs = $upd->getUnappliedUpdateIDs();

			if (count($updateIDs)) { $this->addOnload('gotoUpdates('.count($updateIDs).')'); }
		}
		*/

		?><html>
		<head>
			<title>Fun Unlimited<?=(strlen($this->title) ? ' - ' . $this->title : '');?></title>
			<link rel="stylesheet" type="text/css" href="/styles/global.css">
			<script type="text/javascript" src="/scripts/functions.js"></script>
			<script type="text/javascript" src="/scripts/admin_head.js"></script>
			<script type="text/javascript" src="/scripts/prototype.js"></script>
		</head>
		<body bgcolor="<?=$this->color('body');?>" text="<?=$this->color('text');?>" topmargin="0" leftmargin="0" rightmargin="0" bottommargin="0" marginheight="0" marginwidth="0" onload="if(typeof(doOnload)!='undefined'){doOnload();}">
			<table border="0" cellspacing="0" cellpadding="0" width="100%">
				<?php
				if ($full)
				{
					?>
					<tr>
						<td colspan="2" bgcolor="<?=$this->color('head-bg');?>">
							<table border="0" cellspacing="0" cellpadding="3">
								<tr>
									<td><a href="/admin/index.php" onmouseover="window.status='Return Home';return true" onmouseout="window.status='';return true"><img src="/images/admin_head.gif" width="403" height="68" border="0"></a></td>
									<td align="left" valign="top">
										<?php
										if (@$_SESSION['loggedin']) { $this->quick_lookup_form(); }
										else { ?>&nbsp;<?php }
										?>
									</td>
								</tr>
							</table>
						</td>
					</tr>
					<?php
				}
				?>
				<tr>
					<?php
					if ($full)
					{
						?>
						<td valign="top">
							<script language="javascript" src="/scripts/navmenu<?=(!@$_SESSION['loggedin']?'_notloggedin':'');?>.js"></script>
							<?php
							if (@$_SESSION['cust_invoiceID'] && !$posmenu)
							{
								$blink_time = 500;
								?>
								<p />
								<center>
									<font size="1">
										&nbsp; <span id="invopenblink"><b>An invoice is currently open!</b></span><br />
										&nbsp; <a href="/admin/pos/invoice.php?act=view"><b>Click Here to View Invoice</b></a>
									</font>
								</center>
								<script type="text/javascript">
									function OpenInvoiceBlink()
									{
										var obj = document.getElementById('invopenblink');
										obj.style.visibility = (obj.style.visibility=='hidden'?'visible':'hidden');
										var time = (obj.style.visibility=='hidden'?<?=$blink_time/2;?>:<?=$blink_time;?>);
										setTimeout('OpenInvoiceBlink()',time);
									}
								</script>
								<?php
								$this->addOnload('OpenInvoiceBlink()',$blink_time);
							}
							?>

							<p />
							<!--
							<iframe name="keepalive" src="/admin/keepalive.php" width="30" height="25" frameborder="0" scrolling="no" marginwidth="0" marginheight="0">
								Your browser does not support iframes. Please upgrade.
							</iframe>
							-->
						</td>
						<?php
					}
					?>
					<td class="normal" <?=($center ? 'align="center" ' : '');?>valign="top" width="100%">
						<?php
						if ($posmenu)
						{
							$pos = new pos($this);
							$pos->menu();
						}

						if (strlen($pagehead)) { $this->pageHead($pagehead); }
						?>

						<!-- end header - begin page content -->
						<?php

		$this->check_permissions();
	}

	/**
	* Prints the page footer
	*/
	function foot()
	{
		global $db;

		$full = $this->_getFull();

		// check for messages to show Fun Unlimited of Logan administrators
		if (@$_SESSION['storeID'] == 1)
		{
			$sql = "SELECT * FROM messages_from_scott WHERE mfs_shown=".NO." ORDER BY mfs_time";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($row = mysql_fetch_assoc($result))
			{
				$this->addOnload('alert(\'Message from Scott Carpenter:\n\n'.mysql_escape_string($row['mfs_message']).'\')');
			}

			$sql = "UPDATE messages_from_scott SET mfs_shown=".YES;
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}

		if ($full) { ?><br />&nbsp;<?php }

						?>
						<!-- end page content - begin footer -->

					</td>
				</tr>
			</table>
			<?php
			if ($full)
			{
				?>
				<hr width="100%" size="1" color="#000000">
				<center>
				<table border="0" cellspacing="0" cellpadding="0" width="99%">
					<tr>
						<td width="45%" valign="top" class="pagebottom"><?php
							if (@$_SESSION['loggedin'])
							{
								if ($_SESSION['storeID'])
								{
									$sto = new stores($this);
									$sto->set_storeID($_SESSION['storeID']);
									$store_name = (strlen($sto->info['name'])?$sto->info['name']:'Unknown Store');
								}
								else { $store_name = "All Stores"; }

								?>
								<b>Logged In As:</b> <?=$_SESSION['name'];?><br />
								<b>Store:</b> <?=$store_name;?><br />
								<?php
							}
							?>
							<b><?=(PRODUCTION?'Production':'Development');?> Server</b>
						</td>
						<td width="10%" align="center">
							<?php
							if (@$_SESSION['loggedin']) { ?><a href="javascript:void(0)" onclick="openSalesGraphs()"><img src="/images/salesgraphs.jpg" width="83" height="36" border="0" alt="Graph Current Sales Figures" /></a><?php }
							?>
						</td>
						<td width="45%" align="right" valign="top" class="pagebottom">
							Site Designed/Programmed by <a href="mailto:scott@payforstay.com">Scott Carpenter</a><br />
							All Code Copyright &copy; 2003-<?php echo date('Y');?><br />
							Generated <span id="docsize">[size]</span> in <?=$this->elapsed();?>s<br />
							<!--Displayed in <span id="docdisplaytime">[time]</span>-->
						</td>
					</tr>
				</table>
				</center>
			<?php
			}
			?>
			<?=$this->showOnload();?>
			<!-- execution time: <?=$this->elapsed();?>s -->
		</body>
		</html>
		<?php

		while (@ob_end_flush()); // flush the output buffer(s)
	}

	/**
	* Prints the navigation menu
	*/
	function nav_menu()
	{
		/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		!!! IF YOU CHANGE *ANY* PART OF THE MENU OUTPUT, COPY THE OUTPUT OF /SCRIPTS/NAVMENU.PHP TO /SCRIPTS/NAVMENU.JS !!!
		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

		$loggedin = @$_SESSION['loggedin'];
		$usertype = @$_SESSION['usertype'];

		$output = array();
		$output[] = '<table border="0" cellspacing="1" cellpadding="0" width="140">';

		$menu = array();

		if (!$loggedin) { $menu[] = array('Log In','login.php'); }
		else
		{
			// if they're an admin, they get some added functionality
			if ($usertype == ADMIN)
			{
				$menu[] = array('Manage Stores','setup_misc/stores.php');
			}

			$menu[] = array('Point of Sale','pos/pos.php');
			$menu[] = array(
				'Utilities',
				array(
					array('Wishlists',                        'utilities/wishlist.php'),
					//array('Preorders',                        'utilities/preorder.php'),
					array('Store to Store Inventory Movement','utilities/invmove.php'),
					array('Order Generation',                 'utilities/order.php'),
					array('Receive Order',                    'utilities/receive_order.php'),
					array('Barebones Utilities',              'bare/index.php'),
					array('Top 500 Ranked Address Entry',     'utilities/top500.php'),
					array('Manually Update Rankings',         'update_rankings.php'),
					array('Serial Number History',            'utilities/serial_number_history.php'),
				)
			);
			$menu[] = array(
				'Item Setup',
				array(
					array('Items',                        'setup_items/items.php'),
					array('Platforms',                    'setup_items/platforms.php'),
					array('Whole Platform Item Info',     'setup_items/platform_items.php'),
					array('Whole Platform Source Pricing','setup_items/platform_source.php'),
					array('Whole Platform Quantities',    'setup_items/begqty.php'),
					array('Item UPC Setup',               'setup_items/enterupc.php'),
					array('Quick Item Add',               'setup_items/quickadd.php'),
					array('Quick Item Add - 1 Item',      'setup_items/quickadd.php?numitems=1'),
					//array('Edit Items by Criteria',       'setup_items/criteria_edit.php')
				)
			);
			$menu[] = array(
				'Invoice Setup',
				array(
					array('Sales Milestone Discounts','setup_invoice/milestones.php'),
					array('Cash/Credit Percentages',  'setup_invoice/ccpercs.php'),
					array('Box Discounts',            'setup_invoice/box_discounts.php'),
					array('Per-Copy Discounts',       'setup_invoice/percopy_discounts.php'),
					array('Condition Discounts',      'setup_invoice/condition_discounts.php'),
					array('Minimum Prices',           'setup_invoice/min_prices.php'),
					array('Maximum Copies',           'setup_invoice/max_copies.php')
				)
			);
			$menu[] = array(
				'Misc Setup',
				array(
					array('Employees',                                          'setup_misc/employees.php'),
					array('Online Store - Store Inventory Selection Priorities','setup_misc/priorities.php')
				)
			);
			$menu[] = array(
				'Reports',
				array(
					//array('Doubles List',            'reports/doubles.php'),
					array('Price List',              'reports/price_list.php'),
					array('Inventory Status',        'reports/inventory_status.php'),
					array('Changed Prices List',     'reports/price_changes.php'),
					array('Customer List',           'reports/customer_list.php'),
					array('Customer Rankings',       'reports/customer_rankings.php'),
					array('Current Sales Graphs',    'reports/sales_graphs.php'),
					array('Item History',            'reports/item_history.php'),
					array('Summary Report',          'reports/summary.php'),
					array('Best Selling Items',      'reports/best_sellers.php'),
					array('Sales/Trades by Area',    'reports/by_area.php'),
					array('Sales/Trades by Age/Sex', 'reports/by_demo.php'),
					array('Utah Pawn File Download', 'reports/pawn_download.php'),
				)
			);
			$menu[] = array(
				'End of Year',
				array(
					array('Item Quantities','endofyear/items.php')
				)
			);
			$menu[] = array('Log Out','logout.php');
		}

		$smtot = 0;

		while (list($a,$arr) = each($menu))
		{
			unset($page);unset($parr);
			if (!is_array($arr[1])) { list($title,$page) = $arr; $te = ''; }
			else { list($title,$parr) = $arr; $te = ' &gt;'; }

			$output[] = '<label for="menu'.$a.'">';
			$output[] = '<tr style="cursor:hand" onmouseover="window.status=\''.$title.'\';return true" onmouseout="window.status=\'\';return true">';
			$output[] = '<td onmouseover="this.style.background=\''.$this->color('menu-bg-mouseover').'\'" onmouseout="this.style.background=\''.$this->color('menu-bg').'\'" id="menucell'.$a.'" style="background:'.$this->color('menu-bg').'" class="menu" valign="top"><a href="'.(isset($page)?$_SESSION['root_admin'].$page:"javascript:dosubmenu($a,".count($parr).")").'" class="menu" id="menu'.$a.'">&nbsp;'.$title.$te.'</a></td>';
			$output[] = '</tr>';
			$output[] = '</label>';

			if (isset($parr))
			{
				$output[] = '<script type="text/javascript">submenustatus[submenustatus.length]=\'none\';</script>';
				$output[] = '<tr id="submenu'.$a.'" style="height:'.((count($parr)*17)+1).'px;display:none">';
				$output[] = '<td bgcolor="'.$this->color('menu-bg-sub-around').'" valign="top">';
				$output[] = '<div style="position:absolute;left:10px" onmouseover="inmenu()" onmouseout="outmenu()">';
				$output[] = '<table border="0" cellspacing="0" cellpadding="0"><tr><td bgcolor="#FFFFFF">';
				$output[] = '<table border="0" cellspacing="1" cellpadding="0" width="100%">';
				while (list($b,list($smtitle,$url)) = each($parr))
				{
					$smtot++;

					$output[] = '<label for="submenuitem'.$smtot.'">';
					$output[] = '<tr style="cursor:hand;height:15px" onmouseover="window.status=\''.strip_tags($title).' &gt; '.strip_tags($smtitle).'\';return true" onmouseout="window.status=\'\';return true">';
					$output[] = '<td onmouseover="this.style.background=\''.$this->color('menu-bg-sub-mouseover').'\'" onmouseout="this.style.background=\''.$this->color('menu-bg-sub').'\'" id="menucell'.$a.'" width="100%" class="menu" bgcolor="'.$this->color('menu-bg-sub').'">&nbsp;<a href="'.$_SESSION['root_admin'].$url.'" class="menu" id="submenuitem'.$smtot.'">'.$smtitle.'</a>&nbsp;</td>';
					$output[] = '</tr>';
					$output[] = '</label>';
				}
				$output[] = '</table>';
				$output[] = '</td></tr></table>';
				$output[] = '</div>';
				$output[] = '</td>';
				$output[] = '</tr>';
			}
		}
		$output[] = '</table>';

		$concise = NO; // output the entire code in one document.write line?
		if (!$concise)
		{
			while (list($a,$line) = each($output)) { echo "document.write('".str_replace("'","\\'",$line)."');\n"; }
		}
		else { echo "document.write('".str_replace("'","\\'",implode("\\n",$output))."');"; }
	}

	/**
	* Prints the quick lookup form
	* @param integer $popup Open the popup window? If NO, the current window is used
	* @param string $textcolor Text color [optional, default "#FFFFFF"]
	* @param integer $onloadfocus Set focus to the UPC/Title field onload? [optional, default NO]
	* @param integer $frombare Is this from the bare item search page? [optional, default NO]
	*/
	function quick_lookup_form($popup=YES,$textcolor="#FFFFFF",$onloadfocus=NO,$frombare=NO)
	{
		$sto = new stores($this);
		$sto->setStores();
		$stores = $sto->getStores();

		if (!strlen(trim($frombare))) { $frombare = NO; }

		$pla = new platforms($this,0);
		$pla->set_item('platforms');
		?>
		<form id="quicklookupfrm" onsubmit="return item_search(this,<?=$popup;?>,<?=$frombare;?>)">
			<font color="<?=$textcolor;?>">
				<?php
				if ($popup)
				{
					?>
					<center><font size="1"><b>Q<u>uick Item Looku</u>p:</b></font></center>
					<img src="/images/blank.gif" width="1" height="5" /><br />
					<?php
				}
				?>
				<b>UPC/Title:</b> <input type="text" name="val" id="val" size="25" title="Enter a UPC or a title to view an item's information (you may enter partial titles to search for an item)">&nbsp;
				<b>Platform:</b> <select name="platformID" size="1" style="vertical-align:middle" title="Select a platform to include in your search"><option value=""></option><?php
					while (list($a,$arr) = each($pla->values)) { ?><option value="<?=$arr[0];?>"><?=$arr[1];?></option><?php }
				?></select><br />

				<b>Lookup:</b> <input type="radio" name="type" value="value" style="border-width:0px;vertical-align:middle" checked="checked" id="tv"> <label for="tv" title="Search for an item's price (this DOES NOT count on generated orders)">Price</label> <input type="radio" name="type" value="instock" style="border-width:0px;vertical-align:middle" id="tis"> <label for="tis" title="Search for an item's stock status (this DOES count on generated orders)">Stock Status</label>&nbsp;
				<input type="submit" value="Lookup &gt;" class="btn" style="vertical-align:middle;margin-top:5px">
			</font>
		</form>
		<?php

		if ($onloadfocus) { $this->addOnload("document.getElementById('quicklookupfrm').val.focus()"); }
	}

	/**
	* Prints a page header
	* @param string $txt The page header
	* @param integer $size Font size [optional, default "4"]
	*/
	function pageHead($txt,$size=4)
	{
		?>
		<font size="<?=$size;?>"><b><?=$txt;?></b></font><p />
		<?php
	}

	/**
	* Prints an outlined table header
	* @param integer $width Table width [optional, default '']
	* @param integer $height Table height [optional, default '']
	* @param boolean $single_line return all output on a single line? [optional, default NO]
	* @param string $id id='<id>' of the inner table [optional, default '']
	*/
	function outlineTableHead($width='',$height='',$single_line=NO,$id='')
	{
		if ($single_line) { ob_start(); }

		?>
		<table border="0" cellspacing="0" cellpadding="0"<?=(strlen($width)?' width="'.$width.'"':'');?><?=(strlen($height)?' height="'.$height.'"':'');?>><tr><td bgcolor="#999999">
			<table border="0" cellspacing="1" cellpadding="3" width="100%" height="100%"<?=(strlen($id)?' id="'.$id.'"':'');?>>
				<?php

		if ($single_line) { echo str_replace(array("\n","\t","\r\n"),'',ob_get_clean()); }
	}

	/**
	* Prints an outlined table footer
	* @param boolean $single_line return all output on a single line? [optional, default NO]
	*/
	function outlineTableFoot($single_line=NO)
	{
		if ($single_line) { ob_start(); }

				?>
			</table>
		</td></tr></table>
		<?php

		if ($single_line) { echo str_replace(array("\n","\t","\r\n"),'',ob_get_clean()); }
	}

	/**
	* Returns a color
	* @param string $type
	* @return string The color
	*/
	function color($type)
	{
		return @$this->colors[$type];
	}

	/**
	* Returns formatted phone numbers, zip codes, etc
	* @param string $type
	* @param string $val
	* @return string Formatted output
	*/
	function format($type,$val)
	{
		$return = "";

		if ($type == "phone")
		{
			// phone number
			if (strlen($val) == 11) { $return .= substr($val,0,1).'-'; $val = substr($val,1); }

			if (strlen($val) == 7) { $return .= substr($val,0,3).'-'.substr($val,3,4); }
			else { $return .= substr($val,0,3).'-'.substr($val,3,3).'-'.substr($val,6,4); }
		}
		elseif ($type == "zip")
		{
			// zip code
			$return = substr($val,0,5).'-'.substr($val,5);
		}
		else { $return = $val; }

		$return = trim($return,"-");

		return $return;
	}

	/**
	* Output an error report
	* @param string/array $error
	* @param string $system [optional, default '']
	*/
	function error($error,$system="")
	{
		if (!is_array($error)) { $error = array($error); }

		$this->outlineTableHead();
		while (list($a,$err) = each($error))
		{
			?>
			<tr>
				<td bgcolor="<?=$this->color('table-label');?>"><font color="red"><b><?=$system.(strlen($system)?' ':'');?>ERROR:</b></font></td>
				<td bgcolor="<?=$this->color('table-cell');?>"><?=$err;?></td>
			</tr>
			<?php
		}
		$this->outlineTableFoot();
		?><p /><?php
	}

	/**
	* Output a status report
	* @param string/array $status
	*/
	function status($status)
	{
		if (!is_array($status)) { $status = array($status); }

		$this->outlineTableHead();
		while (list($a,$sta) = each($status))
		{
			if (!strlen($sta)) { continue; }
			?>
			<tr>
				<td bgcolor="<?=$this->color('table-label');?>"><font color="red"><b>STATUS:</b></font></td>
				<td bgcolor="<?=$this->color('table-cell');?>"><?=$sta;?></td>
			</tr>
			<?php
		}
		$this->outlineTableFoot();
		?><p /><?php
	}

	/**
	* Add an onload event
	* @param string $event
	*/
	function addOnload($event)
	{
		$this->onload[] = $event;
	}

	/**
	* Display all onload events and bind the onload function (if needed)
	*/
	function showOnload()
	{
		ob_start();
		?>
		<script type="text/javascript">
			function doOnload()
			{
				<?php
				while (list($a,$ol) = each($this->onload))
				{
					echo "$ol;\n";
				}
				?>
			}
		</script>
		<?php
		echo str_replace("\t",'',ob_get_clean());
	}

	/**
	* Displays a cancel button that returns the user to the provided page
	* @param string $url
	* @param string $rt Tooltip - will read: "Return to the <value>" [optional, default '']
	*/
	function cancel($url,$rt="")
	{
		$rand = rand(0,100); // stick a random number at the end of the function, just in case there is already a function called 'cancel()'
		?>
		<script type="text/javascript">function cancel<?=$rand;?>() { if (confirm('Are you sure you would like to cancel?') == true) { document.location='<?=$url;?>'; } }</script>
		<div align="right" style="padding-right:5px;padding-bottom:1px"><input type="button" value="&lt; Cancel" class="btn" onclick="cancel<?=$rand;?>()"<?=(strlen($rt)?' title="Return to the '.$rt.'"':'');?>></div>
		<?php
	}

	/**
	* Returns millisecond time
	*/
	function micro_time()
	{
		$exp = explode(" ",microtime());
		return ((float)$exp[1]+(float)$exp[0]);
	}

	/**
	* Returns the time between $this->start and now
	* @param integer $places # of decimal places
	*/
	function elapsed($places=3)
	{
		$sub = ($this->micro_time()-$this->start);
		return sprintf("%0.${places}f",$sub);
	}

	/**
	* Determine if the current store/user has permissions for the current page
	*/
	function check_permissions()
	{
		// !!! CREATE PERMISSION SYSTEM !!!
		$only_logan = array('platforms.php','platform_source.php','priorities.php');
		$only_logan = array();
		$current = basename($_SERVER['PHP_SELF']);

		if (in_array($current,$only_logan) && $_SESSION['storeID'] != 1) { $this->error('You do not have permission to access this page'); $this->foot(); die(); }
	}

	/**
	* Output the "Updating..." page
	* @param string $text
	* @param string $redirect URL to redirect to [optional, default '']
	*/
	function showUpdating($text,$url="")
	{
		if (strlen($url))
		{
			// redirect them automatically (this way, they don't see the 'servlet' content)
			header("Location: $url");
			die();
		}

		$this->setFull(NO);

		$this->head($this->title);

		?><table border="0" cellspacing="0" cellpadding="0" width="100%" height="450"><tr><td align="center" valign="middle"><font size="3"><b><?=$text;?></b></font></td></tr></table><?php

		if (strlen($url))
		{
			// if the user's browser doesn't suppport 'Location:' redirecting, they will be redirected by this javascript
			?><script type="text/javascript">function gotourl() { document.location = "<?=$url;?>"; }</script><?php
			$this->addOnload("setTimeout('gotourl()',500)");
		}

		$this->foot();
	}

	/**
	* Return the number of days between $start and $end dates
	* @param integer $start
	* @param integer $end
	* @param integer $sundays count Sundays? [optional, default YES]
	* @return integer
	*/
	function daysBetween($start,$end,$sundays=YES)
	{
		$start = strtotime(date('m/d/Y',$start)); // midnight
		$end = strtotime(date('m/d/Y',$end)); // midnight

		$days = (($end-$start)/(60*60*24));

		if (!$sundays)
		{
			// remove Sundays
			for ($i=$start; $i<=$end; $i+=(60*60*24))
			{
				if (!date('w',$i)) { $days--; }
			}
		}

		return sprintf('%0.0f',$days);
	}
} // end class admin_page

/* END OF FILE */
/* Location: ./classes/admin_page.inc */