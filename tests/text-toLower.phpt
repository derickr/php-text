--TEST--
Text::toLower
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
	new \Text('TURKYE: I', 'de'),
	new \Text('TURKYE: I', 'tr'),
];
	
foreach ($stringSets as $set) {
	echo "\n===\n\n";

	$t = $set->toLower();

	echo (string) $t, ' (', show_bytes($t), "):\n";
	var_dump($t);
}
?>
--EXPECTF--
===

øl i kveld! (c3b86c2069206b76656c6421):
object(Text)#%d (2) {
  ["text"]=>
  string(12) "øl i kveld!"
  ["collation"]=>
  string(16) "Unknown language"
}

===

å e̊ o̊! (c3a52065cc8a206fcc8a21):
object(Text)#%d (2) {
  ["text"]=>
  string(11) "å e̊ o̊!"
  ["collation"]=>
  string(17) "Norwegian Bokmål"
}

===

ĳsselmeer is vol met ideëen (c4b37373656c6d65657220697320766f6c206d657420696465c3ab656e):
object(Text)#%d (2) {
  ["text"]=>
  string(29) "ĳsselmeer is vol met ideëen"
  ["collation"]=>
  string(5) "Dutch"
}

===

turkye: i (7475726b79653a2069):
object(Text)#%d (2) {
  ["text"]=>
  string(9) "turkye: i"
  ["collation"]=>
  string(6) "German"
}

===

turkye: ı (7475726b79653a20c4b1):
object(Text)#%d (2) {
  ["text"]=>
  string(10) "turkye: ı"
  ["collation"]=>
  string(7) "Turkish"
}
