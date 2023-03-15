--TEST--
Text::join with empty array
--EXTENSIONS--
text
--FILE--
<?php
function show_bytes(string $str)
{
	return bin2hex($str);
}

$stringSets = [
	' ' => [],
];
	
foreach ($stringSets as $separator => $set) {
	echo "\n===\n\n";

	$t = Text::join($set, $separator, 'nl');
	echo (string) $t, ' (', show_bytes($t), "):\n";
	var_dump($t);

	$t = Text::join(new \ArrayIterator($set), $separator, 'nl');
	echo (string) $t, ' (', show_bytes($t), "):\n";
	var_dump($t);
}
?>
--EXPECTF--
===

 ():
object(Text)#%d (2) {
  ["text"]=>
  string(0) ""
  ["collation"]=>
  string(5) "Dutch"
}
 ():
object(Text)#%d (2) {
  ["text"]=>
  string(0) ""
  ["collation"]=>
  string(5) "Dutch"
}
