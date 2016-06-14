<?php
include('../include/include.inc');

$image = getG('image');

if ($image)
{
	header('Content-type: image/png');
}

$pg = new admin_page();
$sf = new sales_figures($pg);

// create the image and set the colors
$width = getG('width',600);
$height = getG('height',300);
$padding = 25;
$graph_width = 540;
$graph_height = 200;
$graph_startx = ($width - $graph_width);
$graph_starty = ($height - ($height - $graph_height) + $padding);
$font = 3;
$img = imagecreate($width,$height);

imagecolorallocate($img,255,255,255); // sets the background color to white
$colors = array(
	'black'     => imagecolorallocate($img,  0,  0,  0),
	'white'     => imagecolorallocate($img,255,255,255),
	'lightgray' => imagecolorallocate($img,225,225,225),
	'red'       => imagecolorallocate($img,255,  0,  0),
	'blue'      => imagecolorallocate($img,  0,  0,255),

	'axis'      => imagecolorallocate($img,225,225,225),
	'grid'      => imagecolorallocate($img,240,240,240),
	'label'     => imagecolorallocate($img,  0,100,185)
);

// get the image data; output an error if necessary
$which  = getG('which');
$runtot = getG('runtot',NO);
$labels = getGP('labels',YES);

if (!strlen($which))
{
	outputError('No $which provided!');
}
elseif (!isset($_SESSION['graph_data'][$which]))
{
	outputError("Image data for '$which' does not exist!");
}
else
{
	$graph_data = $_SESSION['graph_data'][$which];
	$data = $graph_data['data'];
	if (!is_array($data[0]))
	{
		$data = array($data);
	}
	$key = @$graph_data['key'];
	$crossover_point = $graph_data['crossover'];

	if (!$image)
	{
		?><div align="left">$graph_data<pre><?php echo print_r($graph_data['key'],true);?></pre></div><?php
		?><div align="left">$data<pre><?php echo print_r($data,true);?></pre></div><?php
		die();
	}

	// get the colors and limits
	assignColors($colors,$data);
	$limits = getLimits($data,$runtot);

	if ($limits == -1)
	{
		outputError('There is not enough data available to graph this date!');
	}
	else
	{
		extract($limits);

		/*
		Hourly                       Daily                        Monthly
		Array                        Array                        Array
		(                            (                            (
		    [x_limit] => 24              [x_limit] => 31              [x_limit] => 12
		    [x_step] => 1                [x_step] => 1                [x_step] => 1
		    [x_column_width] => 16       [x_column_width] => 12       [x_column_width] => 33
		    [y_limit] => 120             [y_limit] => 1400            [y_limit] => 22000
		    [y_step] => 10               [y_step] => 100              [y_step] => 1000
		    [y_row_height] => 20         [y_row_height] => 17         [y_row_height] => 11
		)                            )                            )
		*/

		// draw the graph's gridlines and show the labels
		$extend = 3; // pixels to extend over axis
		$padfromaxis = 7; // pixels between label and axis

		// number of X points
		$keys = array_keys($data[0]);
		$x_count = count($keys);

		// x gridlines/labels (top->bottom)
		$j = 0;
		for ($i=$graph_startx; $j<$x_count; $i+=$x_column_width)
		{
			$j++;
			$label = $sf->formatLabel($which,$keys[($j-1)]);
			$stringwidth = (imagefontwidth($font) * strlen($label));

			imageline(
				$img,
				$i,
				($graph_starty + $extend),
				$i,
				0,
				$colors['grid']
			);
			imagestringup(
				$img,
				$font,
				($i - 6),
				($graph_starty + $padfromaxis + $stringwidth),
				$label,
				$colors['label']
			);
		}

		// y gridlines/labels (left->right)
		$j = -1;
		for ($i=$graph_starty; $i>0; $i-=$y_row_height)
		{
			$j++;
			$string = number_format(($j*$y_step),0);
			$stringwidth = (imagefontwidth($font) * strlen($string));

			if (($j*$y_step) <= $y_limit)
			{
				imageline(
					$img,
					($graph_startx - $extend),
					$i,
					$width,
					$i,
					$colors['grid']
				);
				imagestring(
					$img,
					$font,
					($graph_startx - $padfromaxis - $stringwidth),
					($i - 6),
					$string,
					$colors['label']
				);
			}
		}

		// x axis
		imageline(
			$img,
			($graph_startx - $extend),
			$graph_starty,
			$width,
			$graph_starty,
			$colors['axis']
		);
		// y axis
		imageline(
			$img,
			$graph_startx,
			($graph_starty + $extend),
			$graph_startx,
			0,
			$colors['axis']
		);

		// draw the graph key (if applicable)
		if (count($key))
		{
			$posX = 125;
			$posY = ($graph_starty + 38);

			while (list($a,$show) = each($key))
			{
				if (($a == (count($data)-1) && array_sum($data[$a]) != array_sum($data[0])) || ($a != (count($data)-1) && array_sum($data[$a])))
				{
					$lineY = ($posY + floor(imagefontheight($font) / 2));
					imagesetthickness($img,3);
					imageline(
						$img,
						($posX - 60),
						$lineY,
						($posX - 10),
						$lineY,
						$colors["data_$a"]
					);
					imagesetthickness($img,1);
					imagestring(
						$img,
						$font,
						$posX,
						$posY,
						$show,
						$colors['black']
					);
					//imagestring($img,$font,$posX,$posY,implode(' ',$_SESSION['graph_colors'][$a]),$colors['black']); // uncomment to output this line's colors

					$posY += (imagefontheight($font) - 2);
				}
			}
		}

		// get the minimum/maximum x-axis values
		$vals = array(); // format: $vals[data_array_idx][x_val] = X
		$xy = array(); // format: $xy[data_array_idx][x_val] = array(X,Y)
		$min = 1000000; // Heaven-forbid it's larger than this...
		$max = 0;
		while (list($a,$arr) = each($data))
		{
			$keys = array_keys($arr);
			sort($keys);
			$thismin = array_shift($keys);
			$thismax = array_pop($keys);

			//$curval[$a] = $arr[$thismin];
			$vals[$a] = array();
			$xy[$a] = array();

			if ($thismin < $min) { $min = $thismin; }
			if ($thismax > $max) { $max = $thismax; }
		}
		reset($data);

		// build array of values and get the X/Y points
		$data_count = count($data);
		$count = -1;
		for ($i=$min; $i<=$max; $i++)
		{
			$count++;

			for ($j=0; $j<$data_count; $j++)
			{
				$lastval = ($i>$min ? $vals[$j][($i-1)] : 0);
				$vals[$j][$i] = ($runtot ? ($lastval + @$data[$j][$i]) : @$data[$j][$i]);

				$x = $graph_startx + ($x_column_width * $count);
				$y = floor(($graph_starty - ($graph_height * ($vals[$j][$i] / $y_limit))));

				$xy[$j][$i] = array($x,$y);
			}
		}

		// draw the lines/dots/labels
		$point_size = 6;
		$posX = array();
		$posY = array();
		$label_data = array(); // format: $label_data[#] = array(x,y,label,color)
		for ($i=$min; $i<=$max; $i++)
		{
			for ($j=($data_count - 1); $j>=0; $j--)
			{
				if (array_sum($vals[$j]))
				{
					$val = $vals[$j][$i];
					list($x,$y) = $xy[$j][$i];

					$drawit = (($j > 0 && $j < ($data_count-1)) || ($j == ($data_count-1) && $i > $crossover_point) || (!$j && $i <= $crossover_point)?YES:NO);

					if ($i == $min)
					{
						// far left point - don't draw a line - just set the current X/Y and draw a dot
						$posX[$j] = $x;
						$posY[$j] = $y;
					}
					else
					{
						if ($drawit)
						{
							// output a thicker line
							imagesetthickness($img,3);
							imageline(
								$img,
								$posX[$j],
								$posY[$j],
								$x,
								$y,
								$colors["data_$j"]
							);
							imagesetthickness($img,1);
						}

						$lastX = $posX[$j];
						$lastY = $posY[$j];
						$posX[$j] = $x;
						$posY[$j] = $y;
					}

					// if applicable, calculate the label positions
					if ($drawit && $labels && $val > 0)
					{
						$val_font = 2;
						$val = number_format($val,2,'.','');
						$val_len = (imagefontwidth($val_font) * strlen($val));
						$half_height = floor(imagefontheight($val_font) / 2);
						$overY = ($posY[$j] - 7);
						$underY = ($posY[$j] + $val_len + 5);

						// show it under or over the point?
						//if ((!$j || $underY > $graph_starty) && ($overY-$val_len) >= 0) { $valY = $overY; }
						//else { $valY = $underY; }

						$otherval = (!$j ? $vals[($j + 1)][$i] : $vals[($j - 1)][$i]);
						if ($underY > $graph_starty || ($val > $otherval && ($overY-$val_len) >= 0))
						{
							$valY = $overY;
						}
						else
						{
							$valY = $underY;
						}

						$label_data[] = array(
							($posX[$j] - (!$j || ($j + 1)==$data_count ? ($half_height * 2) - 1 : 2)),
							$valY,
							$val,
							$colors["data_$j"]
						);
					}

					// draw the dots
					if ($drawit || (!$j && $i == ($crossover_point + 1)))
					{
						// draw dot(s)
						imagefilledellipse(
							$img,
							($i==$min ? $x : $lastX),
							($i==$min ? $y : $lastY),
							$point_size,
							$point_size,
							$colors["data_$j"]
						);
						// draw the last dot
						if ($i == $max)
						{
							imagefilledellipse(
								$img,
								$posX[$j],
								$posY[$j],
								$point_size,
								$point_size,
								$colors["data_$j"]
							);
						}
					}
				}
			}
		}

		// draw the labels (this is not done above so that the text appears above all lines/dots)
		while (list($a,list($x,$y,$label,$color)) = each($label_data))
		{
			imagestringup(
				$img,
				$val_font,
				$x,
				$y,
				$label,
				$color
			);
		}

		// write 'current as of...' line
		$string = 'Current as of ' . date('m/d/Y h:i:sa');
		imagestring(
			$img,
			$font,
			($width - (imagefontwidth($font) * strlen($string)) - 1),
			($height - imagefontheight($font) - 1),
			$string,
			$colors['lightgray']
		);
	} // else enough data exists to graph the date
} // else graph data exists

