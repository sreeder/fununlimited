<?php
/**
* Credit card class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class credit_card
{
	/**
	* page class reference
	* @var object $pg
	*/
	var $pg;

	/**
	* Credit card types and their settings
	* @var array $types
	*/
	var $types;

	/**
	* Credit cards
	* @var array $cards
	*/
	var $cards;

	/**
	* Credit card information
	* @var array $info
	*/
	var $info;

	/**
	* Is the card valid?
	* @var integer $valid
	*/
	var $valid;

	/**
	* Key
	* @var string $key
	*/
	var $key;

	/**
	* Non-encoded database fields
	* @var array $non_encoded
	*/
	var $not_encoded;

	/**
	* Error reporting class
	* @var object $error
	*/
	var $error;

	/**
	* Class constructor
	* Initializes variables
	* @param reference &$pg The $pg declaration from the calling page
	* @access private
	*/
	function credit_card(&$pg)
	{
		$this->pg =& $pg;
		$this->key = 'FuNUnlimIt3d_CRca_ENCoDe-2839431857'; // long and nasty enough? ;)
		$this->not_encoded = array('cardID','customerID','typeID','active');

		$this->error = new error('Credit card class');
	}

	/**
	* Return the key
	* @return string
	* @access private
	*/
	function _getKey() { return $this->key; }

	/**
	* Retrieve credit cards for the current/given customer
	* @param mixed $cardIDs array of cardIDs (can be array or single cardID) [optional, default array()]
	* @param integer $customerID customerID to pull the credit card(s) for [optional, default 0]
	* @access public
	*/
	function setCards($cardIDs=array(),$customerID=0)
	{
		global $db;

		if (!$customerID) { $customerID = @$_SESSION['store_customerID']; }
		$key = $this->_getKey();

		$this->cards = array();
		if (!is_array($cardIDs) && strlen($cardIDs)) { $cardIDs = array($cardIDs); }

		if (count($cardIDs)) { $cardwhere = ' AND crc_cardID IN ' . getIn($cardIDs); }
		else { $cardwhere = ''; }

		$prefix = 'crc_';
		$decode_fields = array();
		$sql = 'DESC credit_cards';
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_row($result))
		{
			$field = substr($row[0],4);
			if (!in_array($field,$this->not_encoded)) { $decode_fields[] = $prefix . $field; }
		}

		$sql = "SELECT * FROM credit_cards,credit_card_types WHERE crc_customerID=$customerID AND crc_active=" . YES . ' AND crc_typeID=cct_typeID' . $cardwhere . ' ORDER BY crc_expyear,crc_expmonth';
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result))
		{
			// get the decoded fields
			$vals = array();
			while (list($k,$v) = each($row))
			{
				if (in_array($k,$decode_fields)) { $vals[] = "DECODE('" . mysql_real_escape_string($v) . "','$key!" . substr($k,4) . "') AS $k"; }
			}
			$dsql = 'SELECT ' . implode(',',$vals);
			$dresult = mysql_query($dsql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			while ($drow = mysql_fetch_assoc($dresult))
			{
				while (list($k,$v) = each($drow)) { $row[$k] = $v; }
			}

			$this->cards[$row['crc_cardID']] = $row;
		}
	}

	/**
	* Retrieve one/all credit cards
	* @param integer $cardID ID of the credit card to return [optional, default '']
	* @return array
	* @access public
	*/
	function getCards($cardID=0)
	{
		if ($cardID) { return (isset($this->cards[$cardID]) ? $this->cards[$cardID] : array()); }
		else { return $this->cards; }
	}

	/**
	* Add a credit card to the database
	* @return boolean NO if it wasn't added, otherwise the added cardID
	* @access public
	*/
	function addCard()
	{
		global $db;

		$added = NO;
		$_SESSION['cc_post_info'] = getP();
		$cardID = getP('cardID');
		$key = $this->_getKey();
		$this->info = getP();

		$this->_verifyCard();

		if ($this->_isValid())
		{
			// insert the credit card into the database
			unset($this->info['act']);
			if (!$cardID) { unset($this->info['cardID']); }
			$this->info['customerID'] = $_SESSION['store_customerID'];
			$this->info['active'] = YES;

			$vals = array();
			while (list($k,$v) = each($this->info))
			{
				if (in_array($k,$this->not_encoded) || !strlen($v)) { $vals[] = "crc_$k='" . mysql_real_escape_string($v) . "'"; }
				else { $vals[] = "crc_$k=ENCODE('" . mysql_real_escape_string($v) . "','$key!$k')"; }
			}
			reset($this->info);

			$sql = ($cardID ? 'UPDATE' : 'INSERT INTO') . ' credit_cards SET ' . implode(',',$vals);
			if ($cardID) { $sql .= " WHERE crc_cardID=$cardID AND crc_customerID={$_SESSION['store_customerID']}"; }
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);

			$added = ($cardID ? $cardID : mysql_insert_id());
		}

		return $added;
	}

	/**
	* Deletes the given credit card (sets it as inactive - it will be deleted by /cron/clean_credit_cards.php)
	* @param integer $cardID
	* @access public
	*/
	function deleteCard($cardID)
	{
		global $db;

		if ($cardID)
		{
			// see if the card has been involved in any transactions
			$sql = "SELECT COUNT(*) AS count FROM invoices WHERE inv_cardID=$cardID";
			$result = mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			$row = mysql_fetch_assoc($result);

			if ($row['count'])
			{
				// it has been involved in transactions - set it as inactive
				$sql = 'UPDATE credit_cards SET crc_active=' . NO . " WHERE crc_cardID=$cardID AND crc_customerID={$_SESSION['store_customerID']}";
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}
			else
			{
				// it hasn't been involved in transactions - delete it
				$sql = "DELETE FROM credit_cards WHERE crc_cardID=$cardID AND crc_customerID={$_SESSION['store_customerID']}";
				mysql_query($sql,$db);
				$this->error->mysql(__FILE__,__LINE__);
			}
		}
	}

	/**
	* Pull in the credit card types
	* @access public
	*/
	function setTypes()
	{
		global $db;

		$this->types = array();
		$sql = 'SELECT * FROM credit_card_types ORDER BY cct_typeID';
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result))
		{
			$row['cct_prefixes'] = explode('|',$row['cct_prefixes']);
			$row['cct_lengths'] = explode('|',$row['cct_lengths']);
			$this->types[$row['cct_typeID']] = $row;
		}
	}

	/**
	* Return the settings for a credit card type
	* @param integer $typeID [optional, default 0]
	* @return array
	* @access public
	*/
	function getTypes($typeID=0)
	{
		if ($typeID) { return (isset($this->types[$typeID]) ? $this->types[$typeID] : array()); }
		else { return $this->types; }
	}

	/**
	* Return the typeID for the given credit card number
	* @param string $number
	* @return integer
	* @access public
	*/
	function getTypeID($number)
	{
		$this->setTypes();
		$types = $this->getTypes();

		while (list($typeID,$arr) = each($types))
		{
			while (list($b,$prefix) = each($arr['cct_prefixes']))
			{
				if (substr($number,0,strlen($prefix)) == $prefix) { return $typeID; }
			}
		}

		return -1;
	}

	/**
	* Luhn Formula for validating card numbers (mod10 algorithm)
	* http://www.beachnet.com/~hstiles/cardtype.html
	* @param string $cardnum
	* @return integer
	* @access private
	*/
	function _mod10($cardnum)
	{
		for ($sum=0,$j=0,$i=strlen($cardnum)-1; $i>=0; $i--,$j++)
		{
			if (($j%2)) { $digit = ($cardnum[$i]*2); }
			else { $digit = $cardnum[$i]; }

			for ($k=0; $k<strlen($digit); $k++) { $sum += substr($digit,$k,1); }
		}

		return ($sum%10);
	}

	/**
	* Return YES/NO if they filled in all of the required information
	* @return boolean
	* @access private
	*/
	function _hasRequiredInfo()
	{
		$can_be_blank = array('cardID','companyname');

		$any_blank = NO;
		while (list($k,$v) = each($this->info))
		{
			if (!in_array($k,$can_be_blank) && !strlen(trim($v))) { $any_blank = YES; }
		}
		reset($this->info);

		$this->valid = ($any_blank ? CC_MOREINFO : $this->valid);
		return ($any_blank ? NO : YES);
	}

	/**
	* Decide whether or not a card is valid
	* @return boolean
	* @access private
	*/
	function _verifyCard()
	{
		if ($this->_hasRequiredInfo())
		{
			$this->setTypes();
			$typeinfo = $this->getTypes($this->info['typeID']);
			$realtypeID = $this->getTypeID($this->info['number']);
			$mod10 = $this->_mod10($this->info['number']);

			if (!$mod10)
			{
				if ($realtypeID != $this->info['typeID']) { $this->valid = CC_BADTYPE; }
				else if (in_array(strlen($this->info['number']),$typeinfo['cct_lengths'])) { $this->valid = CC_VALID; }
				else { $this->valid = CC_BADNUM; }
			}
			else { $this->valid = CC_BADNUM; }

			if ($this->_isValid()) // if the card is valid up to this point, check the zip code
			{
				$val = new validate();
				if (!$val->zip_code($this->info['zip'])) { $this->valid = BADZIP; }
				elseif (!$val->phone($this->info['phone'])) { $this->valid = BADPHONE; }
			}
		}
	}



	/**
	* Return YES/NO if the card is valid or not
	* @return integer
	* @access private
	*/
	function _isValid()
	{
		return ($this->valid==CC_VALID ? YES : NO);
	}

	/**
	* Return the error code
	* @return integer
	* @access public
	*/
	function getErrorCode()
	{
		return $this->valid;
	}

	/**
	* Output the error for $error_code
	* @param integer $error_code
	* @access public
	*/
	function showError($error_code)
	{
		$errors = array(
			CC_BADNUM   => 'Invalid credit card number',
			CC_BADTYPE  => 'Credit card number is not valid for chosen card type',
			BADZIP      => 'Invalid zip code',
			BADPHONE    => 'Invalid phone number',
			CC_MOREINFO => 'You did not fill in enough information.<br />Please complete the entire form.'
		);

		if (isset($errors[$error_code])) { $error = $errors[$error_code]; }
		else { $error = 'Unknown error'; }

		$this->pg->error($error,'CREDIT CARD');
	}

	/**
	* Return the masked credit card number
	* @param string $number
	* @param boolean $short return the short number? IE: XX0025 [optional, default NO]
	* @return string
	* @access public
	*/
	function getMasked($number,$short=NO)
	{
		return ($short ? '****' . substr($number,-4) : str_repeat('*',(strlen($number)-4)) . substr($number,-4));
	}
}

?>