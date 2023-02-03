--TEST--
Text properties
--EXTENSIONS--
text
--FILE--
<?php
$t = new Text('Prynhawn da!');
var_dump($t);
?>
--EXPECTF--
object(Text)#%d (%d) {
  ["text"]=>
  string(%d) "Prynhawn da!"
  ["collation"]=>
  string(%d) "Unknown language"
}
