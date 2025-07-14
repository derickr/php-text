--TEST--
Text::getCharacterCount
--EXTENSIONS--
text
--FILE--
<?php
function show_bytes(string $str)
{
	return bin2hex($str);
}

$stringSets = [
	new \Text('Øl i kveld!'),
	new \Text('Å E̊ O̊!', 'nb'),
	new \Text('Ĳsselmeer is vol met ideëen', 'nl'),
	new \Text('Turkye: i', 'de'),
	new \Text(''),
	new \Text('1'),
];
	
foreach ($stringSets as $t) {
	printf( "'%s'\n%3d: %s\n\n", $t, $t->getCharacterCount(), show_bytes($t) );
}
?>
--EXPECTF--
'Øl i kveld!'
 11: c3986c2069206b76656c6421

'Å E̊ O̊!'
  6: c3852045cc8a204fcc8a21

'Ĳsselmeer is vol met ideëen'
 27: c4b27373656c6d65657220697320766f6c206d657420696465c3ab656e

'Turkye: i'
  9: 5475726b79653a2069

''
  0: 

'1'
  1: 31
