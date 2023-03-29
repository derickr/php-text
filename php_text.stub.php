<?php

/** @generate-class-entries */

final class Text
{
	public function __construct(string|Text $text, string $collation = 'root/standard') {}

	public function __toString() : string {}

	static public function concat(string|Text ...$elements) : Text {}

	static public function join(iterable $elements, string|Text $separator, string $collation = NULL) : Text {}

	public function split(string|Text $subString, int $limit = PHP_INT_MAX) : array {}

	public function toLower() : Text {}

	public function toUpper() : Text {}
}
