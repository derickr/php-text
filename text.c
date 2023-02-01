#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_text.h"
#include "ext/standard/info.h"

#include "unicode/unorm2.h"

#ifdef COMPILE_DL_TEXT
ZEND_GET_MODULE(text)
#endif

#include "php_text_arginfo.h"

ZEND_DECLARE_MODULE_GLOBALS(text)

static zend_object_handlers text_object_handlers_text;
zend_class_entry *text_ce;

static zend_object *text_object_new_text(zend_class_entry *class_type) /* {{{ */
{
	php_text_obj *intern = zend_object_alloc(sizeof(php_text_obj), class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->text = NULL;

	return &intern->std;
} /* }}} */

static void text_object_free_storage_text(zend_object *object) /* {{{ */
{
	php_text_obj *intern = php_text_obj_from_obj(object);

	if (intern->text) {
		efree(intern->text);
	}

	zend_object_std_dtor(&intern->std);
} /* }}} */

static bool php_text_init_from_utf8_string(php_text_obj *textobj, const char *text_str, size_t text_str_len)
{
	UErrorCode error    = U_ZERO_ERROR;
	int32_t    dest_len = 0;

	if (textobj->text) {
		efree(textobj->text);
	}

	/* Allocate — guess that the buffer is the length of text_str_len + \0 */
	textobj->text = ecalloc(sizeof(UChar),  text_str_len + 1);

	/* Preflighting */
	u_strFromUTF8(textobj->text, text_str_len + 1, &dest_len, text_str, text_str_len, &error);

	if (error == U_ZERO_ERROR) {
		textobj->text_len = dest_len;
		return true;
	}

	if (error != U_BUFFER_OVERFLOW_ERROR && error != U_STRING_NOT_TERMINATED_WARNING) {
		zend_value_error(u_errorName(error));
		return false;
	}

	/* Clean up earlier allocate (too small) buffer, and retry with exact right sized buffer */
	efree(textobj->text);

	textobj->text = ecalloc(sizeof(UChar), dest_len + 1);
	error = U_ZERO_ERROR;

	u_strFromUTF8(textobj->text, text_str_len + 1, NULL, text_str, text_str_len, &error);

	if (U_FAILURE(error)) {
		efree(textobj->text);
		textobj->text = NULL;
		textobj->text_len = 0;
		zend_value_error(u_errorName(error));
		return false;
	}

	textobj->text_len = dest_len;

	return true;
}

static bool php_text_normalize(php_text_obj *textobj)
{
	UErrorCode error = U_ZERO_ERROR;
	const UNormalizer2 *nfc = unorm2_getNFCInstance(&error);
	UChar *converted = NULL;
	int32_t new_len = 0;

	/* Todo: loop in case space is not enough */
	converted = ecalloc(sizeof(UChar), textobj->text_len * 2 + 1);

	/* Normalize */
	new_len = unorm2_normalize(nfc, textobj->text, textobj->text_len, converted, textobj->text_len * 2, &error);

	/* Replace if OK */
	if (error == U_ZERO_ERROR) {
		efree(textobj->text);
		textobj->text = converted;
		textobj->text_len = new_len;

		return true;
	}

	efree(converted);
	zend_value_error(u_errorName(error));
	return false;
}

static bool php_text_to_utf8(php_text_obj *textobj, char **text_str, size_t *text_str_len)
{
	UErrorCode  error    = U_ZERO_ERROR;
	int32_t     dest_len = 0;

	/* Allocate — guess that the buffer is the length of text->str_len + \0 */
	*text_str = ecalloc(1, textobj->text_len + 1);

	/* Preflighting */
	u_strToUTF8(*text_str, textobj->text_len + 1, &dest_len, textobj->text, textobj->text_len, &error);

	if (error == U_ZERO_ERROR) {
		*text_str_len = dest_len;
		return true;
	}

	if (error != U_BUFFER_OVERFLOW_ERROR && error != U_STRING_NOT_TERMINATED_WARNING) {
		efree(*text_str);
		*text_str = NULL;
		return false;
	}

	/* Clean up earlier allocate (too small) buffer, and retry with exact right sized buffer */
	efree(*text_str);

	*text_str = ecalloc(sizeof(UChar), dest_len + 1);
	error = U_ZERO_ERROR;

	u_strToUTF8(*text_str, dest_len, NULL, textobj->text, textobj->text_len, &error);
	if (U_FAILURE(error)) {
		efree(*text_str);
		return false;
	}

	*text_str_len = dest_len;

	return true;
}

PHP_METHOD(Text, __construct)
{
	char   *text_str = NULL;
	size_t  text_str_len = 0;
	char   *collation_str = "root/standard";
	size_t  collation_str_len = 0;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_STRING(text_str, text_str_len)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(collation_str, collation_str_len)
	ZEND_PARSE_PARAMETERS_END();

	if (!php_text_init_from_utf8_string(Z_PHPTEXT_P(ZEND_THIS), text_str, text_str_len)) {
		RETURN_THROWS();
	}
	if (!php_text_normalize(Z_PHPTEXT_P(ZEND_THIS))) {
		RETURN_THROWS();
	}
}

PHP_METHOD(Text, __toString)
{
	char   *converted;
	size_t  converted_len;

	if (!php_text_to_utf8(Z_PHPTEXT_P(ZEND_THIS), &converted, &converted_len)) {
		RETURN_FALSE;
	}

	RETVAL_STRING(converted);
	efree(converted);
}

PHP_MINIT_FUNCTION(text)
{
	text_ce = register_class_Text();
	text_ce->create_object = text_object_new_text;
	text_ce->default_object_handlers = &text_object_handlers_text;
	memcpy(&text_object_handlers_text, &std_object_handlers, sizeof(zend_object_handlers));
	text_object_handlers_text.offset = XtOffsetOf(php_text_obj, std);
	text_object_handlers_text.free_obj = text_object_free_storage_text;
}

PHP_MSHUTDOWN_FUNCTION(text)
{
	u_cleanup();
}

PHP_RINIT_FUNCTION(text)
{
}

PHP_RSHUTDOWN_FUNCTION(text)
{
}

PHP_GINIT_FUNCTION(text)
{
}

PHP_GSHUTDOWN_FUNCTION(text)
{
}

ZEND_MODULE_POST_ZEND_DEACTIVATE_D(text)
{
}


zend_function_entry text_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry text_module_entry = {
	STANDARD_MODULE_HEADER,
	"text",
	text_functions,
	PHP_MINIT(text),
	PHP_MSHUTDOWN(text),
	PHP_RINIT(text),
	PHP_RSHUTDOWN(text),
	NULL,
	"0.0.1",
	PHP_MODULE_GLOBALS(text),
	PHP_GINIT(text),
	PHP_GSHUTDOWN(text),
	ZEND_MODULE_POST_ZEND_DEACTIVATE_N(text),
	STANDARD_MODULE_PROPERTIES_EX
};
