<!--
// Functionality for Fun Unlimited Store Management header
// All code copyright � 2003-2004 Scott Carpenter [s-carp@comcast.net]

// handle the quick item lookup
function item_search(frm,popup,frombare)
{
	if (frm.elements['val'].value.length)
	{
		var opts = '?';
		var name = '';
		var value = '';

		for (var i=0; i<frm.elements.length; i++)
		{
			if (frm.elements[i].type != 'submit')
			{
				if (frm.elements[i].type == 'select-one')
				{
					name = frm.elements[i].name;
					value = frm.elements[i].options[frm.elements[i].selectedIndex].value;
					if (name != 'storeID') { frm.elements[i].selectedIndex = 0; }
				}
				else if (frm.elements[i].type == 'radio' && frm.elements[i].checked)
				{
					name = frm.elements[i].name;
					value = frm.elements[i].value;
				}
				else if (frm.elements[i].type == 'text')
				{
					name = frm.elements[i].name;
					value = frm.elements[i].value;
					frm.elements[i].value = '';
				}

				if (name.length) { opts += name+'='+value+'&'; }

				name = '';
				value = '';
			}
		}

		opts = opts.substring(0,opts.length-1);
		if (popup) { open_window('/admin/pos/iteminfo.php'+opts,'iteminfo',725,500,'YES',true); }
		else if (frombare) { document.location = '/admin/bare/items.php'+opts; }
		else { document.location = '/admin/pos/iteminfo.php'+opts; }
	}
	else { alert('Please enter a UPC/title'); }

	return false;
}

// -->