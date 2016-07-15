// Functionality for Fun Unlimited Store Management Point-of-Sale Invoice
// All code copyright � 2003-2004 Scott Carpenter [s-carp@comcast.net]
// Version: 10/05/2012

var all_types = [SALE,TRADE,RETURNS]; // sale, trade, return

/**
* table header/footer
*/
var table_head = '<table cellspacing="1" cellpadding="3" width="100%" class="bb" id="datatable%type%">';
var table_foot = '</table>';

/**
* data idx order arrays are initialized/set in /scripts/invoice.js
*/

/**
* functions
* note: the function argument 'type' is either 23, 24, or 25 (sales, trades, and returns, respectively)
*/

/**
* Return the table header for type (<tr>...</tr>)
*/
function buildHeader(type)
{
	var vals = eval('header' + type + '_vals');

	var data = '<tr bgcolor="#DDDDDD" class="bb">';

	for (var i=0; i<vals.length; i++)
	{
		var sv = vals[i].split('|');
		var val = sv[0];
		var align = sv[1];
		var width = sv[2];
		if (typeof align != 'undefined')
		{
			var alignstring = ' align="' + align + '"';
		}
		else
		{
			var alignstring = '';
		}
		if (typeof width != 'undefined')
		{
			var widthstring = ' width="' + width + '"';
		}
		else
		{
			var widthstring = '';
		}

		data += '<td' + widthstring + alignstring + '><b>' + val + '</b></td>';
	}

	data += '</tr>';

	return data;
}

/**
* Return the table footer for type (<tr>...</tr>)
*/
function buildFooter(type)
{
	var total_colspan = (type==SALE ? 8 : (type==TRADE ? 7 : 9));

	var data = '<tr bgcolor="' + color_label + '" class="bb">';

	data += '<td colspan="' + total_colspan + '" align="right"><b>Totals:</b></td>';

	if (type == SALE)
	{
		data += '<td align="right"><b><div id="prctot' + SALE + '0">&nbsp;</div></b></td>';
	}
	else if (type == TRADE)
	{
		data += '<td><div id="alltotalprc' + CASH + '" style="text-align:right;color:#AAAAAA">&nbsp;</div></td>';
		data += '<td><div id="alltotalprc' + CREDIT + '" style="text-align:right;color:#AAAAAA">&nbsp;</div></td>';
		data += '<td>&nbsp;</td>';
		data += '<td align="right"><b><div id="prctot' + TRADE + CASH + '">&nbsp;</div></b></td>';
		data += '<td align="right"><b><div id="prctot' + TRADE + CREDIT + '">&nbsp;</div></b></td>';
	}
	else if (type == RETURNS)
	{
		data += '<td align="right"><b><div id="prctot' + RETURNS + '0">&nbsp;</div></b></td>';
	}

	data += '</tr>';

	return data;
}

