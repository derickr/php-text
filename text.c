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

/* Forward declarations */
static void php_icu_text_dtor(struct _php_icu_text *t);


static struct _php_icu_text* php_icu_text_ctor_from_zstring(const zend_string *text_str, UErrorCode *ret_error)
{
	UErrorCode            error    = U_ZERO_ERROR;
	int32_t               dest_len = 0;

	struct _php_icu_text* new = ecalloc(1, sizeof(struct _php_icu_text));
	new->ref_count++;

	/* Allocate — guess that the buffer is the length of text_str_len + \0 */
	new->val = ecalloc(sizeof(UChar), ZSTR_LEN(text_str) + 1);

	/* Preflighting */
	u_strFromUTF8(new->val, ZSTR_LEN(text_str) + 1, &dest_len, ZSTR_VAL(text_str), ZSTR_LEN(text_str), &error);

	if (error == U_ZERO_ERROR) {
		new->len = dest_len;
		return new;
	}

	if (error != U_BUFFER_OVERFLOW_ERROR && error != U_STRING_NOT_TERMINATED_WARNING) {
		php_icu_text_dtor(new);
		*ret_error = error;
		return NULL;
	}

	/* Clean up earlier allocate (too small) buffer, and retry with exact right sized buffer */
	efree(new->val);

	new->val = ecalloc(sizeof(UChar), dest_len + 1);
	error = U_ZERO_ERROR;

	u_strFromUTF8(new->val, ZSTR_LEN(text_str) + 1, NULL, ZSTR_VAL(text_str), ZSTR_LEN(text_str), &error);

	if (U_FAILURE(error)) {
		php_icu_text_dtor(new);
		*ret_error = error;
		return NULL;
	}

	new->len = dest_len;

	return new;
}

static struct _php_icu_text* php_icu_text_ctor_from_uchar(UChar *val, int32_t len)
{
	struct _php_icu_text* new = ecalloc(1, sizeof(struct _php_icu_text));
	new->ref_count++;

	new->val = val;
	new->len = len;

	return new;
}

static struct _php_icu_text* php_icu_text_ctor_from_text_normalize(struct _php_icu_text *old, UErrorCode *ret_error)
{
	UErrorCode error = U_ZERO_ERROR;
	const UNormalizer2 *nfc = unorm2_getNFCInstance(&error);

	struct _php_icu_text* new = ecalloc(1, sizeof(struct _php_icu_text));
	new->ref_count++;

	/* Assume that by default normalisation to NFC does not increase the number of code points */
	new->val = ecalloc(sizeof(UChar), old->len + 1);

	/* Normalize */
	new->len = unorm2_normalize(nfc, old->val, old->len, new->val, old->len + 1, &error);

	/* Replace if OK */
	if (error == U_ZERO_ERROR) {
		return new;
	}

	/* If there isn't a buffer size issue, bail */
	if (error != U_BUFFER_OVERFLOW_ERROR && error != U_STRING_NOT_TERMINATED_WARNING) {
		php_icu_text_dtor(new);
		*ret_error = error;
		return NULL;
	}

	/* Clean up earlier allocate (too small) buffer, and retry with exact right sized buffer */
	efree(new->val);

	new->val = ecalloc(sizeof(UChar), new->len + 1);
	error = U_ZERO_ERROR;

	new->len = unorm2_normalize(nfc, old->val, old->len, new->val, new->len + 1, &error);

	if (U_FAILURE(error)) {
		php_icu_text_dtor(new);
		*ret_error = error;
		return NULL;
	}

	return new;
}

#define PHP_ICU_TEXT_ADDREF(t) (t)->ref_count++

#if 0
static struct _php_icu_text* php_icu_text_clone(struct _php_icu_text *t)
{
	struct _php_icu_text *new = ecalloc(1, sizeof(struct _php_icu_text));

	new->val = ecalloc(sizeof(UChar), old->len + 1);
	memcpy(new->val, old->val, old->len * sizeof(UChar));
	new->len = old->len;

	new->ref_count++;

	return new;
}
#endif

static void php_icu_text_dtor(struct _php_icu_text *t)
{
	t->ref_count--;

	if (t->ref_count == 0) {
		efree(t->val);
		efree(t);
	}
}

