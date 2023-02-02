--TEST--
Text __construct normalization
--EXTENSIONS--
text
--FILE--
<?php
function show_bytes(string $str)
{
	return bin2hex($str);
}

$strings = [
	"\u{212b}"          => 'Å',
	"A\u{030a}"         => 'Å',
	"\u{2126}"          => "\u{03a9}",
	"\u{1e0b}\u{0323}"  => "\u{1e0d}\u{0307}",
	"q\u{0307}\u{0323}" => "q\u{0323}\u{0307}",
	"\u{fb2e}"          => "\u{05d0}\u{05bf}",
	"\u{fb2e}\u{fb38}"  => "\u{05d0}\u{05bf}\u{05d8}\u{05bc}",
	"\u{01d161}"        => "\u{01d158}\u{01d165}\u{01d16f}",
];

foreach ($strings as $input => $output)
{
	$t = new \Text($input);

	echo "Created Text object:                {$input} (", show_bytes($input), ")\n";

	echo 'Displaying string-cast Text object: ';
	echo (string) $t, ' (', show_bytes($t), ")\n";
	echo 'Expected:                           ';
	echo $output, ' (', show_bytes($output), ")\n\n";
}
?>
--EXPECT--
Created Text object:                Å (e284ab)
Displaying string-cast Text object: Å (c385)
Expected:                           Å (c385)

Created Text object:                Å (41cc8a)
Displaying string-cast Text object: Å (c385)
Expected:                           Å (c385)

Created Text object:                Ω (e284a6)
Displaying string-cast Text object: Ω (cea9)
Expected:                           Ω (cea9)

Created Text object:                ḍ̇ (e1b88bcca3)
Displaying string-cast Text object: ḍ̇ (e1b88dcc87)
Expected:                           ḍ̇ (e1b88dcc87)

Created Text object:                q̣̇ (71cc87cca3)
Displaying string-cast Text object: q̣̇ (71cca3cc87)
Expected:                           q̣̇ (71cca3cc87)

Created Text object:                אַ (efacae)
Displaying string-cast Text object: אַ (d790d6b7)
Expected:                           אֿ (d790d6bf)

Created Text object:                אַטּ (efacaeefacb8)
Displaying string-cast Text object: אַטּ (d790d6b7d798d6bc)
Expected:                           אֿטּ (d790d6bfd798d6bc)

Created Text object:                𝅘𝅥𝅯 (f09d85a1)
Displaying string-cast Text object: 𝅘𝅥𝅯 (f09d8598f09d85a5f09d85af)
Expected:                           𝅘𝅥𝅯 (f09d8598f09d85a5f09d85af)
