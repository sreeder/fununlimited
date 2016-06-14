<?php
/**
* Array of states in the US
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class states
{
	/**
	* Array of states
	* @var array $states
	*/
	var $states;

	/**
	* Class constructor
	* Populates the state array
	*/
	function states()
	{
		$this->states = array(
			""=>"",
			"AL"=>"Alabama",
			"AK"=>"Alaska",
			"AZ"=>"Arizona",
			"AR"=>"Arkansas",
			"CA"=>"California",
			"CO"=>"Colorado",
			"CT"=>"Connecticut",
			"DW"=>"Delaware",
			"FL"=>"Florida",
			"GA"=>"Georgia",
			"HA"=>"Hawaii",
			"ID"=>"Idaho",
			"IL"=>"Illinois",
			"IN"=>"Indiana",
			"IA"=>"Iowa",
			"KS"=>"Kansas",
			"KY"=>"Kentucky",
			"LA"=>"Louisiana",
			"ME"=>"Maine",
			"MD"=>"Maryland",
			"MA"=>"Massachusetts",
			"MI"=>"Michigan",
			"MN"=>"Minnesota",
			"MS"=>"Mississippi",
			"MO"=>"Missouri",
			"MT"=>"Montana",
			"NE"=>"Nebraska",
			"NV"=>"Nevada",
			"NH"=>"New Hampshire",
			"NJ"=>"New Jersey",
			"NM"=>"New Mexico",
			"NY"=>"New York",
			"NC"=>"North Carolina",
			"ND"=>"North Dakota",
			"OH"=>"Ohio",
			"OK"=>"Oklahoma",
			"OR"=>"Oregon",
			"PA"=>"Pennsylvania",
			"RI"=>"Rhode Island",
			"SC"=>"South Carolina",
			"SD"=>"South Dakota",
			"TN"=>"Tennessee",
			"TX"=>"Texas",
			"UT"=>"Utah",
			"VT"=>"Vermont",
			"VA"=>"Virginia",
			"WA"=>"Washington",
			"WV"=>"West Virginia",
			"WI"=>"Wisconsin",
			"WY"=>"Wyoming"
		);
	}

	/**
	* Returns the state name for an abbreviation
	* @param string $abb Abbreviation
	* @return string State name
	*/
	function get_name($abb)
	{
		return @$this->states[strtoupper($abb)];
	}

	/**
	* Returns the abbreviation for a state name
	* @param string $name State name
	* @return string Abbreviation
	*/
	function get_abb($name)
	{
		return array_search($name,$this->states);
	}
}