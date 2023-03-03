--TEST--
Text::concat with strings
--EXTENSIONS--
text
--FILE--
<?php
function show_bytes(string $str)
{
	return bin2hex($str);
}

$stringSets2 = [
	[ 'Øl ', 'i kveld!' ],
	[ 'A', "\u{030a}" ],
	[ new \Text('Øl ', 'nb'), 'i kveld!' ],
	[ 'A', new \Text("\u{030a}") ],
];
	
foreach ($stringSets2 as $set2) {
	echo "\n===\n\n";

	$t = Text::concat(
		$set2[0],
		$set2[1],
	);

	echo (string) $t, ' (', show_bytes($t), "):\n";
	
	var_dump($t);
}
?>
--EXPECTF--
===

Øl i kveld! (c3986c2069206b76656c6421):
object(Text)#3 (2) {
  ["text"]=>
  string(12) "Øl i kveld!"
  ["collation"]=>
  string(16) "Unknown language"
}

===

Å (c385):
object(Text)#4 (2) {
  ["text"]=>
  string(2) "Å"
  ["collation"]=>
  string(16) "Unknown language"
}

===

Øl i kveld! (c3986c2069206b76656c6421):
object(Text)#3 (2) {
  ["text"]=>
  string(12) "Øl i kveld!"
  ["collation"]=>
  string(17) "Norwegian Bokmål"
}

===

Å (c385):
object(Text)#4 (2) {
  ["text"]=>
  string(2) "Å"
  ["collation"]=>
  string(16) "Unknown language"
}