static zend_object *text_object_new_text(zend_class_entry *class_type) /* {{{ */
{
	php_text_obj *intern = zend_object_alloc(sizeof(php_text_obj), class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->txt = NULL;

	return &intern->std;
} /* }}} */

static void text_object_free_storage_text(zend_object *object) /* {{{ */
{
	php_text_obj *intern = php_text_obj_from_obj(object);

	if (intern->txt) {
		php_icu_text_dtor(intern->txt);
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

	new_obj->txt = old_obj->txt;
	PHP_ICU_TEXT_ADDREF(new_obj->txt);

	return true;
}

static bool php_text_init_from_utf8_string(php_text_obj *textobj, const zend_string *text_str)
{
	UErrorCode error    = U_ZERO_ERROR;

	if (textobj->txt) {
		php_icu_text_dtor(textobj->txt);
		textobj->txt = NULL;
	}

	textobj->txt = php_icu_text_ctor_from_zstring(text_str, &error);

	if (!textobj->txt) {
		zend_value_error(u_errorName(error));
		return false;
	}

	return true;
}

static bool php_text_normalize(php_text_obj *textobj)
{
	UErrorCode error = U_ZERO_ERROR;
	struct _php_icu_text *normalized = php_icu_text_ctor_from_text_normalize(textobj->txt, &error);

	if (!normalized) {
		zend_value_error(u_errorName(error));
		return false;
	}

	php_icu_text_dtor(textobj->txt);
	textobj->txt = normalized;

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
	return php_uchar_to_utf8(textobj->txt->val, textobj->txt->len, text_str, text_str_len);
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
	if (!textobj->txt) {
		return props;
	}

	text_object_to_hash(textobj, props);

	return props;
}

PHP_METHOD(Text, __construct)
{
	zval   *z_text_str;
	char   *collation_str = NULL;
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
			if (!php_text_attach_locale(Z_PHPTEXT_P(ZEND_THIS), collation_str ? collation_str : DEFAULT_LOCALE)) {
				RETURN_THROWS();
			}
			break;

		case IS_OBJECT:
			if (!php_text_init_from_text(Z_PHPTEXT_P(ZEND_THIS), Z_OBJ_P(z_text_str))) {
				RETURN_THROWS();
			}
			if (collation_str) {
				if (!php_text_attach_locale(Z_PHPTEXT_P(ZEND_THIS), collation_str)) {
					RETURN_THROWS();
				}
			} else if (!php_text_clone_locale(Z_PHPTEXT_P(ZEND_THIS), Z_OBJ_P(z_text_str))) {
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

PHP_METHOD(Text, concat)
{
	int      argc, i;
	zval    *args = NULL;
	zval    *arg;
	int32_t  buffer_size_needed = 0;
	UChar   *concat_text;

	ZEND_PARSE_PARAMETERS_START(0, -1)
		Z_PARAM_VARIADIC('*', args, argc)
	ZEND_PARSE_PARAMETERS_END();

	if (argc == 0) {
		zend_string *tmp_string = zend_string_init("", 0, 0);

		object_init_ex(return_value, text_ce);

		php_text_init_from_utf8_string(Z_PHPTEXT_P(return_value), tmp_string);
		php_text_attach_locale(Z_PHPTEXT_P(return_value), DEFAULT_LOCALE);

		zend_string_release(tmp_string);
		return;
	}

	/* Loop for type check and buffer size allocation */
	for (i = 0; i < argc; i++) {
		zval *arg = args + i;

		if (Z_TYPE_P(arg) != IS_OBJECT || !instanceof_function(Z_OBJ_P(arg)->ce, text_ce)) {
			zend_argument_type_error(i + 1, "must be of class Text, %s given", zend_zval_value_name(arg));
			RETURN_THROWS();
		}

		buffer_size_needed += Z_PHPTEXT_P(arg)->txt->len;
	}

	/* Allocate */
	concat_text = ecalloc(sizeof(UChar), buffer_size_needed + 1);

	/* Loop to concat */
	int32_t  start_pos = 0;
	for (i = 0; i < argc; i++) {
		zval    *arg = args + i;

		if (Z_TYPE_P(arg) == IS_OBJECT) {
			memcpy((char*) concat_text + start_pos, Z_PHPTEXT_P(arg)->txt->val, Z_PHPTEXT_P(arg)->txt->len * sizeof(UChar));
			start_pos += (Z_PHPTEXT_P(arg)->txt->len * sizeof(UChar));
		} else if (Z_TYPE_P(arg) == IS_STRING) {
		}
	}

	object_init_ex(return_value, text_ce);
	Z_PHPTEXT_P(return_value)->txt = php_icu_text_ctor_from_uchar(concat_text, buffer_size_needed);

	php_text_clone_locale(Z_PHPTEXT_P(return_value), Z_OBJ_P(args));

	if (!php_text_normalize(Z_PHPTEXT_P(return_value))) {
		RETURN_THROWS();
	}
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
