<?php
/**
* Graph drawing class
*
* functions:
*  barGraph
*  lineGraph
*  dateGraph
*  timeGraph
*
* functions need args to determine if it's a bar graph or line graph
**/

class graph
{
	/**
	* Graph data
	* @param array $data
	*/
	var $data;

	/**
	* Initialize class and retrieve graph data
	* The MD5 hash should only be passed if the data is set in the page that outputs the <img> tag
	* The scope of the image variable won't extend to the image unless it's in a session variable
	* @param string $md5 MD5 hash of data variable key [optional, default '']
	*/
	function graph($md5='')
	{
		$this->data = @$_SESSION['image_data'][$md5];
	}

	/**
	* Set the graph data
	* @param array $data
	*/
	function setData($data) { $this->data = $data; }

	/**
	* Get the graph data
	* @return array
	*/
	function getData() { return $this->data; }
}
?>