<!--
// Functionality for Fun Unlimited Online
// All code copyright � 2003-2004 Scott Carpenter [s-carp@comcast.net]

// set the browser type
var isNS = false;
var isIE = false;
var isOther = false;
var browser = navigator.appName;
var browserVersion = parseInt(navigator.appVersion);

if (browser.indexOf("Netscape") >= 0) { isNS = true; }
else if (browser.indexOf("Microsoft") >= 0) { isIE = true; }
else { isOther = true; }

// open a popup window
function open_window(url,winname,width,height,scroll,maximize)
{
	// append '[?/&]ispopup=1' to URL
	url = url+(url.indexOf('?')==-1?'?':'&')+'ispopup=1';

	if (maximize)
	{
		// maximize the window size to fill the entire screen
		l = 0;
		t = 0;
		width = screen.availWidth;
		height = screen.availHeight;
	}
	else
	{
		// center the window
		l = (screen.width/2)-(width/2);
		t = (screen.height/2)-(height/2);
	}

	var win = window.open(url,winname,'width='+width+',height='+height+',top='+t+',left='+l+',resizable=no,scrollbars='+scroll+',toolbar=no,location=no,directories=no,status=YES,menubar=no');
	if (win != null) { win.focus(); win.resizeTo(width,height); }
}

// return the non-hidden form object which precedes a given object
function getprev(obj) {
	var frm = obj.form;
	var j = 0;
	var keepgoing = true;

	for (var i=0; i<frm.elements.length; i++) {
		if (obj == frm.elements[i])
		{
			for (var j=(i-1); keepgoing; j--)
			{
				if (j < 0) { j = 0; keepgoing = false; break; }
				if (frm.elements[j].type != 'hidden') { keepgoing = false; break; }
			}
		}
		if (!keepgoing) { break; }
	}

	if (frm.elements[j].type != 'hidden') { return frm.elements[j]; }
	else { return -1; }
}

// return the non-hidden form object which follows a given object
function getnext(obj) {
	var frm = obj.form;
	var j = (frm.elements.length-1);
	var keepgoing = true;

	for (var i=0; i<frm.elements.length; i++) {
		if (obj == frm.elements[i])
		{
			for (var j=(i+1); keepgoing && j<frm.elements.length; j++)
			{
				if (frm.elements[j].type != 'hidden') { keepgoing = false; break; }
			}
		}
		if (!keepgoing) { break; }
	}

	if (frm.elements[j].type != 'hidden') { return frm.elements[j]; }
	else { return -1; }
}

// if the user hits enter on a form element, go to the next element instead of submitting the form
function checkenter(obj,e)
{
	var code;
	if (window.event) { code = window.event.keyCode; }
	else if (e) { code = e.which; }
	else { return true; }

	var shift = window.event.shiftKey;
	if (code == 13)
	{
		var nobj = (shift?getprev(obj):getnext(obj));
		if (nobj != -1) { nobj.select(); }
		return false;
	}
	else
	{
		return true;
	}
}

// if the user hits backspace on a form element, ignore
function checkbackspace(obj,evt)
{
	var code;
	if (window.event) { code = window.event.keyCode; }
	else if (evt) { code = evt.which; }
	else { return true; }

	return !(code == 8);
}

// only allow numbers in the textbox
// if allowzero is false, don't allow a zero as the first number
// if nextifenter is true and enter is pressed, go to the next field
function onlynumbers(val,e,allowzero,nextifenter)
{
	var code;
	if (window.event) { code = window.event.keyCode; }
	else if (e) { code = e.which; }
	else { return true; }

	var range = document.selection.createRange();
	var seltext = range.htmlText;

	if ((code >= 48 && code <= 57) || code == 46) // a number or a period
	{
		if (code == 46)
		{
			if (val.indexOf('.') == -1 || seltext.length) { return true; } else { return false; }
		}
		else if (allowzero == true || (allowzero == false && (val.length || (!val.length && code != 48))))
		{
			// if the first number is a zero, there is no period, and they typed a zero, return false
			// otherwise, return true
			var first = val.substring(0,1);
			if (first == '0' && val.indexOf('.') == -1 && code == 48) { return false; }
			else { return true; }
		}
		else { return false; }
	}
	else if (code == 13) // enter key
	{
		if (nextifenter) { return checkenter(window.event.srcElement,e); }
		else { return true; }
	}
	else if (code == 45) // '-'
	{
		// do not allow a hyphen unless it is the first character
		if (!val.length || seltext == val) { return true; }
		else { return false; }
	}
	else { return false; }
}

// returns true if the needle (string) is in the haystack (array)
function in_array(needle,haystack)
{
	var isin = false;

	for (var i=0; i<haystack.length; i++)
	{
		if (haystack[i] == needle) { isin = true; break; }
	}

	return isin;
}