/**
* Return the data for idx of type (<tr>...</tr>)
*/
function buildData(type,idx,x)
{
	var data = '<!--idx' + idx + '-->'; // the escapes stop syntax highlighting from messing up
	data += '<tr bgcolor="' + color_cell + '" class="bb">';

	// data used on each row
	data += '<td><input type="checkbox" id="rem'+type+idx+'" onclick="add_remove('+type+','+x+','+idx+',this.checked)" class="nb" title="Check to remove this item" /></td>';
	data += '<td>';

	// output options
	if (type == SALE)
	{
		itemID = sale_itemIDs[idx];
		platform_name = sale_platform_names[idx];
		platform_abbr = sale_platform_abbrs[idx];
		title = sale_titles[idx];
		qty = sale_invoice_qtys[idx];

		this_box = sale_box[idx];
		image = (this_box==NOBOX ? sale_images_nobox[idx] : sale_images_box[idx]);
		if (!image.length && sale_images_box[idx].length)
		{
			image = sale_images_box[idx];
		}

		data += '<a href="javascript:show_layer(1,'+(totlinks+1)+','+x+','+idx+','+itemID+','+sale_platformIDs[idx]+','+type+',0)"><img src="/images/invoice/nu-'+(sale_newused[idx]==ITEM_NEW ? 'new' : 'used')+'.gif" id="optimg'+(totlinks+1)+'" border="0" alt="New/Used"></a> ';
		data += '<a href="javascript:show_layer(3,'+(totlinks+2)+','+x+','+idx+','+itemID+','+sale_platformIDs[idx]+','+type+',0)"><img src="/images/invoice/bns-'+(this_box==BOX ? 'box' : (this_box==NOBOX ? 'nobox' : 'storebox'))+'.gif" id="optimg'+(totlinks+2)+'" border="0" alt="Box Type"></a>';
		data += '<a href="javascript:show_layer(11,'+(totlinks+3)+','+x+','+idx+','+itemID+','+sale_platformIDs[idx]+','+type+',0)"><img src="/images/invoice/serial-num.gif" id="optimg'+(totlinks+3)+'" border="0" alt="Set Serial #"></a>';

		totlinks += 2;
	}
	else if (type == TRADE)
	{
		itemID = trade_itemIDs[idx];
		platform_name = trade_platform_names[idx];
		platform_abbr = trade_platform_abbrs[idx];
		title = trade_titles[idx];
		qty = trade_invoice_qtys[idx];

		this_box = trade_box[idx];
		image = (this_box==NOBOX ? trade_images_nobox[idx] : trade_images_box[idx]);
		if (!image.length && trade_images_box[idx].length)
		{
			image = trade_images_box[idx];
		}

		data += '<a href="javascript:show_layer(2,'+(totlinks+1)+','+x+','+idx+','+itemID+','+trade_platformIDs[idx]+','+type+','+trade_types[idx]+')"><img src="/images/invoice/ccn-'+(trade_types[idx]==CASH ? 'cash' : (trade_types[idx]==CREDIT ? 'credit' : 'neither'))+'.gif" id="optimg'+(totlinks+1)+'" border="0" alt="Price Type"></a> ';
		data += '<a href="javascript:show_layer(4,'+(totlinks+2)+','+x+','+idx+','+itemID+','+trade_platformIDs[idx]+','+type+','+trade_types[idx]+')"><img src="/images/invoice/bns-'+(this_box==BOX ? 'box' : (this_box==NOBOX ? 'nobox' : 'storebox'))+'.gif" id="optimg'+(totlinks+2)+'" border="0" alt="Box Type"></a> ';
		data += '<a href="javascript:show_layer(5,'+(totlinks+3)+','+x+','+idx+','+itemID+','+trade_platformIDs[idx]+','+type+','+trade_types[idx]+')"><img src="/images/invoice/gfp-'+(trade_condition[idx]==GOOD ? 'good' : (trade_condition[idx]==FAIR ? 'fair' : (trade_condition[idx]==POOR ? 'poor' : 'new')))+'.gif" id="optimg'+(totlinks+3)+'" border="0" alt="Condition"></a>';
		data += '<a href="javascript:show_layer(11,'+(totlinks+4)+','+x+','+idx+','+itemID+','+trade_platformIDs[idx]+','+type+','+trade_types[idx]+')"><img src="/images/invoice/serial-num.gif" id="optimg'+(totlinks+4)+'" border="0" alt="Set Serial #"></a>';

		totlinks += 3;
	}
	else if (type == RETURNS)
	{
		itemID = return_itemIDs[idx];
		platform_name = return_platform_names[idx];
		platform_abbr = return_platform_abbrs[idx];
		title = return_titles[idx];
		qty = return_invoice_qtys[idx];
		image = return_images_box[idx];

		data += '<a href="javascript:show_layer(7,'+(totlinks+1)+','+x+','+idx+','+itemID+','+return_platformIDs[idx]+','+type+','+return_types[idx]+')"><img src="/images/invoice/ccn-'+(return_types[idx]==CREDIT ? 'credit' : 'cash')+'.gif" id="optimg'+(totlinks+1)+'" border="0" alt="Price Type"></a> ';
		data += '<a href="javascript:show_layer(1,'+(totlinks+2)+','+x+','+idx+','+itemID+','+return_platformIDs[idx]+','+type+','+return_types[idx]+')"><img src="/images/invoice/nu-'+(return_newused[idx]==ITEM_NEW ? 'new' : 'used')+'.gif" id="optimg'+(totlinks+2)+'" border="0" alt="Condition When Purchased"></a> ';
		data += '<a href="javascript:show_layer(9,'+(totlinks+3)+','+x+','+idx+','+itemID+','+return_platformIDs[idx]+','+type+','+return_types[idx]+')"><img src="/images/invoice/ou-'+(return_opened[idx]==UNOPENED ? 'unopened' : (return_opened[idx]==OPENED ? 'opened' : 'broken'))+'.gif" id="optimg'+(totlinks+3)+'" border="0" alt="Opened/Unopened/Broken/Christmas/Birthday"></a> ';
		data += '<a href="javascript:show_layer(8,'+(totlinks+4)+','+x+','+idx+','+itemID+','+return_platformIDs[idx]+','+type+','+return_types[idx]+')"><img src="/images/invoice/cn-'+(return_charged[idx]==YES ? 'charge' : 'nocharge')+'.gif" id="optimg'+(totlinks+4)+'" border="0" alt="Charged/Not Charged"></a>';
		data += '<a href="javascript:show_layer(10,'+(totlinks+5)+','+x+','+idx+','+itemID+','+return_platformIDs[idx]+','+type+','+return_types[idx]+')"><img src="/images/invoice/ncb-'+(return_occasion[idx]==NONE ? 'none' : (return_occasion[idx]==BIRTHDAY ? 'birthday' : 'christmas'))+'.gif" id="optimg'+(totlinks+5)+'" border="0" alt="Special Occasion"></a>';
		data += '<a href="javascript:show_layer(11,'+(totlinks+6)+','+x+','+idx+','+itemID+','+return_platformIDs[idx]+','+type+','+return_types[idx]+')"><img src="/images/invoice/serial-num.gif" id="optimg'+(totlinks+6)+'" border="0" alt="Set Serial #"></a>';

		totlinks += 5;
	}
	data += '</td>';

	set_width = 40;
	alt = 'Item Image';
	if (!image.length)
	{
		image = '/images/blank.gif';
		set_width = 10;
		alt = '- No Image -';
	}
	data += '<td><img id="itemimg' + type + idx + '" src="' + image + '" border="0" width="' + set_width + '" alt="' + alt + '" /></td>';

	data += '<td title="'+htmlspecialchars(platform_name)+'">'+platform_abbr+'</td>';
	data += '<td style="cursor:hand" onclick="iteminfo_window('+itemID+','+type+')" title="Click for item information or to change pricing and quantity information">';
	var show_title = title;
	if (title.length > max_title_length)
	{
		show_title = title.substring(0,max_title_length) + '...';
		data += '<span title="' + htmlspecialchars(title) + '">';
	}
	data += htmlspecialchars(show_title);
	if (title.length > max_title_length)
	{
		data += '</span>';
	}
	data += '</td>';
	data += '<td title="In-stock quantity (new/used)" id="origqty'+type+idx+'">'+qty_new_orig[array_search(itemID,qty_itemIDs)]+'/'+qty_used_orig[array_search(itemID,qty_itemIDs)]+'</td>';
	data += '<td align="center"><input type="text" name="qty[' + type + '|' + idx + ']" id="qty' + type + idx + '" size="3" onkeypress="return onlynumbers(this.value,event,false)" onfocus="this.select()" onblur="set_quantity(this,' + type + ',' + x + ',' + idx + ',' + itemID + ')" value="' + qty + '" style="text-align:right"></td>';
	//data += '<td align="center"><input type="text" name="qty[' + type + '|' + idx + ']" id="qty' + type + idx + '" size="3" onkeypress="return onlynumbers(this.value,event,false)" onfocus="this.select()" onblur="' + (type!=RETURNS ? 'set_quantity(this,' + type + ',' + x + ',' + idx + ',' + itemID + ')' : '') + '" value="' + qty + '" ' + (type==RETURNS ? ' readonly="readonly"' : '') + ' style="text-align:right"></td>';

	// type-specific data
	if (type == SALE) // sales
	{
		data += '<td style="cursor:hand" title="Set discount percentage" onclick="javascript:set_percent('+type+','+x+','+idx+','+itemID+',true,123456)" id="perc'+idx+'"><div id="percent'+type+'0'+idx+'" style="text-align:center;white-space:nowrap;width:40px">'+sale_showpercentoff[idx]+'</div></td>';
		data += '<td><div id="prc'+type+'0'+idx+'" style="text-align:right">&nbsp;</div></td>';
	}
	else if (type == TRADE) // trades
	{
		data += '<td><div id="prcval'+type+CASH+idx+'" style="text-align:right;color:#AAAAAA">&nbsp;</div></td>';
		data += '<td><div id="prcval'+type+CREDIT+idx+'" style="text-align:right;color:#AAAAAA">&nbsp;</div></td>';
		data += '<td align="center" id="pd'+idx+'" onclick="show_pricing_details('+idx+')" title="Click for Pricing Details" style="cursor:hand"><a href="javascript:show_pricing_details('+idx+')" title="Click for Pricing Details">D</a></td>';
		data += '<td><div id="prc'+type+CASH+idx+'" style="text-align:right">&nbsp;</div></td>';
		data += '<td onclick="manual_price('+x+','+idx+')" style="cursor:hand" title="Click to manually set the credit amount"><div id="prc'+type+CREDIT+idx+'" style="text-align:right">&nbsp;</div></td>';
	}
	else if (type == RETURNS) // returns
	{
		// !!! SEE BELOW FOR NOTE !!!
		data += '<td>'+date_format('m/d/y',return_purchdates[idx])+'</td>';
		data += '<td align="right">$'+return_purchprices[idx]+'</td>';
		data += '<td align="right"><div id="prc'+type+'0'+idx+'" style="text-align:right">&nbsp;</div></td>';
	}

	data += '</tr>';
	data += '<!--/idx'+idx+'-->';

	return data;
}

