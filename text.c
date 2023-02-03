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

#define DISPLAY_LOCALE "en_US"
#define DEFAULT_LOCALE "root"

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
	if (intern->collation_name) {
		efree(intern->collation_name);
	}
	if (intern->collation_obj) {
		ucol_close(intern->collation_obj);
	}

	zend_object_std_dtor(&intern->std);
} /* }}} */

static bool php_text_init_from_text(php_text_obj *new_obj, zend_object *object)
{
	php_text_obj *old_obj = php_text_obj_from_obj(object);

	new_obj->text = ecalloc(sizeof(UChar), old_obj->text_len + 1);
	memcpy(new_obj->text, old_obj->text, old_obj->text_len * sizeof(UChar));
	new_obj->text_len = old_obj->text_len;

	return true;
}

static bool php_text_init_from_utf8_string(php_text_obj *textobj, const zend_string *text_str)
{
	UErrorCode error    = U_ZERO_ERROR;
	int32_t    dest_len = 0;

	if (textobj->text) {
		efree(textobj->text);
	}

	/* Allocate — guess that the buffer is the length of text_str_len + \0 */
	textobj->text = ecalloc(sizeof(UChar),  ZSTR_LEN(text_str) + 1);

	/* Preflighting */
	u_strFromUTF8(textobj->text, ZSTR_LEN(text_str) + 1, &dest_len, ZSTR_VAL(text_str), ZSTR_LEN(text_str), &error);

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

	u_strFromUTF8(textobj->text, ZSTR_LEN(text_str) + 1, NULL, ZSTR_VAL(text_str), ZSTR_LEN(text_str), &error);

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

	/* Assume that by default normalisation to NFC does not increase the number of code points */
	converted = ecalloc(sizeof(UChar), textobj->text_len + 1);

	/* Normalize */
	new_len = unorm2_normalize(nfc, textobj->text, textobj->text_len, converted, textobj->text_len, &error);

	/* Replace if OK */
	if (error == U_ZERO_ERROR) {
		efree(textobj->text);
		textobj->text = converted;
		textobj->text_len = new_len;

		return true;
	}

	/* If there isn't a buffer size issue, bail */
	if (error != U_BUFFER_OVERFLOW_ERROR && error != U_STRING_NOT_TERMINATED_WARNING) {
		efree(converted);
		zend_value_error(u_errorName(error));
		return false;
	}

	/* Clean up earlier allocate (too small) buffer, and retry with exact right sized buffer */
	efree(converted);

	converted = ecalloc(sizeof(UChar), new_len + 1);
	error = U_ZERO_ERROR;

	new_len = unorm2_normalize(nfc, textobj->text, textobj->text_len, converted, new_len + 1, &error);

	if (U_FAILURE(error)) {
		efree(converted);
		zend_value_error(u_errorName(error));
		return false;
	}

	efree(textobj->text);
	textobj->text = converted;
	textobj->text_len = new_len;

	return true;
}

static bool php_uchar_to_utf8(UChar *input, int32_t input_len, char **text_str, size_t *text_str_len)
{
	UErrorCode  error    = U_ZERO_ERROR;
	int32_t     dest_len = 0;

	/* Allocate — guess that the buffer is the length of text->str_len + \0 */
	*text_str = ecalloc(1, input_len + 1);

	/* Preflighting */
	u_strToUTF8(*text_str, input_len + 1, &dest_len, input, input_len, &error);

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

	u_strToUTF8(*text_str, dest_len, NULL, input, input_len, &error);
	if (U_FAILURE(error)) {
		efree(*text_str);
		return false;
	}

	*text_str_len = dest_len;

	return true;
}

static bool php_text_to_utf8(php_text_obj *textobj, char **text_str, size_t *text_str_len)
{
	return php_uchar_to_utf8(textobj->text, textobj->text_len, text_str, text_str_len);
}

static bool php_text_attach_locale(php_text_obj *textobj, const char *collation)
{
	UErrorCode error = U_ZERO_ERROR;

	textobj->collation_obj = ucol_open(collation, &error);
	if (U_FAILURE(error)) {
		zend_value_error("Can't open collation '%s': %s", collation, u_errorName(error));
		return false;
	}

	textobj->collation_name = estrdup(collation);

	return true;
}

