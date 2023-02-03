--TEST--
Text::concat
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
];
	
$t = Text::concat();
var_dump($t);

foreach ($stringSets2 as $set2) {
	echo "\n===\n\n";

	$t = Text::concat(
		new \Text($set2[0], 'nb_NO'),
		new \Text($set2[1], 'nb_NO')
	);

	echo (string) $t, ' (', show_bytes($t), "):\n";
	
	var_dump($t);
}
?>
--EXPECTF--
object(Text)#%d (%d) {
  ["text"]=>
  string(%d) ""
  ["collation"]=>
  string(%d) "Unknown language"
}

===

Øl i kveld! (c3986c2069206b76656c6421):
object(Text)#%d (%d) {
  ["text"]=>
  string(%d) "Øl i kveld!"
  ["collation"]=>
  string(%d) "Norwegian Bokmål (Norway)"
}

===

Å (c385):
object(Text)#%d (%d) {
  ["text"]=>
  string(%d) "Å"
  ["collation"]=>
  string(%d) "Norwegian Bokmål (Norway)"
}
