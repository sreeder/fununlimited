<?php
/**
* Barcode functionality
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class barcode
{
	/**
	* Map of numbers and their respective characters in the font
	* @var array $map
	*/
	var $map;

	/**
	* UPC number system (first character - 4 is custom)
	* @var integer $numbersystem
	*/
	var $numbersystem;

	/**
	* Manufacturer ID for the barcode
	* @var integer $mfgID
	*/
	var $mfgID;

	/**
	* First UPC
	* @var integer $first_upc
	*/
	var $first_upc;

	/**
	* Class constructor
	* Initializes variables
	*/
	function barcode()
	{
		/*
		!!! change $this->mfgID to be XXX000 where XXX=left 0 padded platformID (IE: 064000) !!!
		*/
		$this->numbersystem = 4;
		$this->mfgID = 84321;
		$this->first_upc = $this->numbersystem.$this->mfgID.'000000';

		// map left/right hand barcode numbers to the required values for the font
		$this->map = array(
			'START'=>array(0=>'P',1=>'Q',2=>'R',3=>'S',4=>'T',5=>'U',6=>'V',7=>'W',8=>'X',9=>'Y'),
			'LEFT'=>array(0=>'0',1=>'1',2=>'2',3=>'3',4=>'4',5=>'5',6=>'6',7=>'7',8=>'8',9=>'9'),
			'CENTER'=>'p',
			'RIGHT'=>array(0=>'@',1=>'A',2=>'B',3=>'C',4=>'D',5=>'E',6=>'F',7=>'G',8=>'H',9=>'I'),
			'STOP'=>array(0=>'`',1=>'a',2=>'b',3=>'c',4=>'d',5=>'e',6=>'f',7=>'g',8=>'h',9=>'i')
		);
	}

	/*
	* Returns the check digit for a UPC number
	* (the check digit is the number it takes to make $this->calc($upc) divisible by 10)
	* @param integer $upc
	*/
	function check_digit($upc)
	{
		return ((10-($this->calc($upc)%10))%10);
	}

	/*
	* Determines if the UPC number is valid
	* @param integer $upc
	*/
	function is_valid($upc)
	{
		if ($this->check_digit($upc) == substr($upc,-1)) { return YES; }
		else { return NO; }
	}

	/*
	* Performs the UPC validation math (doesn't add in the check digit)
	* @param integer $upc
	*/
	function calc($upc)
	{
		$tot = 0;
		for ($i=0; $i<=10; $i+=2) { $tot += substr($upc,$i,1); } // sum odd digits
		$tot *= 3; // times sum by 3
		for ($i=1; $i<=9; $i+=2) { $tot += substr($upc,$i,1); } // add sum of even digits

		return $tot;
	}

	/*
	* Returns the next valid UPC number
	* @param integer $lastupc
	*/
	function next_upc($lastupc)
	{
		$nextupc = ltrim(substr($lastupc,6,6),'0');
		if (!strlen($nextupc)) { $nextupc = 0; }
		for ($i=($nextupc+1); !$this->is_valid(($this->numbersystem.$this->mfgID.str_pad($i,6,'0',STR_PAD_LEFT))); $i++) { }
		return $this->numbersystem.$this->mfgID.str_pad($i,6,'0',STR_PAD_LEFT);
	}

	/*
	* Fixes a UPC number for display with the font
	* @param integer $upc
	*/
	function fix($upc)
	{
		$fixed = "";

		$numsys = substr($upc,0,1);
		$lfive = substr($upc,1,5);
		$rfive = substr($upc,6,5);
		$checkdgt = substr($upc,11,1);

		$fixed .= $this->map['START'][$numsys];
		for ($i=0; $i<5; $i++) { $fixed .= $this->map['LEFT'][substr($lfive,$i,1)]; }
		$fixed .= $this->map['CENTER'];
		for ($i=0; $i<5; $i++) { $fixed .= $this->map['RIGHT'][substr($rfive,$i,1)]; }
		$fixed .= $this->map['STOP'][$checkdgt];

		return $fixed;
	}
}

?>