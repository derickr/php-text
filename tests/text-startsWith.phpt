--TEST--
Text::startsWith
--EXTENSIONS--
text
--FILE--
<?php
function show_bytes(string $str)
{
	return bin2hex($str);
}

$stringSets = [
	[
		'string' => 'This is a sentence.',
		'locale' => 'nb_NO', 
		'checks' => [
			'This', 'This is', 'T',
			't', 'sentence',
			new \Text('This is'),
			new \Text('t'),
		],
	],
	[
		'string' => 'This is a sentence.',
		'locale' => 'nb_NO-u-ks-primary', 
		'checks' => [
			'This', 'This is', 'T',
			't', 'sentence',
			new \Text('This is'),
			new \Text('t'),
		],
	],
	[
		'string' => 'This is a sentence.',
		'locale' => 'nb_NO-u-ks-level2', 
		'checks' => [
			'This', 'This is', 'T',
			't', 'sentence',
			new \Text('This is'),
			new \Text('t'),
		],
	],
];
	
foreach ($stringSets as $set) {
	$t = new \Text($set['string'], $set['locale']);

	foreach ($set['checks'] as $check) {
		echo "'{$set['string']}' ({$set['locale']}) starts with '{$check}': ",
			 $t->startsWith($check) ? 'yes' : 'no',
			 "\n";
	}

	echo "\n===\n\n";
}
?>
--EXPECT--
'This is a sentence.' (nb_NO) starts with 'This': yes
'This is a sentence.' (nb_NO) starts with 'This is': yes
'This is a sentence.' (nb_NO) starts with 'T': yes
'This is a sentence.' (nb_NO) starts with 't': no
'This is a sentence.' (nb_NO) starts with 'sentence': no
'This is a sentence.' (nb_NO) starts with 'This is': yes
'This is a sentence.' (nb_NO) starts with 't': no

===

'This is a sentence.' (nb_NO-u-ks-primary) starts with 'This': yes
'This is a sentence.' (nb_NO-u-ks-primary) starts with 'This is': yes
'This is a sentence.' (nb_NO-u-ks-primary) starts with 'T': yes
'This is a sentence.' (nb_NO-u-ks-primary) starts with 't': yes
'This is a sentence.' (nb_NO-u-ks-primary) starts with 'sentence': no
'This is a sentence.' (nb_NO-u-ks-primary) starts with 'This is': yes
'This is a sentence.' (nb_NO-u-ks-primary) starts with 't': yes

===

'This is a sentence.' (nb_NO-u-ks-level2) starts with 'This': yes
'This is a sentence.' (nb_NO-u-ks-level2) starts with 'This is': yes
'This is a sentence.' (nb_NO-u-ks-level2) starts with 'T': yes
'This is a sentence.' (nb_NO-u-ks-level2) starts with 't': yes
'This is a sentence.' (nb_NO-u-ks-level2) starts with 'sentence': no
'This is a sentence.' (nb_NO-u-ks-level2) starts with 'This is': yes
'This is a sentence.' (nb_NO-u-ks-level2) starts with 't': yes

===