/**
* Return an array of idxs for the given type
*/
function getIDXs(type)
{
	var idxs = eval('type' + type + '_idxs');
	var newidxs = new Array();

	for (var i=0; i<idxs.length; i++)
	{
		if (typeof idxs[i] != 'undefined') { newidxs[newidxs.length] = idxs[i]; }
	}

	return newidxs;
}

/**
* Return the number of idxs for the given type
*/
function getIDXLength(type)
{
	var idxs = eval('type' + type + '_idxs');
	return idxs.length;
}

/**
* Draw a single row
* 'function' = change/add/remove/addbefore/addafter
* if 'function' is addbefore or addafter, baidx must be passed (before/after idx)
*/
function drawData(type,idx,func,baidx,dontdrawall)
{
	var dataobj = $('data'+type);
	var data = getData(type);

	var idxs = getIDXs(type);

	if (idxs.length == 1 && func.substr(0,3) == 'add')
	{
		// just draw the table
		drawAllData(type);
	}
	else
	{
		var table_tag = (data.indexOf('</table>')!=-1 ? '</table>' : '</TABLE>');

		if (func == 'change')
		{
			var idxs = getIDXs(type);
			if (in_array(idx,idxs))
			{
				var thisdata = getIDXString(type,idx);
				var newdata = buildData(type,idx,array_search(idx,idxs));
				data = data.split(thisdata).join(newdata);
			}
		}
		else if (func == 'add')
		{
			var thisdata = buildData(type,idx,array_search(idx,idxs));
			data = data.split(table_tag).join(thisdata+table_tag);
		}
		else if (func == 'remove')
		{
			var thisdata = getIDXString(type,idx,array_search(idx,idxs));
			data = data.split(thisdata).join('');
		}
		else if (func == 'addbefore' || func == 'addafter')
		{
			var thisdata = buildData(type,idx,array_search(idx,idxs));
			var badata = getIDXString(type,baidx);
			data = data.split(badata).join((func=='addbefore'?thisdata+badata:badata+thisdata));
		}

		dataobj.innerHTML = data;
	}

	rebuildIDXs(type); // make sure that getFirstIDX() pulls the right IDX
	if (!dontdrawall) { drawAllData(type); recolorTable(type); }
}

