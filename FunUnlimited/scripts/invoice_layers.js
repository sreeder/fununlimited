// Invoice option layer handling
// All code copyright � 2003-2004 Scott Carpenter [s-carp@comcast.net]
// Version: 10/05/2012

var posX = 0;
var posY = 0;
var width = 0;
var height = 0;
var lineheight = 12;
var curoptID = 0;
var curlinkID = 0;
var cleartime = 1000;
var timer;
var lock_timer;
var prc_timer;

// option arrays (image filename:display text:URL)
// possible identifiers in URL:
//  %idx% (which # on the invoice it is - IE: 3rd in trade)
//  %itemID%
//  %platformID%
//  %type% (SALE/TRADE)
//  %trade_type% (CASH/CREDIT/NEITHER)
//  %image% (option image)
var opts1 = new Array(
	"nu-new.gif:New:change_newused(%type%,%x%,%idx%,'%image%',%linkID%," + ITEM_NEW + ",%itemID%)",
	"nu-used.gif:Used:change_newused(%type%,%x%,%idx%,'%image%',%linkID%," + ITEM_USED + ",%itemID%)"
);
var opts2 = new Array(
	"ccn-cash.gif:Cash:change_ccn(%type%,%x%,%idx%,'%image%',%linkID%," + CASH + ",%itemID%,true)",
	"ccn-credit.gif:Credit:change_ccn(%type%,%x%,%idx%,'%image%',%linkID%," + CREDIT + ",%itemID%,true)",
	"ccn-neither.gif:Neither:change_ccn(%type%,%x%,%idx%,'%image%',%linkID%," + NEITHER + ",%itemID%,true)"
);
var opts3 = new Array(
	"bns-box.gif:Box:change_box(%type%,%x%,%idx%,'%image%',%linkID%," + BOX + ",%itemID%)",
	"bns-nobox.gif:No Box:change_box(%type%,%x%,%idx%,'%image%',%linkID%," + NOBOX + ",%itemID%)",
	"bns-storebox.gif:Store Printed Box:change_box(%type%,%x%,%idx%,'%image%',%linkID%," + STOREBOX + ",%itemID%)"
);
var opts4 = new Array(
	"bns-box.gif:Box:change_box(%type%,%x%,%idx%,'%image%',%linkID%," + BOX + ",%itemID%)",
	"bns-nobox.gif:No Box:change_box(%type%,%x%,%idx%,'%image%',%linkID%," + NOBOX + ",%itemID%)"
);
var opts5 = new Array(
	"gfp-new.gif:New:change_gfp(%type%,%x%,%idx%,'%image%',%linkID%," + CNEW + ",%itemID%)",
	"gfp-good.gif:Good:change_gfp(%type%,%x%,%idx%,'%image%',%linkID%," + GOOD + ",%itemID%)",
	"gfp-fair.gif:Fair:change_gfp(%type%,%x%,%idx%,'%image%',%linkID%," + FAIR + ",%itemID%)",
	"gfp-poor.gif:Poor:change_gfp(%type%,%x%,%idx%,'%image%',%linkID%," + POOR + ",%itemID%)"
);
var opts6 = new Array(
	"ou-opened.gif:Opened:change_open(%type%,%x%,%idx%,'%image%',%linkID%," + OPENED + ",%itemID%)",
	"ou-unopened.gif:Unopened:change_open(%type%,%x%,%idx%,'%image%',%linkID%," + UNOPENED + ",%itemID%)"
);
var opts7 = new Array(
	"ccn-cash.gif:Cash:change_ccn(%type%,%x%,%idx%,'%image%',%linkID%," + CASH + ",%itemID%,true)",
	"ccn-credit.gif:Credit:change_ccn(%type%,%x%,%idx%,'%image%',%linkID%," + CREDIT + ",%itemID%,true)"
);
var opts8 = new Array(
	"cn-charge.gif:Charged:change_charged(%type%,%x%,%idx%,'%image%',%linkID%," + YES + ",%itemID%)",
	"cn-nocharge.gif:Not Charged:change_charged(%type%,%x%,%idx%,'%image%',%linkID%," + NO + ",%itemID%)"
);
var opts9 = new Array(
	"ou-opened.gif:Opened:change_open(%type%,%x%,%idx%,'%image%',%linkID%," + OPENED + ",%itemID%)",
	"ou-unopened.gif:Unopened:change_open(%type%,%x%,%idx%,'%image%',%linkID%," + UNOPENED + ",%itemID%)",
	"ou-broken.gif:Broken:change_open(%type%,%x%,%idx%,'%image%',%linkID%," + BROKEN + ",%itemID%)"
);
var opts10 = new Array(
	"ncb-none.gif:None:change_occasion(%type%,%x%,%idx%,'%image%',%linkID%," + NONE + ",%itemID%)",
	"ncb-birthday.gif:Birthday:change_occasion(%type%,%x%,%idx%,'%image%',%linkID%," + BIRTHDAY + ",%itemID%)",
	"ncb-christmas.gif:Christmas:change_occasion(%type%,%x%,%idx%,'%image%',%linkID%," + CHRISTMAS + ",%itemID%)"
);
var opts11 = new Array(
	"serial-num.gif:Set Serial #:set_serial_number(%type%,%x%,%idx%,'%image%',%linkID%,false,%itemID%)"
);

