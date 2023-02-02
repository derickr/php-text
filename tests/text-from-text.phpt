--TEST--
Text from Text 
--EXTENSIONS--
text
--FILE--
<?php
$t1 = new \Text("We gaan van deze weer een andere Text maken");
$t2 = new \Text($t1);

echo "Created Text object: {$t2}\n";

echo 'Displaying string-cast Text object: ';
echo (string) $t2, "\n\n";
?>
--EXPECT--
Created Text object: We gaan van deze weer een andere Text maken
Displaying string-cast Text object: We gaan van deze weer een andere Text maken