// returns the array position of needle (string) in haystack (array)
function array_search(needle,haystack)
{
	var ret = -1;

	for (var i=0; i<haystack.length; i++)
	{
		if (haystack[i] == needle) { ret = i; break; }
	}

	return ret;
}

// removes an element from an array and returns the new array
function array_remove(needle,arr)
{
	var newarr = new Array();
	for (var i=0; i<arr.length; i++)
	{
		if (arr[i] != needle) { newarr[newarr.length] = arr[i]; }
	}
	return newarr;
}

// !!! JAVASCRIPT HAS <array>.join('glue') !!!
// implode an array using the given string as 'glue' (same function as PHP implode())
function implode(glue,pieces)
{
	var str = '';

	for (var i=0; i<pieces.length; i++)
	{
		str += (i?glue:'')+pieces[i];
	}

	return str;
}

// takes a number and formats it as a price ([-]N.NN)
function format_price(n,zeroifblank)
{
	if (zeroifblank == null) { zeroifblank = true; }

	if ((n == '' || n == '-') && zeroifblank) { return '0.00'; }
	else if ((n == '' || n == '-') && !zeroifblank) { return ''; }
	else
	{
		var s = ''+Math.round(n*100)/100;
		var i = s.indexOf('.');
		if (i<0) { return s+'.00'; }
		var t = s.substring(0,i+1)+s.substring(i+1,i+3);
		if ((i+2) == s.length) { t += '0'; }
		return t;
	}
}

// find the Nth occurance of a string in another string (returns the index)
function strpos(str,fnd,n)
{
	var pos = -1;
	var totmatches = 0;
	var chr = '';

	for (var i=0; i<str.length; i++)
	{
		chr = str.substring(i,(i+1));
		if (chr == fnd)
		{
			totmatches++;
			if (totmatches == n) { pos = i; break; }
		}
	}

	return pos;
}

// return the Nth token in the string, separated by a separator
function token(str,sep,n)
{
	if (str.substring(0,1) != sep) { str = sep+str; }
	if (str.substring((str.length - 1),1) != sep) { str = str + sep; }

	var pos1 = (n==1 && 0 ? 0 : strpos(str,sep,n) + 1);
	var pos2 = strpos(str,sep,(n + 1));

	return str.substr(pos1,(pos2 - pos1));
}

// returns true if the given date is valid (mm/dd/yyyy)
function validDate(d)
{
	if (d.length > 0)
	{
		var dateregex = /^[ ]*[0]?(\d{1,2})\/(\d{1,2})\/(\d{2,4})[ ]*$/;
		var match = d.match(dateregex);
		
		if (match && match[3].length != 3)
		{
			match[3] = makeYear(match[3]);
			var tmpdate = new Date(match[3], parseInt(match[1], 10) - 1, match[2]);
			
			if
			(
				tmpdate.getDate() == parseInt(match[2], 10)
				&& tmpdate.getFullYear() == parseInt(match[3], 10)
				&& (tmpdate.getMonth() + 1) == parseInt(match[1], 10)
			)
			{
				return true;
			}
		}
		
		return false;
	}
	else
	{
		return false;
	}
} // end function validDate

// make 2-character years into 4-character years (IE: 83 = 1983, 04 = 2004)
function makeYear(str) {
	var year = ''+parseInt(str);
	if (year.length == 4) { return year; }
	else if (year >= 25) { return '19'+year; }
	else { return '20'+(year<10?'0':'')+year; }
}

// return the number of days between two Unix timestamps
function daysBetween(first,last,count_sundays)
{
	if (last == 'now')
	{
		var dobj = new Date();
		last = Math.floor(dobj.getTime()/1000);
	}

	first = first*1000; // JS date object is in milliseconds, not seconds
	last = last*1000;

	// set first/last times to midnight
	var dobj = new Date();
	dobj.setTime(first); dobj.setHours(0); dobj.setMinutes(0); dobj.setSeconds(0);
	first = dobj.getTime();
	dobj.setTime(last); dobj.setHours(0); dobj.setMinutes(0); dobj.setSeconds(0);
	last = dobj.getTime();

	var dayseconds = (60*60*24)*1000;
	var diff = (last-first);
	var days = (diff/dayseconds);

	if (!count_sundays)
	{
		// remove Sundays
		for (var i=first; i<=last; i+=dayseconds)
		{
			dobj.setTime(i);
			if (!dobj.getDay()) { days--; }
		}
	}

	return days;
}

// resize a popup window to fit the screen
function resize_popup()
{
	var aw = screen.availWidth;
	var ah = screen.availHeight;

	if (aw != screen.width || ah != screen.height) {
		window.resizeTo(aw,ah);
		window.moveTo(0,0);
	}
}