// set the mouse movement handler
if (document.layers)
{
	document.captureEvents(event.MOUSEMOVE);
	document.onmousemove = mouseXY;
}
else if (document.all)
{
	document.onmousemove = mouseXY;
}
else if (document.getElementById)
{
	document.onmousemove = mouseXY;
}

// sets posX/posY when the mouse is moved
function mouseXY(e)
{
	if (document.layers)
	{
		posX = e.pageX;
		posY = e.pageY;
		width = window.innerWidth + window.pageXOffset;
		height = window.innerHeight + window.pageYOffset;
	}
	else if (document.all)
	{
		posX = window.event.x + document.body.scrollLeft;
		posY = window.event.y + document.body.scrollTop;
		width = document.body.clientWidth + document.body.scrollLeft;
		height = document.body.clientHeight + document.body.scrollTop;
	}
	else if (document.getElementById)
	{
		posX = e.pageX;
		posY = e.pageY;
		width = window.innerWidth + window.pageXOffset;
		height = window.innerHeight + window.pageYOffset;
	}
}

// show/hide the layer; if showing, display the options, resize the layer, and position it
var inlyr = false;
function show_layer(optID,linkID,x,idx,itemID,platformID,type,trade_type)
{
	if (checklocked() == false)
	{
		var obj = optslyr;
		if (!optID) { optID = (Math.floor(Math.random()*3)+1); }
		if (linkID != curlinkID) { hide_layer(); }
		curlinkID = linkID;
		curoptID = optID;

		clearTimeout(timer);

		var vis = '';
		if (obj.style.visibility == 'hidden')
		{
			inlyr = true;
			vis = 'visible';
		}
		else
		{
			inlyr = false;
			vis = 'hidden';
		}
		obj.style.visibility = 'hidden';
		obj.style.visibility = vis;

		if (obj.style.visibility == 'visible')
		{
			var opts = eval('opts'+optID);
			obj.innerHTML = buildOptsHTML(opts,linkID,x,idx,itemID,platformID,type,trade_type);

			var t = (posY-((lineheight*opts.length)/2)-(lineheight/2));
			var l = (posX-(parseFloat(obj.style.width)/2));

			if (t < 1) { t = 1; } // too far up
			if (l < 1) { l = 1; } // too far left
			if (t > (height-40)) { t = (height-40); } // too far down
			if ((l+parseFloat(obj.style.width)) > width)
			{
				// too far right
				l = (width - parseFloat(obj.style.width));
			}

			obj.style.height = 1;
			obj.style.top = t;
			obj.style.left = l;
		}
	}
}

document.onclick = bodyclickhide;
function bodyclickhide()
{
	if (!inlyr)
	{
		hide_layer();
	}
}
function hide_layer()
{
	optslyr.style.visibility = 'hidden';
}

// builds the HTML for the options layer
function buildOptsHTML(opts,linkID,x,idx,itemID,platformID,type,trade_type)
{
	var html = '<div style="text-align:left;width:1px"><nobr>';

	for (var i=0; i<opts.length; i++)
	{
		var img = token(opts[i],':',1);
		var text = token(opts[i],':',2);
		var url = token(opts[i],':',3);

		url = url.replace('%linkID%',linkID);
		url = url.replace('%x%',x);
		url = url.replace('%idx%',idx);
		url = url.replace('%itemID%',itemID);
		url = url.replace('%platformID%',platformID);
		url = url.replace('%type%',type);
		url = url.replace('%trade_type%',trade_type);
		url = url.replace('%image%',img);

		html = html + (i ? '<br />' : '') + '<a href="javascript:' + url + '" class="optbox"><img src="/images/invoice/' + img + '" border="0" style="vertical-align:middle"> ' + text + '</a>';
	}

	html = html + '</nobr></div>';

	return html;
}

// chnage the option image
function change_image(linkID,image)
{
	document.getElementById('optimg' + linkID).src = '/images/invoice/' + image;
}

//
function change_item_image(itype, idx, file)
{
	$('itemimg' + itype + idx).src = file;
}

function mousein()
{
	inlyr=true;
	clearTimeout(timer);
}
function mouseout()
{
	inlyr=false;
	timer = setTimeout('hide_layer()',cleartime);
}

/* END OF FILE */