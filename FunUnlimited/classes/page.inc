<?php
/**
* Website page display class (header, footer, background colors, etc)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class page
{
	/**
	* Shopping cart class reference
	* @var reference $cart
	*/
	var $cart;

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
	* Show the 'BROWSE' menu?
	* @var boolean $show_browse
	*/
	var $show_browse;

	/**
	* Output the full header? (images, menu, etc)
	* @var boolean $full
	*/
	var $full;

	/**
	* Center the page contents?
	* @var boolean $center
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
	function page()
	{
		global $db;

		$this->timer =& $GLOBALS['timer_class'];
		$this->start = $_SESSION['start_time'];

		$_SESSION['storeID'] = 1; // default to the Logan Store

		$this->colors = array(
			'table-head'            => '#CFCFCF', // table header background
			'table-head-darker'     => '#BFBFBF', // darker table header background
			'table-head-lighter'    => '#DFDFDF', // lighter table header background
			'table-label'           => '#E8E8E8', // table label (IE: for tables with left column = label, right column = textbox)
			'table-cell'            => '#FFFFFF', // table cell background (single cell or the first cell of alternating color cells)
			'table-cell2'           => '#FFFFFF', // table cell background (second cell of alternating color cells)
			'outline'               => '#FFFFFF', // outline table border color
			//'table-label'           => '#CFCFCF', // table label (IE: for tables with left column = label, right column = textbox)
			//'table-cell'            => '#E7E7E7', // table cell background (single cell or the first cell of alternating color cells)
			//'table-cell2'           => '#EEEEEE', // table cell background (second cell of alternating color cells)
			//'outline'               => '#999999', // outline table border color
			'body'                  => '#F0F0F0', // body background color
			'text'                  => '#000000', // text color
			'link'                  => '#1E86DD', // link color (non-hover/visited)
			'link-hover'            => '#1E86DD', // link color (hover)
			'head-bg'               => '#0064B9', // header background
			'menu-bg'               => '#0064B9', // menu background color
			'menu-bg-mouseover'     => '#0073D4', // menu background color (mouseover)
			'menu-bg-sub'           => '#999999', // submenu background color
			'menu-bg-sub-mouseover' => '#AAAAAA', // submenu background color
			'menu-bg-sub-around'    => '#DDDDDD', // "around the submenu" background color
			'menu-bg-pos'           => '#DDDDDD', // point-of-sale menu background color
			'menu-bg-pos-mouseover' => '#E5E5E5', // point-of-sale menu background color (mouseover)
			'menu-text'             => '#FFFFFF', // menu text
			'menu-link'             => '#FFFFFF', // menu link color (non-hover/visited)
			'menu-link-hover'       => '#FFFFFF' // menu link color (hover)
		);
		$this->onload = array();
		$this->setFull(YES);
		$this->setCenter(YES);
		$this->setShowBrowse(YES);

		// make sure a shopping cart exists
		$cart = new cart($this);
		$cart->checkCart();

		$this->error = new error('Page');

		// if it's 2am-2:30am, show the maintenance message
		if (PRODUCTION && date('G') == 2)
		{
			$this->setShowBrowse(NO);
			$this->setTitle('Daily Maintenance');
			$this->head("We're Sorry!");
			?>
			We're sorry, but we are undergoing daily maintenance.
			<p />
			This maintenance is scheduled each morning from 2:00am to 3:00am.
			<p />
			Right now, the time is <?=date('h:ia');?>.
			<?php
			$this->foot();
			die();
		}
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
	* Set $this->show_browse
	* @param boolean $show_browse
	* @access public
	*/
	function setShowBrowse($show_browse) { $this->show_browse = $show_browse; }

	/**
	* Return $this->show_browse
	* @return boolean
	* @access private
	*/
	function _getShowBrowse() { return $this->show_browse; }

	/**
	* Prints the page header
	* @param string $pagehead Page header [optional, default '']
	* @param integer $posmenu Output the point-of-sale menu? [optional, default NO]
	*/
	function head($pagehead='',$posmenu=NO)
	{
		$title = $this->_getTitle();
		$full = $this->_getFull();
		$center = $this->_getCenter();
		$show_browse = $this->_getShowBrowse();

		?>
		<html>
		<head>
			<title><?=(strlen($title) ? "$title - " : '');?>Fun Unlimited Online</title>
			<link rel="stylesheet" type="text/css" href="/styles/global.css">
			<link rel="stylesheet" type="text/css" href="/styles/store.css">
			<script type="text/javascript" src="/scripts/functions.js"></script>
		</head>
		<body background="/images/storebg.gif" bgcolor="<?=$this->color('body');?>" text="<?=$this->color('text');?>" topmargin="0" leftmargin="0" rightmargin="0" bottommargin="0" marginheight="0" marginwidth="0" onload="doOnload()">
			<img src="/images/blank.gif" width="1" height="5" /><br />
			<center>
			<?php
			if ($full)
			{
				?>
				<table border="0" cellspacing="0" cellpadding="0" width="760">
					<tr>
						<td><img src="/images/corner_tl.gif" width="10" height="10" /></td>
						<td width="100%" bgcolor="#E0E0E0" style="border-top:solid 1px #000000"><img src="/images/blank.gif" width="1" height="1" /></td>
						<td><img src="/images/corner_tr.gif" width="10" height="10" /></td>
					</tr>
				</table>
				<?php
			}
			?>
			<table border="0" cellspacing="0" cellpadding="0" width="<?=($full ? 760 : '100%');?>"><tr><td align="center" bgcolor="#FFFFFF">
				<table border="0" cellspacing="0" cellpadding="0" width="100%">
					<?php
					if ($full)
					{
						?>
						<tr bgcolor="<?=$this->color('body');?>">
							<td background="images/head_left.gif" width="100%" height="82" align="right" valign="middle" onclick="go('/index.php')" class="header">
								<?php
								if (!@$_SESSION['store_loggedin'])
								{
									?>
									Guest |
									<a href="login.php" class="graylink">Log In</a> |
									<a href="login.php?action=register" class="graylink">Register</a>
									<?php
								}
								else
								{
									?>
									<?=$_SESSION['store_name'];?> |
									<a href="logout.php" class="graylink">Log Out</a>
									<?php
								}
								?>
							</td>
							<td width="11"><img src="images/head_right.gif" width="11" height="82" /></a></td>
						</tr>
						<tr>
							<td height="20" colspan="3" bgcolor="#A8A8A8">
								<script type="text/javascript">
									function searchverify(frm)
									{
										if (!frm.text.value.length) { alert('Please enter a search value.'); frm.text.focus(); return false; }
										else { return true; }
									}
								</script>
								<table border="0" cellspacing="0" cellpadding="0" width="100%" height="20">
									<tr>
										<td width="1"><img src="images/blank_black.gif" width="1" height="20" /></td>
										<form method="get" action="search.php" onsubmit="return searchverify(this)">
										<input type="hidden" name="act" value="search">
										<input type="hidden" name="search_type" value="<?=ITEM_SEARCH_SIMPLE;?>">
										<td width="100%" valign="middle">
											&nbsp;&nbsp;&nbsp;
											<img src="images/searchby.gif" width="85" height="20" align="middle" />
											<select name="searchby" size="1">
												<?php
												$options = array(
													'title'      => 'Title',
													'platform'   => 'Platform',
													'company'    => 'Company',
													'type'       => 'Type (IE: romance, sports)',
													'lessprice'  => 'Price Less Than',
													'moreprice'  => 'Price Greater Than',
													'pricerange' => 'Price Range (IE: 10-14)',
													'age'        => 'Age (IE: 6, 10-12, 14+)',
													'year'       => 'Year'
												);
												while (list($val,$show) = each($options))
												{
													?><option value="<?=$val;?>"><?=$show;?></option><?php
												}
												?>
											</select>
											&nbsp;&nbsp;
											<input type="text" name="text" size="30" />
											<input type="image" src="images/searchgo.gif" width="56" height="20" border="0" class="nb" align="middle" />
											<font color="#FFFFFF">[<a href="search.php" class="mainmenu">Advanced Search</a>]</font>
										</td>
										</form>
										<td width="1"><img src="images/blank_black.gif" width="1" height="20" /></td>
									</tr>
								</table>
							</td>
						</tr>
						<tr><td height="1" colspan="3" bgcolor="#000000"><img src="images/blank_black.gif" width="100%" height="1" /></td></tr>
						<tr>
							<td colspan="3" style="border-left:solid 1px black;border-right:solid 1px black">
								<table border="0" cellspacing="0" cellpadding="0" width="100%">
									<tr>
										<td><?=$this->nav_menu();?></td>
										<td align="right">
											<font color="#555555">
												<?php
												?>
												<b>Items In Cart:</b> <?=$_SESSION['cart_qty'];?> / $<?=number_format($_SESSION['cart_total'],2);?> |
												<a href="cart.php" class="graylink">View Cart</a>
												&nbsp;
											</font>
										</td>
									</tr>
								</table>
							</td>
						</tr>
						<?php
					}
					?>
				</table>
				<?php
				if ($full)
				{
					$show_browse = $this->_getShowBrowse();

					?>
					<table border="0" cellspacing="0" cellpadding="0">
						<tr class="content_table">
							<?php
							if ($show_browse)
							{
								?>
								<td valign="top" style="border-left:solid 1px black">
									&nbsp;<br />
									<table border="0" cellspacing="0" cellpadding="0" width="165" class="left_menu">
										<tr>
											<td background="/images/leftmenu_head.jpg" class="left_menu_top">BROWSE</td>
										</tr>
										<tr>
											<td class="left_menu_content">
												<?php
												$left_menu = array(
													'Catalog' => array(
														'Search Catalog'     => '/search.php',
														'Browse by Platform' => '/catalog.php'
													),
													'Shopping Cart' => array(
														'View Shopping Cart' => '/cart.php',
														'Check Out'          => '/checkout.php'
													),
													'My Account' => array(
														'Order History'       => '/history.php',
														'Account Information' => '/account.php',
														'Payment Methods'     => '/pay_methods.php'
													),
												);

												$i = 0;
												while (list($section,$links) = each($left_menu))
												{
													$i++;
													?>
													<span class="left_menu_section"><?=$section;?></span><br />
													<?php
													while (list($text,$url) = each($links))
													{
														?>
														&raquo; <a href="<?=$url;?>" class="left_menu_link"><?=$text;?></a><br />
														<?php
													}

													if ($i < count($left_menu))
													{
														?><p /><?php
													}
												}
												?>
											</td>
										</tr>
									</table>
								</td>
								<?php
							}
							?>
							<td width="760" valign="top" style="border-right:solid 1px black<?=(!$show_browse ? ';border-left:solid 1px black' : '');?>">
								<?php
				}
				?>

				<?=($center ? '<center>' : '');?>
				<?php
				if (strlen($pagehead)) { $this->pageHead($pagehead); }
	}

	/**
	* Prints the page footer
	*/
	function foot()
	{
		global $db;

		$full = $this->_getFull();
		$center = $this->_getCenter();

			if ($center) { echo '</center>'; }

			if ($full)
			{
							?><br />
						</td>
					</tr>
					<tr>
						<td colspan="2" align="center" class="footer">
							<table border="0" cellspacing="0" cellpadding="0" width="99%">
								<tr>
									<td valign="top" class="pagebottom" width="45%">Copyright &copy; 2004-<?php echo date('Y');?> Fun Unlimited</td>
									<td valign="top" align="center" class="pagebottom" width="10%">
										<a href="/policy.php">Return Policy</a> |
										<a href="/policy.php">Privacy Policy</a>
									</td>
									<td align="right" valign="top" class="pagebottom" width="45%">
										Website by <a href="mailto:s-carp@comcast.net">.evolve Internet Inc.</a><br />
										<?=date('l, F d, Y');?> : <?=$this->getElapsed();?>
									</td>
								</tr>
							</table>
						</td>
					</tr>
				</table>
				<?php
			}
			?>
			<?=$this->showOnload();?>
			<!-- execution time: <?=$this->getElapsed();?> -->
			</table>
			<?php
			if ($full)
			{
				?>
				<table border="0" cellspacing="0" cellpadding="0" width="760">
					<tr>
						<td><img src="/images/corner_bl.gif" width="10" height="10" /></td>
						<td width="100%" bgcolor="#E0E0E0" style="border-bottom:solid 1px #000000"><img src="/images/blank.gif" width="1" height="1" /></td>
						<td><img src="/images/corner_br.gif" width="10" height="10" /></td>
					</tr>
				</table>
				<?php
			}
			?>

			<p />
			<?php
			$this->timer->stopTimer('entire_page');
			$this->timer->viewTimes();
			?>
			</center>
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
		$loggedin = @$_SESSION['store_loggedin'];
		$usertype = @$_SESSION['usertype'];

		$menu = array(
			'catalog'  => 'catalog.php',
			//'specials' => 'specials.php',
			'cart'     => 'cart.php',
			'checkout' => 'checkout.php'
		);

		while (list($img,$url) = each($menu))
		{
			?><a href="<?=$url;?>"><img src="images/menu_<?=$img;?>.gif" border="0" /></a><?php
		}
	}

	/**
	* Prints a page header
	* @param string $txt The page header
	* @param integer $size Font size [optional, default "4"]
	*/
	function pageHead($txt,$size=4)
	{
		?><font size="<?=$size;?>"><b><?=$txt;?></b></font><p /><?php
	}

	/**
	* Prints an outlined table header
	* @param integer $width Table width [optional, default '']
	*/
	function outlineTableHead($width='')
	{
		?>
		<table border="0" cellspacing="1" cellpadding="3"<?=(strlen($width) ? ' width="' . $width . '"' : '');?>>
		<?php
	}

	/**
	* Prints an outlined table footer
	*/
	function outlineTableFoot()
	{
		?>
		</table>
		<?php
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
		$return = '';

		$vald = new validate();

		if ($type == "phone")
		{
			// phone number
			$val = $vald->strip($val);
			if (strlen($val) == 11) { $return .= substr($val,0,1).'-'; $val = substr($val,1); }
			elseif (strlen($val) == 7) { $return .= substr($val,0,3).'-'.substr($val,3,4); }
			else { $return .= substr($val,0,3).'-'.substr($val,3,3).'-'.substr($val,6,4); }
		}
		elseif ($type == "zip")
		{
			// zip code
			$val = $vald->strip($val);
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
	function error($error,$system='')
	{
		if (!is_array($error)) { $error = array($error); }

		$this->outlineTableHead();
		?>
		<tr>
			<td valign="top"><img src="/images/error_arrow.gif" width="8" height="12" /></td>
			<td valign="top"><font color="red"><b><u><?=$system . (strlen($system) ? ' ' : '');?>ERROR:</u></b></font></td>
			<td bgcolor="<?=$this->color('table-cell');?>"><b><?=implode('<br />',$error);?></b></td>
		</tr>
		<?php
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
		?>
		<tr>
			<td valign="top"><img src="/images/error_arrow.gif" width="8" height="12" /></td>
			<td valign="top"><font color="red"><b><u>STATUS:</u></b></font></td>
			<td bgcolor="<?=$this->color('table-cell');?>"><b><?=implode('<br />',$status);?></b></td>
		</tr>
		<?php
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
		?>
		<script type="text/javascript">
			function doOnload()
			{
				<?php
				while (list($a,$ol) = each($this->onload))
				{
					echo "$ol;";
				}
				?>
			}
		</script>
		<?php
	}

	/**
	* Displays a cancel button that returns the user to the provided page
	* @param string $url
	* @param string $rt Tooltip - will read: "Return to the <value>" [optional, default '']
	*/
	function cancel($url,$rt='')
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
	function getElapsed($places=3)
	{
		$sub = ($this->micro_time()-$this->start);
		return sprintf("%0.${places}f",$sub) . 's';
	}

	/**
	* Output the "Updating..." page
	* @param string $text
	* @param string $redirect URL to redirect to [optional, default '']
	*/
	function showUpdating($text,$url='')
	{
		if (strlen($url))
		{
			// redirect them automatically (this way, they don't see the 'servlet' content)
			@header("Location: $url");
			die();
		}

		$this->setFull(NO);
		$this->setTitle($text);

		$this->head();

		?><table border="0" cellspacing="0" cellpadding="0" width="100%" height="75%"><tr><td align="center" valign="middle"><font size="3"><b><?=$text;?></b></font></td></tr></table><?php

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

		return $days;
	}
}

?>