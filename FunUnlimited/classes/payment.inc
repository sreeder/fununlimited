<?php
/**
* Payment class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class payment
{
	/**
	* page class reference
	* @var object $pg
	*/
	var $pg;

	/**
	* Transaction data
	* @var array $data
	*/
	var $data;

	/**
	* Success?
	* @var integer $success
	*/
	var $success;

	/**
	* Error code
	* @var integer $error_code
	*/
	var $error_code;

	/**
	* Error text
	* @var string $error_text
	*/
	var $error_text;

	/**
	* Transaction data
	* @var array $transaction_data
	*/
	var $transaction_data;

	/**
	* Transaction data field human-readable equivalents
	* @var array $transaction_data_words
	*/
	var $transaction_data_words;

	/**
	* Transaction data field names
	* @var array $transaction_data_fields
	*/
	var $transaction_data_fields;

	/**
	* Error reporting class
	* @var object $error
	*/
	var $error;

	/**
	* Class constructor
	* Initializes variables
	*/
	function payment()
	{
		$this->setTransactionDataWords();
		$this->error = new error('Payment class');
	}

	/**
	* Set transaction data
	* @param array $data
	*/
	function setData($data)
	{
		$this->data = $data;
	}

	/**
	* Return a data element
	* @param string $element
	* @param integer $required die if the requested element is not set? [optional, default NO]
	* @return string
	*/
	function getData($element,$required=NO)
	{
		if ($required && !isset($this->data[$element]))
		{
			die("Payment class error - data element not set: $element");
		}

		return @$this->data[$element];
	}

	/**
	* Submit the payment and set the success/error codes
	*/
	function submitPayment()
	{
		// Paymentech/Authorize.net
		$login = 'CONNEX21';
		$tran_key = 'ZTFL9Pn1eisImyrh'; //ZTFL9Pn1eisImyrh

		$test_mode = YES; // run test transactions? (no money is actually charged/transferred)

		// URLs for payment posting
		$liveURL = 'https://secure.authorize.net/gateway/transact.dll'; // used when NOT in test mode
		$testURL = 'https://certification.authorize.net/gateway/transact.dll'; // used when in test mode

		// return value delimeter
		$delimeter = ',';

		// merchant data
		$merchant_data = array(
			'x_login'          => $login,
			'x_tran_key'       => $tran_key,
			'x_test_request'   => ($test_mode ? 'TRUE' : 'FALSE'),
			'x_type'           => 'AUTH_CAPTURE',
			'x_method'         => 'CC',
			'x_merchant_email' => $this->getData('store_email',YES) // uncomment to send the confirmation email
		);

		// customer data
		$customer_data = array(
			'x_first_name'     => $this->getData('first_name',YES),
			'x_last_name'      => $this->getData('last_name',YES),
			'x_company'        => $this->getData('company',YES),
			'x_address'        => $this->getData('address',YES),
			'x_city'           => $this->getData('city',YES),
			'x_state'          => $this->getData('state',YES),
			'x_zip'            => $this->getData('zip',YES),
			'x_phone'          => $this->getData('phone',YES),
			'x_cust_id'        => $this->getData('customerID',YES),
			'x_customer_ip'    => $this->getData('customer_ip',YES),
			'x_email'          => $this->getData('customer_email',YES),
			'x_email_customer' => 'FALSE' // set to 'TRUE' to send the confirmation email to the customer
		);

		// transaction data
		$transaction_data = array(
			'x_description'    => $this->getData('description',YES),
			//'x_amount'       => (rand(1,300)/100), // uncomment and comment next line to run $3.00 or less transactions
			'x_amount'         => $this->getData('amount',YES),
			'x_card_num'       => $this->getData('card_number',YES),
			'x_exp_date'       => $this->getData('card_expiration_date',YES)
			//'x_card_code'    => 139
		);

		// other data
		$other_data = array(
			'x_delim_data'     => 'TRUE',
			'x_delim_char'     => $delimeter,
			'x_version'        => '3.1',
			'x_relay_response' => 'FALSE' // this HAS TO BE be false!
		);

		$all_data = array_merge($merchant_data,$customer_data,$transaction_data,$other_data);

		$get_line = $this->buildGetLine($all_data);
		$postURL = ($test_mode ? $testURL : $liveURL);

		// initialize the URL for cURL
		$ch = curl_init($postURL);

		// set the cURL options
		curl_setopt($ch,CURLOPT_RETURNTRANSFER,1);
		curl_setopt($ch,CURLOPT_POST,1);
		curl_setopt($ch,CURLOPT_POSTFIELDS,$get_line);
		curl_setopt($ch,CURLOPT_SSL_VERIFYPEER,0); // don't verify the certificate (if it tries to verify, it fails)
		curl_setopt($ch,CURLOPT_CONNECTTIMEOUT,20);
		curl_setopt($ch,CURLOPT_TIMEOUT,20);

		// post the data, read the result, and close the cURL session
		$return = curl_exec($ch);

		if ($return == NULL)
		{
			$this->success = NO;
			$this->error_code = -1;
			$this->error_text = 'Unable to connect to the payment processing center. Please try again later. We apologize for any inconvenience. <!-- cURL Error ('.curl_errno($ch).'): '.curl_error($ch).' -->';

			/*
			echo "cURL Error:<br />";
			echo curl_errno($ch).' - '.curl_error($ch).'<p />';
			*/
		}
		else
		{
			// split the return string by the delimeter
			$status = explode($delimeter,$return);

			$response_code      = $status[0];
			$reason_code        = $status[2];
			$reason_text        = $status[3];
			$authorization_code = $status[4];
			$avs_response       = $status[5];
			$transactionID      = $status[6];
			$amount             = $status[9];
			$cardcode_response  = $status[38];

			// generate the transaction data
			// this is saved to the database after the invoice has been created
			$transaction_data = array(
				'transactionID'  => $transactionID,
				'amount'         => $amount,
				'cardID'         => $this->getData('cardID',YES),
				'authorize_code' => $authorization_code,
				'response_code'  => $response_code,
				'reason_code'    => $reason_code,
				'reason_text'    => $reason_text,
				'avs_response'   => $avs_response,
				'ccv_response'   => $cardcode_response
			);
			$_SESSION['transaction_data'] = $transaction_data;
			//$this->saveTransactionData($orderID,$transaction_data);

			if ($response_code == 1)
			{
				// the card was successfully charged/the transaction was approved
				$this->success = YES;
			}
			else
			{
				// the transaction failed - store the error code/reason
				$this->success = NO;
				$this->error_code = $reason_code;
				$this->error_text = $reason_text;
			}
		}

		curl_close($ch);
	}

	/**
	* Return YES/NO if the payment was a success
	*/
	function paymentSuccess()
	{
		return ($this->success ? YES : NO);
	}

	/**
	* If the payment failed, return the error code and the error text
	* @return array format: array(code,text)
	*/
	function getError()
	{
		return array($this->error_code,$this->error_text);
	}

	/**
	* Build and return an HTTP GET line from the key=>value pairs
	* @param array $pairs
	* @return string
	*/
	function buildGetLine($pairs)
	{
		$vals = array();
		while (list($k,$v) = each($pairs))
		{
			//$vals[] = "$k=".urlencode($v); // is urlencode necessary?
			$vals[] = "$k=".str_replace(' ','+',$v);
		}

		return implode('&',$vals);
	}

	/**
	* Save transaction-related data
	* @param integer $orderID
	* @param array $data format: array('name'=>'value'[,'name'=>'value',...])
	*/
	function saveTransactionData($orderID,$data)
	{
		global $db;

		$vals = array();
		while (list($k,$v) = each($data))
		{
			if (strlen(trim($v))) { $vals[] = "($orderID,'$k','".mysql_real_escape_string($v)."',".time().")"; }
		}

		$sql = "INSERT INTO transaction_data VALUES ".implode(',',$vals);
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
	}

	/**
	* Pull in the transaction data for the given orderID
	* @param integer $orderID
	*/
	function setTransactionData($orderID)
	{
		global $db;

		$this->transaction_data = array();

		$sql = "SELECT * FROM transaction_data WHERE trd_orderID=$orderID ORDER BY trd_time";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result))
		{
			$this->transaction_data[$row['trd_name']] = $row;
		}
	}

	/**
	* Return the transaction data
	* @param string $name Field name [optional, default '']
	* @param string $column Column to return [optional, default '']
	* @return string
	*/
	function getTransactionData($name='',$column='')
	{
		if (strlen($name))
		{
			$data = @$this->transaction_data[$name];

			if (!strlen($column)) { return $data; }
			else { return @$data[$column]; }
		}
		else { return $this->transaction_data; }
	}

	/**
	* Clear the transaction data for the given order
	* @param integer $orderID
	*/
	function clearTransactionData($orderID)
	{
		global $db;

		$sql = "DELETE FROM transaction_data WHERE trd_orderID=$orderID";
		mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
	}

	/**
	* Sets the valid transaction data field names
	*/
	function setTransactionDataWords()
	{
		$this->transaction_data_words = array(
			'transactionID'  => 'Transaction ID',
			'amount'         => 'Amount',
			'cardID'         => 'Credit Card ID',
			'authorize_code' => 'Authorization Code',
			'response_code'  => 'Response Code',
			'reason_code'    => 'Response Reason Code',
			'reason_text'    => 'Response Reason/Status',
			'avs_response'   => 'AVS Response',
			'ccv_response'   => 'CCV Response'
		);
		$this->transaction_data_fields = array_keys($this->transaction_data_words);
	}

	/**
	* Return the human-readable equivalent of a transaction data field name (if there is no human-readable equivalent, the field name is returned)
	* @param string $name
	* @return string
	*/
	function getTransactionDataWord($name)
	{
		$return = @$this->transaction_data_words[$name];
		if (!strlen($return)) { $return = $name; }

		return $return;
	}
}

?>