/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: add17c48c9f4c45ed996874107be1fc8a8e4e74d */

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
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, collation, IS_STRING, 0, "NULL")
ZEND_END_ARG_INFO()


ZEND_METHOD(Text, __construct);
ZEND_METHOD(Text, __toString);
ZEND_METHOD(Text, concat);
ZEND_METHOD(Text, join);


static const zend_function_entry class_Text_methods[] = {
	ZEND_ME(Text, __construct, arginfo_class_Text___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Text, __toString, arginfo_class_Text___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Text, concat, arginfo_class_Text_concat, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Text, join, arginfo_class_Text_join, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Text(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Text", class_Text_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL;

	return class_entry;
}
