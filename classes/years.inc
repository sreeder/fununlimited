<?php
/**
* Year maintenance
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class years
{
	/**
	* All years
	* @var array $years
	*/
	var $years;

	/**
	* Error reporting class
	* @var identifier $error
	*/
	var $error;

	/**
	* Class constructor
	* Pulls in all years and makes sure the current year exists in the database - if it doesn't, it's inserted
	* @param integer $lowfirst Start at the lowest year (1970-current, instead of current-1970)? [optional, default NO]
	*/
	function years($lowfirst=NO)
	{
		global $db;

		$this->error = new error('Year');

		$this->years = array();

		$curyear = date('Y');
		$hascuryear = NO;
		$unknown = array();

		$sql = "SELECT * FROM years WHERE yer_year<=".(date('Y')+1)." ORDER BY yer_year ".($lowfirst==YES?'ASC':'DESC');
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);
		while ($row = mysql_fetch_assoc($result))
		{
			if ($row['yer_year'] != 'Unknown')
			{
				$this->years[] = $row;

				$year = $row['yer_year'];
				if ($year == $curyear) { $hascuryear = YES; }
			}
			else { $unknown = array($row); }
		}
		$this->years = array_merge($this->years,$unknown);

		if ($hascuryear == NO)
		{
			// insert the current year
			$sql = "INSERT INTO year VALUES (NULL,$curyear)";
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
			$curarr = array('yer_yearID'=>mysql_insert_id(),'yer_year'=>$curyear);
			if ($lowfirst == YES) { $this->years[] = $curarr; }
			else { $this->years = array_merge(array($curarr),$this->years); }
		}
	}

	/**
	* Return the years
	* @return	array
	* @access	public
	*/
	function getYears()
	{
		return $this->years;
	}
}

?>