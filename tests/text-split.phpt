--TEST--
Text::split
--EXTENSIONS--
text
--FILE--
<?php
function show_bytes(string $str)
{
	return bin2hex($str);
}

$stringSets = [
	[ new \Text('Øl i kveld!'), new \Text('i') ],
	[ new \Text('Øl i kveld!'), 'i' ],
	[ new \Text('Å E̊ O̊!', 'nb'), "\u{030a}" ],
	[ new \Text("’S e Bòrd na Gàidhlig a’ phrìomh bhuidheann phoblach ann an Alba. Comhairle do Mhinistearan na h-Alba air cùisean Gàidhlig.", 'gd'), "Gàidhlig" ],
];
	
foreach ($stringSets as $set) {
	echo "\n===\n\n";

	$t = $set[0]->split( $set[1] );

	foreach ($t as $i => $value) {
		echo $i, ': ', (string) $value, ' (', show_bytes($value), "):\n";
		var_dump($value);
	}
}
?>
--EXPECTF--
===

0: Øl  (c3986c20):
object(Text)#6 (2) {
  ["text"]=>
  string(4) "Øl "
  ["collation"]=>
  string(16) "Unknown language"
}
1:  kveld! (206b76656c6421):
object(Text)#7 (2) {
  ["text"]=>
  string(7) " kveld!"
  ["collation"]=>
  string(16) "Unknown language"
}

===

0: Øl  (c3986c20):
object(Text)#8 (2) {
  ["text"]=>
  string(4) "Øl "
  ["collation"]=>
  string(16) "Unknown language"
}
1:  kveld! (206b76656c6421):
object(Text)#9 (2) {
  ["text"]=>
  string(7) " kveld!"
  ["collation"]=>
  string(16) "Unknown language"
}

===

0: Å E (c3852045):
object(Text)#7 (2) {
  ["text"]=>
  string(4) "Å E"
  ["collation"]=>
  string(17) "Norwegian Bokmål"
}
1:  O (204f):
object(Text)#6 (2) {
  ["text"]=>
  string(2) " O"
  ["collation"]=>
  string(17) "Norwegian Bokmål"
}
2: ! (21):
object(Text)#10 (2) {
  ["text"]=>
  string(1) "!"
  ["collation"]=>
  string(17) "Norwegian Bokmål"
}

===

0: ’S e Bòrd na  (e280995320652042c3b27264206e6120):
object(Text)#9 (2) {
  ["text"]=>
  string(16) "’S e Bòrd na "
  ["collation"]=>
  string(15) "Scottish Gaelic"
}
1:  a’ phrìomh bhuidheann phoblach ann an Alba. Comhairle do Mhinistearan na h-Alba air cùisean  (2061e2809920706872c3ac6f6d682062687569646865616e6e2070686f626c61636820616e6e20616e20416c62612e20436f6d686169726c6520646f204d68696e697374656172616e206e6120682d416c6261206169722063c3b9697365616e20):
object(Text)#8 (2) {
  ["text"]=>
  string(97) " a’ phrìomh bhuidheann phoblach ann an Alba. Comhairle do Mhinistearan na h-Alba air cùisean "
  ["collation"]=>
  string(15) "Scottish Gaelic"
}
2: . (2e):
object(Text)#11 (2) {
  ["text"]=>
  string(1) "."
  ["collation"]=>
  string(15) "Scottish Gaelic"
}
