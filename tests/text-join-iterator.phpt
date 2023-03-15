--TEST--
Text::join through iterator
--EXTENSIONS--
text
--FILE--
<?php
function show_bytes(string $str)
{
	return bin2hex($str);
}

$stringSets = [
	' ' => [ 'Øl', 'i kveld!' ],
	"\u{030a}" => [ 'A', ' E', ' O', '!' ],
	'i' => [ new \Text('Øl ', 'nb'), ' kveld!' ],
	'I' => [ 'Øl ', new \Text(' kveld!', 'nb') ],
	"\u{030a}\u{030b}\u{030c}\u{030d}\u{030e}\u{030f}—" => [ 'A', 'U', 'Y' ],
];
	
foreach ($stringSets as $separator => $set) {
	echo "\n===\n\n";

	$t = Text::join(new \ArrayIterator($set), $separator, 'nl');

	echo (string) $t, ' (', show_bytes($t), "):\n";
	
	var_dump($t);
}
?>
--EXPECTF--
===

Øl i kveld! (c3986c2069206b76656c6421):
object(Text)#%d (2) {
  ["text"]=>
  string(12) "Øl i kveld!"
  ["collation"]=>
  string(5) "Dutch"
}

===

Å E̊ O̊! (c3852045cc8a204fcc8a21):
object(Text)#%d (2) {
  ["text"]=>
  string(11) "Å E̊ O̊!"
  ["collation"]=>
  string(5) "Dutch"
}

===

Øl i kveld! (c3986c2069206b76656c6421):
object(Text)#%d (2) {
  ["text"]=>
  string(12) "Øl i kveld!"
  ["collation"]=>
  string(5) "Dutch"
}

===

Øl I kveld! (c3986c2049206b76656c6421):
object(Text)#%d (2) {
  ["text"]=>
  string(12) "Øl I kveld!"
  ["collation"]=>
  string(5) "Dutch"
}

===

Å̋̌̍̎̏—Ů̋̌̍̎̏—Y (c385cc8bcc8ccc8dcc8ecc8fe28094c5aecc8bcc8ccc8dcc8ecc8fe2809459):
object(Text)#%d (2) {
  ["text"]=>
  string(31) "Å̋̌̍̎̏—Ů̋̌̍̎̏—Y"
  ["collation"]=>
  string(5) "Dutch"
}
