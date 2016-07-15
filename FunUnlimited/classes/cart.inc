<?php
/**
* Shopping cart class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class cart
{
	/**
	* page class reference
	* @var reference $pg
	*/
	var $pg;

	/**
	* Cart item details
	* @var array $items
	*/
	var $items;

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
	function cart(&$pg)
	{
		$this->timer =& $GLOBALS['timer_class'];
		$this->pg =& $pg;
		$this->customerID = @$_SESSION['store_customerID'];
		$this->status = array();
		$this->items = array();

		$this->error = new error('Shopping Cart class');
	}

	/**
	* Checks if there is currently an open cart; if not, create it
	* @access public
	*/
	function checkCart()
	{
		if (!isset($_SESSION['cart_items']))
		{
			$_SESSION['cart_items'] = array(
				ITEM_NEW  => array(),
				ITEM_USED => array()
			);
			$_SESSION['cart_qty'] = 0;
			$_SESSION['cart_total'] = 0;
		}
	}

	/**
	* Add an item to the shopping cart
	* @param integer $newused
	* @param integer $itemID
	* @access public
	*/
	function addItem($newused,$itemID)
	{
		if (in_array($newused,array(ITEM_NEW,ITEM_USED)) && $itemID)
		{
			@$_SESSION['cart_items'][$newused][$itemID] += 1;

			// we must pass a variable because it's passed by reference
			$blank = -1;
			$this->_updateSessionTotals($blank);
		}
	}

	/**
	* Remove an item from the shopping cart
	* @param integer $newused
	* @param integer $itemID
	* @access public
	*/
	function removeItem($newused,$itemID)
	{
		if (isset($_SESSION['cart_items'][$newused][$itemID]))
		{
			unset($_SESSION['cart_items'][$newused][$itemID]);

			$blank = -1;
			$this->_updateSessionTotals($blank);
		}
	}

	/**
	* Update the session totals
	* @param array &$items
	* @access private
	*/
	function _updateSessionTotals(&$items)
	{
		$this->timer->startTimer(__CLASS__.'::'.__FUNCTION__);
		if ($items == -1)
		{
			$this->setCartItems();
			$items = $this->getCartItems();
		}

		$total_qty = 0;
		$total_amount = 0;

		while (list($nu,$nuitems) = each($items))
		{
			while (list($itemID,$arr) = each($nuitems))
			{
				$total_qty += $arr['cart_qty'];
				$total_amount += ($arr['cart_qty'] * $arr['price'][$nu]);
			}
		}

		$_SESSION['cart_qty'] = $total_qty;
		$_SESSION['cart_total'] = $total_amount;
		$this->timer->stopTimer(__CLASS__.'::'.__FUNCTION__);
	}

	/**
	* Pull the item information for the cart items
	* @access public
	*/
	function setCartItems()
	{
		$this->timer->startTimer(__CLASS__.'::'.__FUNCTION__);
		$this->items = array(
			ITEM_NEW  => array(),
			ITEM_USED => array()
		);

		if (!isset($_SESSION['cart_qty_note'])) { $_SESSION['cart_qty_note'] = NO; }
		$items = $_SESSION['cart_items'];
		$itemIDs = array();
		while (list($nu,$nuitemIDs) = each($items))
		{
			$itemIDs = array_merge($itemIDs,array_keys($nuitemIDs));
		}
		reset($items);

		if (count($itemIDs))
		{
			$sort = new sort();
			$ii = new items($this->pg);
			$ii->setItems($itemIDs);
			$item_info = $ii->getItems();

			$update_session = NO;

			while (list($nu,$nuitemIDs) = each($items))
			{
				while (list($itemID,$qty) = each($nuitemIDs))
				{
					$info = $item_info[$itemID];

					if ($qty > $info['quantity'][$nu])
					{
						$qty = $info['quantity'][$nu];
						$_SESSION['cart_items'][$nu][$itemID] = $qty;
						$update_session = YES;

						$_SESSION['cart_qty_note'] = YES;
					}

					$info['cart_each'] = $info['price'][$nu];
					$info['cart_qty'] = $qty;
					$info['cart_total'] = ($qty * $info['price'][$nu]);
					$this->items[$nu][$itemID] = $info;
				}

				if (count($this->items[$nu]))
				{
					$this->items[$nu] = $sort->doSort($this->items[$nu],'itm_title',SORT_ASC);
					$sort->fixArrayKeys($this->items[$nu],'itm_itemID');
				}
			}

			if ($update_session)
			{
				$this->_updateSessionTotals($this->items);
			}
		}
		$this->timer->stopTimer(__CLASS__.'::'.__FUNCTION__);
	}

	/**
	* Return the shopping cart item info
	* @return array
	* @access public
	*/
	function getCartItems()
	{
		return $this->items;
	}

	/**
	* Return the total number/amount in the cart
	* @return array array(#,$)
	*/
	function getTotal()
	{
		$num = 0;
		$total = 0;

		while (list($a,$arr) = each($_SESSION['cart_items']))
		{
			$num += $arr['qty'];
			$total += ($arr['price']*$arr['qty']);
		}
		reset($_SESSION['cart_items']);

		return array($num,$total);
	}

	/**
	* Update the shopping cart quantities and recalculate the totals
	* @param array &$qtys
	* @access public
	*/
	function recalculateCartItems(&$qtys)
	{
		$this->timer->startTimer(__CLASS__.'::'.__FUNCTION__);
		$cart = $_SESSION['cart_items'];
		$update_session = NO;

		while (list($nu,$nuitems) = each($qtys))
		{
			while (list($itemID,$qty) = each($nuitems))
			{
				$cart_qty = $cart[$nu][$itemID];
				if (is_numeric($qty) && $qty != $cart_qty)
				{
					if ($qty <= 0)
					{
						// remove the item from the cart
						unset($_SESSION['cart_items'][$nu][$itemID]);
					}
					else
					{
						// update the quantity
						$_SESSION['cart_items'][$nu][$itemID] = floor($qty);
					}
					$update_session = YES;
				}
			}
		}

		if ($update_session)
		{
			// update the session totals
			$blank = -1;
			$this->_updateSessionTotals($blank);
		}
		$this->timer->stopTimer(__CLASS__.'::'.__FUNCTION__);
	}

	/**
	* Save the selected payment method
	* @param integer $cardID
	* @access public
	*/
	function setPaymentMethod($cardID)
	{
		$_SESSION['cart_cardID'] = $cardID;
	}

	/**
	* Process the order (run the credit card, create the invoice, add the items, etc)
	* @return boolean
	* @access public
	*/
	function processOrder()
	{
		// validate the request
		$cardID = @$_SESSION['cart_cardID'];
		if (!$cardID) { headerLocation('/cart.php'); }
		if (!$_SESSION['cart_qty']) { headerLocation('/cart.php'); }
		$total = $_SESSION['cart_total'];

		// process the payment
		// charge the selected credit card
		$cc = new credit_card($pg);
		$cc->setCards($cardID);
		$card_info = $cc->getCards($cardID);

		$payment_data = array(
			'first_name'           => $card_info['crc_fname'],
			'last_name'            => $card_info['crc_lname'],
			'company'              => $card_info['crc_companyname'],
			'address'              => $card_info['crc_address'],
			'city'                 => $card_info['crc_city'],
			'state'                => $card_info['crc_state'],
			'zip'                  => $card_info['crc_zip'],
			'phone'                => $card_info['crc_phone'],
			'card_number'          => $card_info['crc_number'],
			'card_expiration_date' => str_pad($card_info['crc_expmonth'],2,'0',STR_PAD_LEFT) . '-' . $card_info['crc_expyear'],
			'customerID'           => $_SESSION['store_customerID'],
			'customer_ip'          => $_SERVER['REMOTE_ADDR'],
			'customer_email'       => $_SESSION['userinfo']['cus_email'],
			'store_email'          => 'scott@payforstay.com',
			'description'          => 'Online Payment - FunUnlimitedOnline.com Order',
			'amount'               => $_SESSION['cart_total'],
			'cardID'               => $cardID
		);

		$pay = new payment($pg);
		$pay->setData($payment_data);
		$pay->submitPayment();

		if ($pay->paymentSuccess())
		{
			// get the items
			$this->setCartItems();
			$items = $this->getCartItems();

			/*
			?><div align="left"><pre><?php print_r($items); ?></pre></div><?php
			die();
			*/

			return YES;
		}
		else
		{
			list($code,$text) = $pay->getError();
			$text = str_replace('. ','.<br />',$text) . " ($code)";
			$_SESSION['payment_error'] = $text;

			return NO;
		}
	}
}
?>