/**
* Draw all 3 tables
*/
function drawAllTables()
{
	drawAllData(SALE);
	drawAllData(TRADE);
	drawAllData(RETURNS);
}

/**
* [Re]draws an entire table
*/
var totlinks = 0; // used in generating the options on each item
function drawAllData(type)
{
	setTotlinks(type);

	var obj = $('data'+type);
	var idxs = getIDXs(type);

	var alldata = replaceSubstring(table_head,'%type%',type);
	alldata += buildHeader(type);

	for (var i=0; i<idxs.length; i++) { alldata += buildData(type,idxs[i],i); }

	alldata += '<!--idx9999--><!--/idx9999-->';

	if (!idxs.length) { alldata += '<tr><td colspan="20" bgcolor="' + color_cell + '"><font color="#BBBBBB">No items in this category.</font></td></tr>'; }
	else { alldata += buildFooter(type); }

	alldata += table_foot;
	obj.innerHTML = alldata;

	recolorTable(type);
}

/**
* Check all tables; if there are no idxs in any, redraw the table
* This is called after items are removed
*/
function checkTables(type)
{
	for (var i=0; i<all_types.length; i++)
	{
		var type = all_types[i];
		var idxs = getIDXs(type);
		if (!idxs.length) { drawAllData(type); }
	}
}

