--TEST--
Text::toUpper
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
	new \Text('Turkye: i', 'tr'),
];
	
foreach ($stringSets as $set) {
	echo "\n===\n\n";

	$t = $set->toUpper();

	echo (string) $t, ' (', show_bytes($t), "):\n";
	var_dump($t);
}
?>
--EXPECTF--
===

ØL I KVELD! (c3984c2049204b56454c4421):
object(Text)#%d (2) {
  ["text"]=>
  string(12) "ØL I KVELD!"
  ["collation"]=>
  string(16) "Unknown language"
}

===

Å E̊ O̊! (c3852045cc8a204fcc8a21):
object(Text)#%d (2) {
  ["text"]=>
  string(11) "Å E̊ O̊!"
  ["collation"]=>
  string(17) "Norwegian Bokmål"
}

===

ĲSSELMEER IS VOL MET IDEËEN (c4b25353454c4d45455220495320564f4c204d455420494445c38b454e):
object(Text)#%d (2) {
  ["text"]=>
  string(29) "ĲSSELMEER IS VOL MET IDEËEN"
  ["collation"]=>
  string(5) "Dutch"
}

===

TURKYE: I (5455524b59453a2049):
object(Text)#%d (2) {
  ["text"]=>
  string(9) "TURKYE: I"
  ["collation"]=>
  string(6) "German"
}

===

TURKYE: İ (5455524b59453a20c4b0):
object(Text)#%d (2) {
  ["text"]=>
  string(10) "TURKYE: İ"
  ["collation"]=>
  string(7) "Turkish"
}