// return the largest of two numbers
function largest(num1,num2) { return (num1>num2?num1:num2); }

// display the document size
function showDocSize()
{
	var size = format_bytes(document.body.innerHTML.length);
	var obj = $('docsize');
	if (obj) { obj.innerText = size; }
}

// return the highest format according to a filesize (IE: 1.5kb, 1023.9kb, 1.99mb, etc)
function format_bytes(bytes)
{
	if (bytes >= (1024*1024)) { return roundto2(bytes/(1024*1024))+'mb'; }
	else if (bytes >= 1024) { return roundto2(bytes/1024)+'kb'; }
	else { return bytes+'b'; }
}

// these could (and should) be done with powers, but I'm lazy right now...
function roundto2(num) { return (Math.round(num*100)/100); }
function roundto3(num) { return (Math.round(num*1000)/1000); }

function showDisplayTime(start)
{
	return; // !!! TEMPORARY !!!
	var obj = $('docdisplaytime');
	if (obj)
	{
		var d = new Date();
		var end = d.getTime();
		var diff = Math.abs((end - start) / 1000);
		obj.innerText = roundto3(diff) + 's';
	}
}

// redirect to the updates page
function gotoUpdates(num)
{
	if (confirm('There ' + (num!=1 ? 'are' : 'is') + ' ' + num + ' software update' + (num!=1 ? 's' : '') + ' that need to be applied to your store.\nPress OK to update the store software.\nNOTE: IF YOU CANCEL THIS, YOU *WILL* BE ASKED AGAIN!'))
	{
		document.location = '/admin/update.php';
	}
}

// open the sales graphs modal dialog
function openSalesGraphs()
{
	width = Math.floor(screen.width * 0.99);
	if (width > 1000) { width = 1000; }

	height = Math.floor(screen.height * 0.99);
	if (height > 850) { height = 850; }

	var url = '/admin/reports/sales_graphs.php?popup=1';
	if (0)
	{
		window.showModalDialog(
			url,
			'',
			'dialogheight:' + height + 'px;dialogwidth:' + width + 'px;edge:raised;center:YES;help:no;resizable:YES;status:YES'
		);
	}
	else
	{
		open_window(url,'graphs',width,height);
	}
}

// JS version of PHP's htmlspecialchars() [see http://us2.php.net/manual/en/function.htmlspecialchars.php]
function htmlspecialchars(string)
{
	string = replaceSubstring(string,'&','&amp;');
	string = replaceSubstring(string,'"','&quot;');
	string = replaceSubstring(string,"'",'&#039;');
	string = replaceSubstring(string,'<','&lt;');
	string = replaceSubstring(string,'>','&gt;');

	return string;
}

// replace a substring
function replaceSubstring(text,rep,repwith)
{
	var exp = new RegExp(rep,'g')
	return text.replace(exp,repwith)
}

// VERY basic time formatting
function date_format(format,unixtime)
{
	if (!unixtime)
	{
		// if unixtime is not passed, use the time right now
		var d = new Date();
		unixtime = Math.floor(d.getTime()/1000);
	}

	var time = (unixtime*1000);
	var d = new Date(time);

	var h = (d.getHours()+1);
	if (h > 12 && h != 24) { var ampm = 'pm'; h -= 12; }
	else { var ampm = 'am'; }

	format = replaceSubstring(format,'m',(d.getMonth()+1));
	format = replaceSubstring(format,'d',d.getDate());
	format = replaceSubstring(format,'y',d.getYear());
	format = replaceSubstring(format,'Y',makeYear(d.getYear()));

	format = replaceSubstring(format,'h',h);
	format = replaceSubstring(format,'i',d.getMinutes());
	format = replaceSubstring(format,'a',ampm);

	return format;
}

var errfound = false;
function err(txt,obj)
{
	errfound = true;
	alert(txt);

	if (typeof(obj) == 'object')
	{
		obj.focus();
		if (obj.type != 'select-one' && obj.type != 'button') { obj.select(); }
	}
}

function go(url) { document.location = url; }

function disableButton(obj)
{
	obj.value = '- Please Hold -';
	obj.disabled = true;
	obj.form.submit();
}

/**
 * Return true/false if any checkboxes are checked with the given classname
 * @param  mixed  arg1    either a classname or the parent
 * @param  mixed  arg2    classname if arg1 is the parent
 */
function anyChecked(arg1, arg2)
{
	use_parent = (arguments.length==2 ? arg1 : document);
	cls = (arguments.length==2 ? arg2 : arg1);
	return $(use_parent).getElementsByClassName(cls).pluck('checked').any();
}

-->