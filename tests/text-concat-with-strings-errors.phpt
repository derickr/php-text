--TEST--
Text::concat with error strings 
--EXTENSIONS--
text
--FILE--
<?php
function show_bytes(string $str)
{
	return bin2hex($str);
}

$stringSets2 = [
	[ "\xC3", 'i kveld!' ],
	[ 'I kveld, ', "\xC3", "\x98l!" ],
	[ 'I kveld, ', "\xC3\x98l!" ],
];
	
foreach ($stringSets2 as $set2) {
	echo "\n===\n\n";

	try {
		$t = Text::concat(
			$set2[0],
			$set2[1],
		);

		echo (string) $t, ' (', show_bytes($t), "):\n";
		
		var_dump($t);
	} catch (Error $e) {
		echo get_class($e), ': ', $e->getMessage(), "\n";
	}
}
?>
--EXPECTF--
===

Error: Text::concat(): Argument #1 U_INVALID_CHAR_FOUND

===

Error: Text::concat(): Argument #2 U_INVALID_CHAR_FOUND

===

I kveld, Øl! (49206b76656c642c20c3986c21):
object(Text)#%d (2) {
  ["text"]=>
  string(13) "I kveld, Øl!"
  ["collation"]=>
  string(16) "Unknown language"
}
