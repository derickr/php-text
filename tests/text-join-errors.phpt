--TEST--
Text::join with wrong data
--EXTENSIONS--
text
--FILE--
<?php
$stringSets = [
	' ' => [ 42, 'i kveld!' ],
	"\u{030a}" => [ 'A', M_PI ],
];
	
foreach ($stringSets as $separator => $set)
{
	try {
		$t = Text::join($set, $separator);
	} catch (\TypeError $e) {
		echo get_class($e), ': ', $e->getMessage(), "\n";
	}
}
?>
--EXPECTF--
TypeError: Text::join(): Argument #1 ($elements) must be of class Text or a string, int given
TypeError: Text::join(): Argument #1 ($elements) must be of class Text or a string, float given