/**
* functions
*/

// output an error string
function outputError($text)
{
	global $img,$colors,$font;

	$text = "Error: $text";

	$heightperline = (imagefontheight($font)+4);
	$posY = 5;
	$maxchars = floor((imagesx($img)/imagefontwidth($font))-1);

	$lines = explode('|||',wordwrap($text,$maxchars,'|||',1));

	//imagefilledrectangle($img,0,0,imagesx($img),imagesy($img),$colors['red']);
	while (list($a,$line) = each($lines))
	{
		imagestring($img,$font,5,$posY,$line,$colors['black']);
		$posY += $heightperline;
	}
} // end function outputError

/**
* Given a data set (or data sets) return the X/Y limit, step, and column/row width/height
* @param	array	$data
* @param	boolean	$runtot
* @return	array
* @access	public
*/
function getLimits($data,$runtot)
{
	global $graph_width,$graph_height;

	$x_limit = 0;
	$x_step = 1;
	$x_column_width = 0;
	$y_limit = 0;
	$y_step = 0;
	$y_row_height = 0;

	// calculate limits
	while (list($a,$arr) = each($data))
	{
		$keys = array_keys($arr);
		asort($arr);

		// use array_pop(array) to not modify the actual array
		if (array_pop(array_values($keys)) > $x_limit)
		{
			$x_limit = array_pop(array_values($keys));
		}
		if ($runtot)
		{
			$sum = array_sum($arr);
			if ($sum > $y_limit)
			{
				$y_limit = $sum;
			}
		}
		else
		{
			if (array_pop(array_values($arr)) > $y_limit)
			{
				$y_limit = array_pop(array_values($arr));
			}
		}
	}

	// decide on the y-axis increments
	if ($y_limit < 10) { $num = 1; }
	elseif ($y_limit < 100) { $num = 10; }
	elseif ($y_limit < 500) { $num = 50; }
	elseif ($y_limit < 1000) { $num = 100; }
	elseif ($y_limit < 3000) { $num = 250; }
	elseif ($y_limit < 10000) { $num = 500; }
	elseif ($y_limit < 100000) { $num = 2000; }
	elseif ($y_limit < 1000000) { $num = 12000; } // this would be really nice...
	else { $num = 125000; } // this would be even nicer! =P

	// even up y_limit (if they go above $1,000,000, this will break - lets hope that happens =)
	$y_limit = (ceil($y_limit / $num) * $num);
	$y_step = $num;

	// check for 0 limits
	if (!$x_limit || !$y_limit || !$x_step || !$y_step)
	{
		return -1;
	}
	else
	{
		// calculate column/row widths/heights
		$x_column_width = floor($graph_width / count($data[0]));
		$y_row_height = floor($graph_height / ($y_limit / $y_step));

		return array(
			'x_limit'        => $x_limit,
			'x_step'         => $x_step,
			'x_column_width' => $x_column_width,
			'y_limit'        => $y_limit,
			'y_step'         => $y_step,
			'y_row_height'   => $y_row_height
		);
	}
} // end function getLimits

// assign colors to the data sets
function assignColors(&$colors,$data)
{
	global $img;

	$use_colors = array(
		array( 93,148,236), // data
		array( 61, 41,102), // average
		array(210,210,210), // estimate
		array( 20,  9,242)
	);

	while (list($a,$arr) = each($data))
	{
		list($r,$g,$b) = $use_colors[$a];
		$_SESSION['graph_colors'][$a] = array($r,$g,$b);
		$colors["data_$a"] = imagecolorallocate($img,$r,$g,$b);
	}
} // end function assignColors

// output and destroy the image
imagepng($img);
imagedestroy($img);
?>