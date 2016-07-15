<?php
include('../include/include.inc');

$platformID = $_GET['platformID'];
$platform_name = $_GET['platform_name'];
?>
var upcs = new Array();
var itemIDs = new Array();
var titles = new Array();
var striptitles = new Array();
var lastval = '';
var lastobj = null;
var lastnewused = 0;
var lastisupc = true;
var lastfocus = null;
var totmultiple = 0;

// set the upc/itemID/title for a line
function info(idx,upc,itemID,title)
{
	upcs[idx] = upc;
	itemIDs[idx] = itemID;
	titles[idx] = title;
	striptitles[idx] = stripString(title);
}

function stripString(txt)
{
	var keepchars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
	var retstring = '';
	for (var i=0; i<txt.length; i++)
	{
		var c = txt.charAt(i);
		if (keepchars.indexOf(c) != -1) { retstring += c; }
	}
	return retstring;
}


// go to a different page
function goto_page(page)
{
	var frm = document.itmfrm;
	frm.page.value = page;
	frm.submit();
}

// play the sound
function play_sound(times)
{
	var obj = document.getElementById('beep');
	obj.volume = 0;
	obj.loop = times;
	obj.src = 'beep.wav';
}

// reset the form
function reset_form(frm)
{
	if (confirm('Are you sure you want to reset the form?\n\nYou will lose any changes to the quantities!'))
	{
		clear_scanned();
		frm.reset();
		document.upc.newused[1].checked = true;
		document.upc.upc.focus();
	}
}

// clear the 'scanned item details' values
function clear_scanned()
{
	scan_upc.innerText = '';
	scan_title.innerText = '';
	scan_qty.innerText = '';
}

// checks key presses in the UPC/title box
// if enter, check the UPC/title
// if n/u, change to new/used (if in UPC box)
function checkpress(obj,e,code,isupc)
{
	if (!code)
	{
		if (window.event) { code = window.event.keyCode; }
		else if (e) { code = e.which; }
		else { return true; }
	}

	if (code != 13 && code != 122) { clear_scanned(); }

	if (code == 13)
	{
		// enter was pressed - check entered UPC and increment qty
		var val = obj.value;

		if (!isupc)
		{
			// perform title search
			add(val,1,obj,0,false);
		}
		else
		{
			// perform the UPC search
			add(val,1,obj,0,true);
		}
		return false;
	}
	else if (isupc && ((code == 78 || code == 110) || (code == 85 || code == 117)))
	{
		// change radio to new/used when 'n' or 'u' pressed
		document.upc.newused[((code==78||code==110)?0:1)].checked = true;
		return false;
	}
	else if (isupc && code == 122)
	{
		// undo last quantity change
		if (lastval.length)
		{
			add(lastval,-1,lastobj,lastnewused,lastisupc);
			lastval = '';
			lastnewused = 0;
		}

		return false;
	}
	else { return true; }
}

// add an amount to the quantity for the given UPC/title
function add(val,amount,obj,nu,isupc)
{
	if (val.length)
	{
		var idx = -1;
		var upc = '';
		var multi = false;

		if (isupc)
		{
			upc = val;
			idx = array_search(upc,upcs);
		}
		else
		{
			for (var i=0; i<striptitles.length; i++)
			{
				var stripval = stripString(val);
				if (striptitles[i].toLowerCase().indexOf(stripval.toLowerCase()) > -1)
				{
					if (idx == -1) { idx = i; }
					else { multi = true; break; }
				}
			}
		}

		if (!multi && idx > -1)
		{
			// item found - increase quantity
			var itemID = itemIDs[idx];
			var title = titles[idx];
			scan_upc.innerText = upc;
			scan_title.innerText = title;

			var frm = document.itmfrm;
			var newused = (!nu?(document.upc[0].checked==true?<?=ITEM_NEW;?>:<?=ITEM_USED;?>):nu);
			var qtyobj = eval("frm.elements['setqtys["+itemID+"]["+newused+"]']");
			qtyobj.value = (qtyobj.value*1)+amount;
			qtyobj.focus();
			scan_qty.innerText = (newused==<?=ITEM_NEW;?>?'New':'Used')+' - '+qtyobj.value;
			if (isupc) { obj.value = ''; }
			add.obj = obj;
			setTimeout('add.obj.select()',500);

			play_sound(qtyobj.value); // beep X times

			lastval = val;
			lastobj = obj;
			lastnewused = newused;
			lastisupc = isupc;
			totmultiple = 0;
		}
		else
		{
			if (isupc)
			{
				scan_upc.innerText = upc;
				scan_title.innerHTML = '&nbsp;&nbsp;&nbsp;&nbsp;<font color="red"><b>UNKNOWN UPC!</b></font>';
				check_other_platform(upc,(document.upc[0].checked==true?<?=ITEM_NEW;?>:<?=ITEM_USED;?>));
			}
			else
			{
				if (!multi)
				{
					alert('Unable to find an item matching title: '+val);
					obj.select();
				}
				else
				{
					alert('Multiple items for title "'+val+'" on line '+idx);
					totmultiple++;

					if (totmultiple == 3)
					{
						totmultiple = 0;
						var obj2 = document.getElementById('q'+itemIDs[idx]+'<?=ITEM_USED;?>');
						obj2.focus();
						window.scrollBy(0,100);
					}
				}
			}
		}
	}
}

