<?php
include('../../include/include.inc');

$cl = new check_login(STORE);

$pg = new admin_page();
$pg->setTitle('Edit Items by Criteria');
$pg->head('Edit Items by Criteria');

// get the possible values
/*
Year
Platform
Company 1
Company 2
Type
Price New
Price Used
*/

// years
$yr = new years($pg);
$years = makeKeyValueArray(
	$yr->getYears(),
	array('yer_yearID','yer_year')
);

// platforms
$pla = new platforms($pg,0);
$pla->set_item('platforms');
$platforms = makeKeyValueArray(
	$pla->getGoodValues(),
	array('pla_platformID','pla_name')
);

// companies
$companies = makeKeyValueArray($pla->getDistinctCompanies());

// types
$types = makeKeyValueArray($pla->getDistinctTypes());

// operations
$ops = items::getCriteriaOperations();

// options setup
// format: $options['field'] = array('description','string/number',field_size[,values])
// if values are provided, a selectbox will be shown (field_size will become 1)
$options = array(
	'year'       => array('Year',      'number',1,$years),
	'platform'   => array('Platform',  'string',1,$platforms),
	'company1'   => array('Company 1', 'string',1,$companies),
	'company2'   => array('Company 2', 'string',1,$companies),
	'type'       => array('Type',      'string',1,$types),
	'price_new'  => array('Price New', 'number',6),
	'price_used' => array('Price Used','number',6)
);

// show the form
?>
<script type="text/javascript">
</script>

Select up to 10 different criteria and enter what you would like changed:
<p />

<form method="post" action="/admin/setup_items/criteriaUpdate.php">
	<input type="hidden" name="act" value="get_items" />

	<?php
	echo $pg->outlineTableHead();
	for ($i=0; $i<10; $i++)
	{
		?>
		<tr>
			<td bgcolor="<?php echo TABLE_CELL;?>">
				<select name="field[<?php echo $i;?>]" size="1" onchange="setField(<?php echo $i;?>,this.value)">
					<option value="">[select field]</option>
					<?php
					while (list($field,$arr) = each($options))
					{
						$title  = $arr[0];
						$type   = $arr[1];
						$size   = $arr[2];
						$values = @$arr[3];

						?>
						<option value="<?php echo $field;?>"><?php echo $title;?></option>
						<?php
					} // each options
					reset($options);
					?>
				</select>
			</td>
		</tr>
		<?php

		if ($i < 9)
		{
			?>
			<tr>
				<td bgcolor="<?php echo TABLE_CELL;?>" colspan="3">
					- AND -
				</td>
			</tr>
			<?php
		}
	} // for i=0 through 9
	echo $pg->outlineTableFoot();
	?>
</form>
<?php

$pg->foot();
?>