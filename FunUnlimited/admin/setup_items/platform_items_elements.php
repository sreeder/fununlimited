<?php
// Whole Platform Item Info element array

$elements_merge = array();

if (isset($platformID))
{
	// pull values
	$pla = new platforms($pg,$platformID);

	$years = array();
	$yr = new years(); // pull in the years
	while (list($a,$arr) = each($yr->years)) { $years[$arr['yer_yearID']] = $arr['yer_year']; }

	$companies = array();
	$pla->set_item('companies');
	while (list($a,$arr) = each($pla->values)) { $companies[$arr[0]] = $arr[1]; }

	$types = array();
	$pla->set_item('types');
	while (list($a,$arr) = each($pla->values)) { $types[$arr[0]] = $arr[1]; }

	$price_sourceIDs = array();
	$rating_sourceIDs = array();
	$pla->set_item('sources');
	while (list($a,$arr) = each($pla->values))
	{
		if ($arr[2] == PRICE)
		{
			$price_sourceIDs[] = $arr[0];
			$elements_merge["sp_{$arr[0]}"] = array("Price-{$arr[1]}",'text',6,"sp_{$arr[0]}",NO);
		}
		elseif ($arr[2] == RATING)
		{
			$rating_sourceIDs[] = $arr[0];
			$elements_merge["sr_{$arr[0]}"] = array("Rating-{$arr[1]}",'text',4,"sr_{$arr[0]}",NO);
		}
	}
}
else
{
	// set values variables to array()
	$years = array();
	$companies = array();
	$types = array();
}

// format: 'element'=>array('name','field_type','size','database_field',[array_of_values_if_select -OR- YES/NO_include_in_list])
$elements = array(
	'title'=>array('Title','text',50,'itm_title',NO),
	'description'=>array('Description','text',40,'itm_description'),
	'upc'=>array('UPC','text',20,'itm_upc'),
	'age'=>array('Age','text',4,'itm_age'),
	'year'=>array('Year','select',1,'itm_yearID',$years),
	'companies'=>array('Companies','expand',1,'companies'),
	'company1'=>array('Company 1','select',1,'itm_company1ID',$companies,NO),
	'company2'=>array('Company 2','select',1,'itm_company2ID',$companies,NO),
	'type'=>array('Type','select',1,'itm_typeID',$types),
	'quantities'=>array('Quantities','expand',1,'quantities'),
	'qtynew'=>array('QtyNew','text',3,'qty_new',NO),
	'qtyused'=>array('QtyNew','text',3,'qty_used',NO),
	'pricing'=>array('Pricing','expand',1,'pricing'),
	'pricenew'=>array('PriceNew','text',6,'prc_new',NO),
	'priceused'=>array('PriceUsed','text',6,'prc_used',NO),
	'sourcepricing'=>array('Source Pricing','expand',1,'sourcepricing'),
	'sourceratings'=>array('Source Ratings','expand',1,'sourceratings')
);
$elements = array_merge($elements,$elements_merge);
$element_count = 0;
while (list($a,$arr) = each($elements))
{
	if (!isset($arr[4]) || (is_array($arr[4]) && (!isset($arr[5]) || (isset($arr[5]) && $arr[5])))) { $element_count++; }
}
reset($elements);

// format: array('name',array('elem1','elem2'[,etc]))
$presets = array(
	array('Basic Info',array('description','upc')),
	array('Extended Info',array('age','year','companies','type')),
	array('Quantity',array('quantities')),
	array('Pricing',array('pricing','sourcepricing')),
	array('Sources',array('sourcepricing','sourceratings')),
	array('All',array_keys($elements))
);
?>