static bool php_text_clone_locale(php_text_obj *textobj, zend_object *object)
{
	UErrorCode error = U_ZERO_ERROR;
	php_text_obj *old_obj = php_text_obj_from_obj(object);

	textobj->collation_obj = ucol_clone(old_obj->collation_obj, &error);
	if (U_FAILURE(error)) {
		zend_value_error("Can't clone collation: %s", u_errorName(error));
		return false;
	}

	textobj->collation_name = estrdup(old_obj->collation_name);

	return true;
}

#define MAX_COLLATION_DISPLAY_NAME 256

static void text_object_to_hash(php_text_obj *textobj, HashTable *props)
{
	zval       zv;
	char      *text_str;
	size_t     text_len_str;
	UChar     *display_buffer;
	int32_t    display_buffer_len;
	UErrorCode error = U_ZERO_ERROR;

	if (!php_text_to_utf8(textobj, &text_str, &text_len_str)) {
		return;
	}

	/* The text */
	ZVAL_STRING(&zv, text_str);
	zend_hash_str_update(props, "text", sizeof("text")-1, &zv);
	efree(text_str);

	/* The locale */
	display_buffer = ecalloc(sizeof(UChar), MAX_COLLATION_DISPLAY_NAME + 1);
	display_buffer_len = ucol_getDisplayName(textobj->collation_name, DISPLAY_LOCALE, display_buffer, MAX_COLLATION_DISPLAY_NAME, &error);

	if (U_FAILURE(error) || error == U_STRING_NOT_TERMINATED_WARNING) {
		zend_value_error(u_errorName(error));
		efree(display_buffer);
		return;
	}

	if (!php_uchar_to_utf8(display_buffer, display_buffer_len, &text_str, &text_len_str)) {
		efree(display_buffer);
		return;
	}

	ZVAL_STRING(&zv, text_str);
	zend_hash_str_update(props, "collation", sizeof("collation")-1, &zv);

	efree(display_buffer);
	efree(text_str);
}


static HashTable *text_object_get_properties_for(zend_object *object, zend_prop_purpose purpose)
{
	HashTable *props;
	php_text_obj *textobj;

	switch (purpose) {
		case ZEND_PROP_PURPOSE_DEBUG:
		case ZEND_PROP_PURPOSE_SERIALIZE:
		case ZEND_PROP_PURPOSE_VAR_EXPORT:
		case ZEND_PROP_PURPOSE_JSON:
		case ZEND_PROP_PURPOSE_ARRAY_CAST:
			break;
		default:
			return zend_std_get_properties_for(object, purpose);
	}

	textobj = php_text_obj_from_obj(object);
	props = zend_array_dup(zend_std_get_properties(object));
	if (!textobj->text) {
		return props;
	}

	text_object_to_hash(textobj, props);

	return props;
}

PHP_METHOD(Text, __construct)
{
	zval   *z_text_str;
	char   *collation_str = DEFAULT_LOCALE;
	size_t  collation_str_len = 0;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_ZVAL(z_text_str)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(collation_str, collation_str_len)
	ZEND_PARSE_PARAMETERS_END();

	switch (Z_TYPE_P(z_text_str)) {
		case IS_STRING:
			if (!php_text_init_from_utf8_string(Z_PHPTEXT_P(ZEND_THIS), Z_STR_P(z_text_str))) {
				RETURN_THROWS();
			}
			if (!php_text_normalize(Z_PHPTEXT_P(ZEND_THIS))) {
				RETURN_THROWS();
			}
			if (!php_text_attach_locale(Z_PHPTEXT_P(ZEND_THIS), collation_str)) {
				RETURN_THROWS();
			}
			break;

		case IS_OBJECT:
			if (!php_text_init_from_text(Z_PHPTEXT_P(ZEND_THIS), Z_OBJ_P(z_text_str))) {
				RETURN_THROWS();
			}
			if (!php_text_clone_locale(Z_PHPTEXT_P(ZEND_THIS), Z_OBJ_P(z_text_str))) {
				RETURN_THROWS();
			}
			break;

		default: {
			zend_argument_error(NULL, 1, "must be a valid string or Text object");
			RETURN_THROWS();
		}
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
	text_object_handlers_text.get_properties_for = text_object_get_properties_for;
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