// go to the add new item page
function newitem(upc)
{
	lock_upc();
	var frm = document.itmfrm;
	frm.tonewitem.value = <?=YES;?>;
	frm.newitemupc.value = upc;
	frm.submit();
}

// return to the platform selection page
function changeplatform()
{
	var frm = document.itmfrm;
	frm.tochangeplatform.value = <?=YES;?>;
	frm.submit();
}

// check if the UPC exists in another platform
function check_other_platform(upc,nu)
{
	lock_upc();
	checkupcifrm.location = '/admin/setup_items/begqty_checkupc.php?platformID=<?=$platformID;?>&upc='+upc+'&newused='+nu;
}

// lock the UPC textbox
locked = false;
function lock_upc()
{
	var frm = document.upc;
	frm.upc.value = '- Please Hold -';
	frm.upc.style.textAlign = 'center';
	frm.upc.disabled = true;
	frm.title.disabled = true;

	locked = true;
}

// unlock the UPC textbox
function unlock_upc()
{
	clear_scanned();

	var frm = document.upc;
	frm.upc.value = '';
	frm.upc.style.textAlign = 'left';
	frm.upc.disabled = false;
	frm.title.disabled = false;
	frm.upc.focus();

	locked = false;
}

// ask if they would like to add a new item
// if YES, go to the add new item page
function ask_addnew(upc)
{
	if (confirm('UPC number '+upc+' not found.\n\nWould you like to add this item?\n\nPlease verify that none of the items on this page match this UPC!\n-------------------------\nOK/Enter = YES\nCancel/ESC = no')) { newitem(upc); return; }
	unlock_upc();
}

// set the UPC for an item
function set_upc(idx,fillupc)
{
	var itemID = itemIDs[idx];
	var title = titles[idx];
	var upc = prompt('Please enter the UPC for the item titled: '+title,fillupc);

	if (upc != null)
	{
		var existsidx = array_search(upc,upcs);

		if (upc == '' || existsidx == -1)
		{
			lock_upc();
			setupcifrm.location = '/admin/setup_items/begqty_setupc.php?idx='+idx+'&itemID='+itemID+'&upc='+upc;
		}
		else
		{
			upc_exists(upc,title);
		}
	}
}

// alert that the UPC is already set
function upc_exists(upc,title,platform)
{
	if (platform == null) { platform = '<?=mysql_real_escape_string($platform_name);?>'; }
	alert('The UPC '+upc+' already exists\n\nPlatform: '+platform+'\nItem: '+title);
}

// change the shown UPC for a given item
function change_upc(idx,upc)
{
	var obj = document.getElementById('upc'+idx);
	obj.innerText = upc;
	unlock_upc();
	upcs[idx] = upc;
}

// add an amount to the quantity in the given field
function add_amt(idx,amt)
{
	var obj = document.getElementById(idx);
	obj.value = parseInt(obj.value)+amt;
	if (obj.value > 0) { play_sound(obj.value); }
	if (parseInt(obj.value) < 0) { obj.value = 0; }
	if (lastfocus) { lastfocus.select(); }
}

// when the window is scrolled, move the floating UPC box to the middle of the left of the window
// (fixed content code: http://forums.devshed.com/archive/t-43600)
window.onscroll = move_upc;
function move_upc(forceupcfocus)
{
	if (forceupcfocus) { lastfocus = document.upc.upc; }

	if (typeof move_upc.timer == 'undefined') { move_upc.timer = null; }
	if (move_upc.timer) { clearTimeout(move_upc.timer); }
	obj = document.getElementById('upctable');
	if (obj && obj.style)
	{
		obj.style.visibility = 'hidden';
		obj.style.left = 5+getScrollLeft();
		obj.style.top = getScrollTop()+((screen.availHeight/2)-parseInt(obj.style.height))+(screen.availHeight/4);
		move_upc.timer = setTimeout("document.getElementById('upctable').style.visibility='visible'"+(lastfocus!=null?';lastfocus.select()':''),200);
	}
}

function getScrollLeft() { return (document.body&&typeof document.body.scrollLeft!='undefined'?document.body.scrollLeft:(window.pageXOffset?pageXOffset:null)); }
function getScrollTop() { return (document.body&&typeof document.body.scrollTop!='undefined'?document.body.scrollTop:(window.pageYOffset?pageYOffset:null)); }
