<!--
// File: selection.js
// Description: javascript functions for the selection class (selection.inc)

// some of these functions were borrowed/modified from Matt Kruse @ http://www.mattkruse.com/javascript/selectbox/

function move(from,to)
{
	// move to new
	for (var i=0; i<from.options.length; i++)
	{
		var opt = from.options[i];
		if (opt.selected)
		{
			to.options[to.options.length] = new Option(opt.text, opt.value, false, false);
		}
	}

	// delete from original
	for (var i=(from.options.length-1); i>=0; i--)
	{
		var opt = from.options[i];
		if (opt.selected)
		{
			from.options[i] = null;
		}
	}

	sortSelect(from);
	sortSelect(to);

	from.selectedIndex = -1;
	to.selectedIndex = -1;
}

function sortSelect(obj)
{
	var opt = new Array();
	for (var i=0; i<obj.options.length; i++)
	{
		opt[opt.length] = new Option(obj.options[i].text, obj.options[i].value, obj.options[i].defaultSelected, obj.options[i].selected) ;
	}
	opt = opt.sort(
		function(a,b)
		{
			var txta = a.text+"";
			var txtb = b.text+"";
			txta = txta.toLowerCase();
			txtb = txtb.toLowerCase();

			if (txta < txtb) { return -1; }
			if (txta > txtb) { return 1; }
			return 0;
		}
	);

	for (var i=0; i<opt.length; i++)
	{
		obj.options[i] = new Option(opt[i].text, opt[i].value, opt[i].defaultSelected, opt[i].selected);
	}
}

function moveAll(from,to)
{
	selectAllOptions(from);
	move(from,to);
}

function selectAllOptions(obj)
{
	for (var i=0; i<obj.options.length; i++)
	{
		obj.options[i].selected = true;
	}
}

function swapOptions(obj,i,j)
{
	var o = obj.options;
	var i_selected = o[i].selected;
	var j_selected = o[j].selected;
	var temp = new Option(o[i].text, o[i].value, o[i].defaultSelected, o[i].selected);
	var temp2= new Option(o[j].text, o[j].value, o[j].defaultSelected, o[j].selected);
	o[i] = temp2;
	o[j] = temp;
	o[i].selected = j_selected;
	o[j].selected = i_selected;
}

function moveUp(obj)
{
	// If > 1 option selected, do nothing
	var selectedCount=0;
	for (i=0; i<obj.options.length; i++)
	{
		if (obj.options[i].selected) { selectedCount++; }
	}
	if (selectedCount > 1 || selectedCount == 0) { return; }

	// If this is the first item in the list, do nothing
	var i = obj.selectedIndex;
	if (i == 0) { return; }

	swapOptions(obj,i,i-1);
	obj.options[i-1].selected = true;
}

function moveDown(obj)
{
	// If > 1 option selected, do nothing
	var selectedCount = 0;
	for (i=0; i<obj.options.length; i++)
	{
		if (obj.options[i].selected) { selectedCount++; }
	}
	if (selectedCount > 1 || selectedCount == 0) { return; }

	// If this is the last item in the list, do nothing
	var i = obj.selectedIndex;
	if (i == (obj.options.length-1)) { return; }

	swapOptions(obj,i,i+1);
	obj.options[i+1].selected = true;
}

function setDefault(frm,obj,def_box,def_box_hidden)
{
	if (obj.selectedIndex > -1)
	{
		def_box.value = obj.options[obj.selectedIndex].text;
		def_box_hidden.value = obj.options[obj.selectedIndex].value;
	}
}

// -->