<?php
/**
* Point-of-sale class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class pos
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
	* Current customerID
	* @var integer $customerID
	*/
	var $customerID;

  /**
  * Status variable used for error reporting (bad email, bad zip code, success, etc)
  * @var array $status
  */
  var $status;

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	*/
	function pos(&$pg)
	{
		global $db;

		$this->pg =& $pg;
		$this->customerID = @$_SESSION['customerID'];
		$this->status = array();

		$this->error = new error('Point-of-Sale Class');

		if (!isset($_SESSION['cust_items']) || !is_array(@$_SESSION['cust_items'])) { $_SESSION['cust_items'] = array(); }

		// check for any active invoices
		/* DON'T DO THIS - IT STOPS MULTIPLE COMPUTERS FROM OPENING DIFFERENT CUSTOMERS/INVOICES!
		if (!@$_SESSION['cust_invoiceID'] && !@$_SESSION['cust_checkedactive'])
		{
			$sql = "SELECT inv_customerID,inv_invoiceID FROM invoices WHERE inv_storeID=".$_SESSION['storeID']." AND inv_active=".YES;
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			if (mysql_num_rows($result))
			{
				$row = mysql_fetch_assoc($result);

				$cust = new customers($this->pg);
				$cust->set_customerID($row['inv_customerID']);

				$inv = new invoice($this->pg);
				$inv->set_invoiceID($row['inv_invoiceID']);

				$changeloc = YES;
				if (basename($_SERVER['PHP_SELF']) == 'pos.php' && @$_GET['act'] == "new") { $changeloc = NO; }

				if ($changeloc == YES)
				{
					$this->pg->addOnload("document.location='{$_SESSION['root_admin']}pos/invoice.php?alert=1'");
				}
			}

			$_SESSION['cust_checkedactive'] = YES;
		}
		*/
	}

	/**
	* Displays the appropriate point-of-sale menu
	*/
	function menu()
	{
		if ($this->customerID)
		{
			$menu = array(
				'Customer History'   => 'pos/custhistory.php',
				'Edit Customer Info' => "pos/customers.php?act=edit&customerID={$_SESSION['customerID']}",
				'Close Customer'     => 'pos/pos.php?act=new'
			);
			if (@$_SESSION['cust_invoiceID'])
			{
				$append = array(
					"Edit Current Invoice" => 'pos/invoice.php?act=view'
				);
			}
			else
			{
				$append = array(
					'New Invoice' => 'pos/invoice.php?act=new'
				);
			}
			$append = array_merge(
				array('Customer Info' => 'pos/pos.php'),
				$append
			);
			$menu = array_merge($append,$menu);
		}
		else
		{
			$menu = array(
				'Select Customer' => 'pos/pos.php',
				//'Reports'         => 'pos/reports.php',
				//'Invoice History' => 'pos/history.php'
			);
		}

		?><div style="padding:5px"><?php
		$this->pg->outlineTableHead();

		?>
		<tr>
			<?php
			if ($this->customerID)
			{
				$words = '<b>Current Customer:</b><br />' . $_SESSION['cust_info']['fname'] . ' ' . $_SESSION['cust_info']['lname'];

				// determine if the customer is in the top 500
				$rnk = new rankings($this->pg);
				$top500 = $rnk->inTop500($this->customerID);
				if ($top500)
				{
					$words .= '<br /><font color="red"><b>&raquo; TOP 500 [#' . $top500 . ']';

					// do they have an address on file?
					$info = $_SESSION['cust_info'];
					if (!strlen($info['address']) || !strlen($info['city']) || !strlen($info['state']) || !strlen($info['zip']))
					{
						$words .= ' - NO ADDRESS!';
					}

					$words .= '</b></font>';
				}
			}
			else
			{
				$words = '<b>Point of Sale:</b>';
			}
			?>
			<td bgcolor="#DDDDDD">
				<?php echo $words;?>
			</td>
			<?php

			$i = 0;
			while (list($title,$url) = each($menu))
			{
				$i++;
				?>
				<label for="lnk<?php echo $i;?>">
					<td bgcolor="<?php echo $this->pg->color('menu-bg-pos');?>" style="cursor:hand" onmouseover="this.style.background='<?php echo $this->pg->color('menu-bg-pos-mouseover');?>'" onmouseout="this.style.background='<?php echo $this->pg->color('menu-bg-pos');?>'">
						<a href="<?php echo $_SESSION['root_admin'] . $url;?>" id="lnk<?php echo $i;?>"><?php echo $title;?></a>
					</td>
				</label>
				<?php
			}
			?>
		</tr>
		<?php

		$this->pg->outlineTableFoot();
		?></div><?php

		// if necessary, popup the contact-the-wishlist-customer window
		if (isset($_SESSION['invoice_wishlist_itemIDs']))
		{
			?>
			<script type="text/javascript">
				function contact_window()
				{
					alert('Some items from the last invoice were found in wishlists. A window will now be opened listing these items.');
					open_window('/admin/pos/wishlist_contact.php','wlprint',750,450,'YES',true);
				}
			</script>
			<?php
			$this->pg->addOnload('contact_window()');
		}
	}
}

?>