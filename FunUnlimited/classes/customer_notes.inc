<?php
/**
* Customer notes class (adding/editing/removing)
*
* @author Scott Carpenter <s-carp@comcast.net>
* @copyright Copyright � 2003-2006 Scott Carpenter <s-carp@comcast.net>
* @version 1.0
* @package Fun Unlimited
*/

class customer_notes
{
	/**
	* Customer notes
	* @var array $notes
	*/
	var $notes;

	/**
	* Note type words
	* @var array $note_words
	*/
	var $note_words;

	/**
	* Note type images
	* @var array $note_images
	*/
	var $note_images;

	/**
	* Class constructor (called in customer.inc constructor)
	*/
	function customer_notes()
	{
		$this->note_words = array(
			NOTE_POSITIVE=>'Positive',
			NOTE_NEUTRAL=>'Neutral',
			NOTE_NEGATIVE=>'Negative'
		);
		$this->note_images = array(
			NOTE_POSITIVE=>'/images/note_positive.gif',
			NOTE_NEUTRAL=>'/images/note_neutral.gif',
			NOTE_NEGATIVE=>'/images/note_negative.gif'
		);
	}

	/**
	* Pull in customer notes
	* @param integer $customerID customerID of customer to pull notes for
	* @param integer/array $noteIDs array of noteIDs (can be array or single noteID) [optional, default array()]
	*/
	function setNotes($customerID,$noteIDs=array())
	{
		global $db;

		if (!is_array($noteIDs)) { $noteIDs = array($noteIDs); }

		if (count($noteIDs)) { $notewhere = "AND cun_noteID IN (".implode(',',$noteIDs).") "; } else { $notewhere = ""; }

		$this->notes = array();
		$sql = "SELECT * FROM customer_notes,customers,users WHERE cun_customerID=$customerID AND cun_customerID=cus_customerID AND cun_userID=use_userID AND cun_active=" . YES . " {$notewhere}ORDER BY cun_time DESC";
		$result = mysql_query($sql,$db);
		$this->error->mysql(__FILE__,__LINE__);

		while ($row = mysql_fetch_assoc($result)) { $this->notes[$row['cun_noteID']] = $row; }
	}

	/**
	* Retrieve one/all notes
	* @param integer $noteID noteID of the note to return [optional, default 0]
	*/
	function getNotes($noteID=0)
	{
		if (isset($this->notes[$noteID])) { return $this->notes[$noteID]; }
		else { return $this->notes; }
	}

	/**
	* Add the POSTed customer note
	*/
	function addNote()
	{
		global $db;

		$data = @$_POST['data'];

		if (is_array($data))
		{
			$vals = array(
				"cun_customerID={$data['customerID']}",
				"cun_userID={$_SESSION['userID']}",
				"cun_type={$data['type']}",
				"cun_note='".mysql_real_escape_string($data['note'])."'",
				"cun_time=".time(),
				"cun_active=".YES
			);

			$sql = "INSERT INTO customer_notes SET ".implode(',',$vals);
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}
	}

	/**
	* Set a customer note as inactive
	* @param integer $noteID
	*/
	function deleteNote($noteID)
	{
		global $db;

		if ($noteID)
		{
			$sql = "UPDATE customer_notes SET cun_active=".NO." WHERE cun_noteID=$noteID";
			mysql_query($sql,$db);
			$this->error->mysql(__FILE__,__LINE__);
		}
	}

	/**
	* Return the note types
	* @return array
	*/
	function getNoteTypes()
	{
		return array_keys($this->note_words);
	}

	/**
	* Return the word for the given note type
	* @param integer $type
	* @return string
	*/
	function getNoteTypeWord($type)
	{
		return @$this->note_words[$type];
	}

	/**
	* Return the image path for the given note type
	* @param integer $type
	* @return string
	*/
	function getNoteTypeImagePath($type)
	{
		return @$this->note_images[$type];
	}

	/**
	* Return the image tag for the given note type
	* @param integer $type NOTE_[POSITIVE/NEUTRAL/NEGATIVE]
	* @return string
	*/
	function getNoteTypeImage($type)
	{
		$path = $this->getNoteTypeImagePath($type);
		if (strlen($path)) { return '<img src="'.$path.'" border="0" align="top" alt="'.$this->getNoteTypeWord($type).'" />'; }
		else { return ''; }
	}
}

?>