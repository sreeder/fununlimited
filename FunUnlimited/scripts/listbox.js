// -------------------------------------------------------------------
// selectAllOptions(select_object)
//  This function takes a select box and selects all options (in a
//  multiple select object). This is used when passing values between
//  two select boxes. Select all options in the right box before
//  submitting the form so the values will be sent to the server.
// -------------------------------------------------------------------
function selectAllOptions(obj)
{
	for (var i=0; i<obj.options.length; i++)
	{
		obj.options[i].selected = true;
	}
}

// -------------------------------------------------------------------
// swapOptions(select_object,option1,option2)
//  Swap positions of two options in a select list
// -------------------------------------------------------------------
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

// -------------------------------------------------------------------
// moveOptionUp(select_object)
//  Move selected option in a select list up one
// -------------------------------------------------------------------
function moveOptionUp(obj)
{
	// If > 1 option selected, do nothing
	var selectedCount=0;
	for (i=0; i<obj.options.length; i++)
	{
		if (obj.options[i].selected) { selectedCount++; }
	}
	if (!selectedCount || selectedCount > 1) { return; }

	// If this is the first item in the list, do nothing
	var i = obj.selectedIndex;
	if (i == 0) { return; }

	swapOptions(obj,i,i-1);
	obj.options[i-1].selected = true;
}

// -------------------------------------------------------------------
// moveOptionDown(select_object)
//  Move selected option in a select list down one
// -------------------------------------------------------------------
function moveOptionDown(obj)
{
	// If > 1 option selected, do nothing
	var selectedCount=0;
	for (i=0; i<obj.options.length; i++)
	{
		if (obj.options[i].selected) { selectedCount++; }
	}
	if (!selectedCount || selectedCount > 1) { return; }

	// If this is the last item in the list, do nothing
	var i = obj.selectedIndex;
	if (i == (obj.options.length-1)) { return; }

	swapOptions(obj,i,i+1);
	obj.options[i+1].selected = true;
}

function move(from,to,nosort)
{
  // move to new
  for (var i=0; i<from.options.length; i++)
  {
    var opt = from.options[i];
    if (opt.selected) { to.options[to.options.length] = new Option(opt.text, opt.value, false, false); }
  }

  // delete from original
  for (var i=(from.options.length-1); i>=0; i--)
  {
    var opt = from.options[i];
    if (opt.selected) { from.options[i] = null; }
  }

  if (!nosort) { sortSelect(from); }
  if (!nosort) { sortSelect(to); }

  from.selectedIndex = -1;
  to.selectedIndex = -1;
}

function moveAll(from,to,nosort)
{
  selectAllOptions(from);
  move(from,to,nosort);
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
