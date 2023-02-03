--TEST--
Text debug display with locale
--EXTENSIONS--
text
--FILE--
<?php
$string = 'Hàlo agus fàilte';

$t1 = new \Text($string, 'gd');
var_dump($t1);

$t2 = new \Text($string, 'de-u-co-phonebk-kc-kv-space');
var_dump($t2);

$t3 = new \Text($t1);
var_dump($t3);

$t4 = new \Text($t2, 'en_GB');
var_dump($t4);
?>
--EXPECTF--
object(Text)#%d (%d) {
  ["text"]=>
  string(%d) "Hàlo agus fàilte"
  ["collation"]=>
  string(%d) "Scottish Gaelic"
}
object(Text)#%d (%d) {
  ["text"]=>
  string(%d) "Hàlo agus fàilte"
  ["collation"]=>
  string(%d) "German (colcaselevel=yes, Sort Order=Phonebook Sort Order, Highest Ignored=Ignore Symbols affects spaces only)"
}
object(Text)#%d (%d) {
  ["text"]=>
  string(%d) "Hàlo agus fàilte"
  ["collation"]=>
  string(%d) "Scottish Gaelic"
}
object(Text)#%d (%d) {
  ["text"]=>
  string(%d) "Hàlo agus fàilte"
  ["collation"]=>
  string(%d) "English (United Kingdom)"
}
