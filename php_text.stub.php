<?php

/** @generate-class-entries */

final class Text
{
	public function __construct(string|Text $text, string $collation = 'root/standard') {}

	public function __toString() : string {}

	/**
	 * @param string|Text $elements
	 */
	static public function concat(...$elements) : Text {}
}
