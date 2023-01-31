/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: bcd3a967345dc7fffa8e960e8895511bc4e81ced */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Text___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, collation, IS_STRING, 0, "\'root/standard\'")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Text___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Text, __construct);
ZEND_METHOD(Text, __toString);


static const zend_function_entry class_Text_methods[] = {
	ZEND_ME(Text, __construct, arginfo_class_Text___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Text, __toString, arginfo_class_Text___toString, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Text(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Text", class_Text_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);

	return class_entry;
}