/**
* Return the data for the given type
*/
function getData(type)
{
	return $('data'+type).innerHTML;
}

/**
* Return the data string for the given idx in the given table
*/
function getIDXString(type,idx)
{
	var data = getData(type);
	var pos = getPosition(type,idx);

	return data.substring(pos[0],pos[1]);
}

/**
* Get the beginning/ending positions for the given idx in the given table
*/
function getPosition(type,idx)
{
	var pos = [0,0];
	var data = getData(type);
	var find_start = '<!--idx'+idx+'-->';
	var find_end = '<!--/idx'+idx+'-->';

	pos[0] = data.indexOf(find_start);
	pos[1] = (data.indexOf(find_end)+find_end.length);

	return pos;
}

/**
* Rebuilds the type<type>_idxs array according to the contents of the table
*/
function rebuildIDXs(type)
{
	var data = getData(type);
	var find = '<!--idx';
	var pos = -1;
	var idxs = new Array();

	while (true)
	{
		pos = data.indexOf(find,(pos+1));
		if (pos == -1) { break; }
		else
		{
			var idx = parseInt(data.substr((pos+find.length),4));
			if (idx != 9999) { idxs[idxs.length] = idx; }
		}
	}

	eval('type' + type + '_idxs=idxs');
	//alert('total found: '+idxs.length+' ('+idxs+')');
}

/**
* Return the first IDX of the given type
*/
function getFirstIDX(type)
{
	var idxs = getIDXs(type);
	if (idxs.length) { return idxs[0]; }
	else { return 0; }
}

/**
* Return the last IDX of the given type
*/
function getLastIDX(type)
{
	var idxs = getIDXs(type);
	if (idxs.length) { return idxs[(idxs.length-1)]; }
	else { return 0; }
}

/**
* Return the next IDX of the given type
*/
function getNextIDX(type)
{
	var idxs = getIDXs(type);
	if (idxs.length) { return (idxs[(idxs.length-1)]+1); }
	else { return 0; }
}

/**
* Set the 'totlinks' value for the given type
*/
function setTotlinks(type)
{
	var sale_idxs = getIDXs(SALE);
	var trade_idxs = getIDXs(TRADE);

	if (type == SALE) { totlinks = 0; }
	else if (type == TRADE) { totlinks = (sale_idxs.length*2); }
	else if (type == RETURNS) { totlinks = (sale_idxs.length*2)+(trade_idxs.length*3); }
}

/**
* Recolor the row backgrounds for the given type
*/
function recolorTable(type)
{
	var header_color = color_label;
	var footer_color = color_label;
	var highlight_color = '#FFDDDD';
	var row_colors = [color_cell,color_cell2];

	var obj = $('datatable' + type).rows;
	for (var i=0; i<obj.length; i++)
	{
		if (!i) { bg = header_color; }
		else if (i == (obj.length-1)) { bg = footer_color; }
		else { bg = (i==1 && obj.length>3 ? highlight_color : row_colors[(i % 2)]); }

		obj[i].bgColor = bg;
	}
}

/* END OF FILE */