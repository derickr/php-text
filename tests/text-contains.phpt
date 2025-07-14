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
		'string' => 'Belgiere er kjent for ølet sitt!',
		'locale' => 'nb_NO', 
		'checks' => [
			'øl', 'Bel', 'kjent',
			'sitt!', '!', 'ØL',
			new \Text('ølet'),
			new \Text('Ø'),
		],
	],
	[
		'string' => 'Belgiere er kjent for ølet sitt!',
		'locale' => 'nb_NO-u-ks-level2', 
		'checks' => [
			'øl', 'Bel', 'kjent',
			'sitt!', '!', 'ØL',
			new \Text('ølet'),
			new \Text('Ø'),
		],
	],
	[
		'string' => 'Belgiere er kjent for ølet sitt!',
		'locale' => 'nb_NO-u-ks-level4', 
		'checks' => [
			'øl', 'Bel', 'kjent',
			'sitt!', '!', 'ØL',
			new \Text('ølet'),
			new \Text('Ø'),
		],
	],
];
	
foreach ($stringSets as $set) {
	$t = new \Text($set['string'], $set['locale']);

	foreach ($set['checks'] as $check) {
		echo "'{$set['string']}' ({$set['locale']}) contains '{$check}': ",
			 $t->contains($check) ? 'yes' : 'no',
			 "\n";
	}

	echo "\n===\n\n";
}
?>
--EXPECT--
'Belgiere er kjent for ølet sitt!' (nb_NO) contains 'øl': yes
'Belgiere er kjent for ølet sitt!' (nb_NO) contains 'Bel': yes
'Belgiere er kjent for ølet sitt!' (nb_NO) contains 'kjent': yes
'Belgiere er kjent for ølet sitt!' (nb_NO) contains 'sitt!': yes
'Belgiere er kjent for ølet sitt!' (nb_NO) contains '!': yes
'Belgiere er kjent for ølet sitt!' (nb_NO) contains 'ØL': no
'Belgiere er kjent for ølet sitt!' (nb_NO) contains 'ølet': yes
'Belgiere er kjent for ølet sitt!' (nb_NO) contains 'Ø': no

===

'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level2) contains 'øl': yes
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level2) contains 'Bel': yes
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level2) contains 'kjent': yes
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level2) contains 'sitt!': yes
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level2) contains '!': yes
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level2) contains 'ØL': yes
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level2) contains 'ølet': yes
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level2) contains 'Ø': yes

===

'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level4) contains 'øl': yes
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level4) contains 'Bel': yes
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level4) contains 'kjent': yes
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level4) contains 'sitt!': yes
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level4) contains '!': yes
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level4) contains 'ØL': no
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level4) contains 'ølet': yes
'Belgiere er kjent for ølet sitt!' (nb_NO-u-ks-level4) contains 'Ø': no

===
