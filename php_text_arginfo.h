/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: c0ce0d9d087eefc34b50e3c00e9a530774cf7c23 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Text___construct, 0, 0, 1)
	ZEND_ARG_OBJ_TYPE_MASK(0, text, Text, MAY_BE_STRING, NULL)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, collation, IS_STRING, 0, "\'root/standard\'")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Text___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Text_concat, 0, 0, Text, 0)
	ZEND_ARG_VARIADIC_OBJ_TYPE_MASK(0, elements, Text, MAY_BE_STRING)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Text_join, 0, 2, Text, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, elements, Traversable, MAY_BE_ARRAY, NULL)
	ZEND_ARG_OBJ_TYPE_MASK(0, separator, Text, MAY_BE_STRING, NULL)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, collation, IS_STRING, 1, "NULL")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Text_split, 0, 1, IS_ARRAY, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, subString, Text, MAY_BE_STRING, NULL)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, limit, IS_LONG, 0, "PHP_INT_MAX")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Text_contains, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, needle, Text, MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

#define arginfo_class_Text_startsWith arginfo_class_Text_contains

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Text_toLower, 0, 0, Text, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Text_toUpper arginfo_class_Text_toLower

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Text_getByteCount, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Text, __construct);
ZEND_METHOD(Text, __toString);
ZEND_METHOD(Text, concat);
ZEND_METHOD(Text, join);
ZEND_METHOD(Text, split);
ZEND_METHOD(Text, contains);
ZEND_METHOD(Text, startsWith);
ZEND_METHOD(Text, toLower);
ZEND_METHOD(Text, toUpper);
ZEND_METHOD(Text, getByteCount);

static const zend_function_entry class_Text_methods[] = {
	ZEND_ME(Text, __construct, arginfo_class_Text___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Text, __toString, arginfo_class_Text___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Text, concat, arginfo_class_Text_concat, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Text, join, arginfo_class_Text_join, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Text, split, arginfo_class_Text_split, ZEND_ACC_PUBLIC)
	ZEND_ME(Text, contains, arginfo_class_Text_contains, ZEND_ACC_PUBLIC)
	ZEND_ME(Text, startsWith, arginfo_class_Text_startsWith, ZEND_ACC_PUBLIC)
	ZEND_ME(Text, toLower, arginfo_class_Text_toLower, ZEND_ACC_PUBLIC)
	ZEND_ME(Text, toUpper, arginfo_class_Text_toUpper, ZEND_ACC_PUBLIC)
	ZEND_ME(Text, getByteCount, arginfo_class_Text_getByteCount, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Text(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Text", class_Text_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL);

	return class_entry;
}
