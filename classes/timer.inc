<?php
/**
* Timer class
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright © 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class timer
{
	/**
	* Start times
	* @var array $start
	*/
	var $start;

	/**
	* Stop times
	* @var array $stop
	*/
	var $stop;

	/**
	* Name counts
	* @var array $name_counts
	*/
	var $name_counts;

	/**
	* Class constructor
	* Initializes variables
	*/
	function timer()
	{
		$this->start = array();
		$this->stop = array();
		$this->name_counts = array();
	}

	/**
	* Start a timer
	* @param string $name
	* @param integer $microtime if not passed, defaults to $this->micro_time() [optional, default -1]
	*/
	function startTimer($name,$microtime=-1)
	{
		if ($microtime == -1) { $microtime = $this->micro_time(); }
		if (!isset($this->name_counts[$name])) { $this->name_counts[$name] = 0; }
		$this->name_counts[$name]++;

		$this->start["{$this->name_counts[$name]}_$name"] = $microtime;
	}

	/**
	* Stop a timer
	* @param string $name
	*/
	function stopTimer($name)
	{
		$this->stop["{$this->name_counts[$name]}_$name"] = $this->micro_time();
	}

	/**
	* Output a table of times
	*/
	function viewTimes()
	{
		$names = array_keys($this->start);

		?>
		<table border="1" cellspacing="0" cellpadding="3" style="color:black">
			<tr>
				<td><b>Name</b></td>
				<td><b>Start</b></td>
				<td><b>Stop</b></td>
				<td><b>Time</b></td>
			</tr>
		<?php

		while (list($a,$name) = each($names))
		{
			if (isset($this->stop[$name]))
			{
				$start = $this->start[$name];
				$stop = $this->stop[$name];
				$time = ($stop-$start);

				?>
				<tr>
					<td><?=$name;?></td>
					<td align="right"><?=sprintf('%0.3f',$start);?></td>
					<td align="right"><?=sprintf('%0.3f',$stop);?></td>
					<td align="right"><?=sprintf('%0.3f',$time);?></td>
				</tr>
				<?php
			}
		}

		?></table><?php
	}

	/**
	* Returns millisecond time
	*/
	function micro_time()
	{
		$exp = explode(" ",microtime());
		return ((float)$exp[1]+(float)$exp[0]);
	}
}

?>
