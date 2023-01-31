--TEST--
Text basic in/out
--EXTENSIONS--
text
--FILE--
<?php
$goodStrings = [
	'Å',
	'Hello World!',
	'Hàlo agus fàilte',
];

$badStrings = [
	"H\xA7llo",
	"H\xC3",
];

foreach (array_merge($goodStrings, $badStrings) as $string)
{
	$t = NULL;

	try {
		$t = new \Text($string);

		echo "Created Text object: {$string}\n";
		echo 'Resulting type: ', gettype($t), "\n";

		echo 'Displaying string-cast Text object: ';
		echo (string) $t, "\n\n";
	} catch (ValueError $e) {
		echo get_class($e), ': ', $e->getMessage(), "\n\n";
	}
}
?>
--EXPECT--
Created Text object: Å
Resulting type: object
Displaying string-cast Text object: Å

Created Text object: Hello World!
Resulting type: object
Displaying string-cast Text object: Hello World!

Created Text object: Hàlo agus fàilte
Resulting type: object
Displaying string-cast Text object: Hàlo agus fàilte

ValueError: U_INVALID_CHAR_FOUND

ValueError: U_INVALID_CHAR_FOUND
