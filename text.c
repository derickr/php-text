#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_text.h"
#include "ext/standard/info.h"
#include "ext/spl/spl_iterators.h"

#include "Zend/zend_interfaces.h"

#include "unicode/unorm2.h"

#ifdef COMPILE_DL_TEXT
ZEND_GET_MODULE(text)
#endif

#include "php_text_arginfo.h"

#define DISPLAY_COLLATION "en_US"
#define DEFAULT_COLLATION "root"
#define MAX_COLLATION_DISPLAY_NAME 256


ZEND_DECLARE_MODULE_GLOBALS(text)

/****************************************************************************
 * Text Object Wrapper Helpers
 */
static zend_object_handlers text_object_handlers_text;
zend_class_entry *text_ce;

static zend_object *text_object_new_text(zend_class_entry *class_type)
{
	php_text_obj *intern = zend_object_alloc(sizeof(php_text_obj), class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->txt = NULL;

	return &intern->std;
}

static void text_object_free_storage_text(zend_object *object)
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
}

/* Initialisation Routines */

static bool php_text_init_from_text(php_text_obj *new_obj, zend_object *object)
{
	php_text_obj *old_obj = php_text_obj_from_obj(object);

	new_obj->txt = old_obj->txt;
	PHP_ICU_TEXT_ADDREF(new_obj->txt);

	return true;
}

static bool php_text_init_from_uchar(php_text_obj *textobj, UChar *val, uint32_t len)
{
	textobj->txt = php_icu_text_ctor_from_uchar(val, len);

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

/* Normalisation Helper */

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

/* Conversion to String helpers */

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
	return php_uchar_to_utf8(PHP_ICU_TEXT_VAL(textobj->txt), PHP_ICU_TEXT_LEN(textobj->txt), text_str, text_str_len);
}

/* Collation Helpers */

static bool php_text_attach_collation(php_text_obj *textobj, const char *collation)
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

static bool php_text_clone_collation(php_text_obj *textobj, zend_object *object)
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

/* Debug and Serialisation Helpers */

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

	/* The collation */
	assert(textobj->collation_name != NULL);
	display_buffer = ecalloc(sizeof(UChar), MAX_COLLATION_DISPLAY_NAME + 1);
	display_buffer_len = ucol_getDisplayName(textobj->collation_name, DISPLAY_COLLATION, display_buffer, MAX_COLLATION_DISPLAY_NAME, &error);

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

/****************************************************************************
 * Text Object Implementation
 */

/* Text::__construct() */

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
			if (!php_text_attach_collation(Z_PHPTEXT_P(ZEND_THIS), collation_str ? collation_str : DEFAULT_COLLATION)) {
				RETURN_THROWS();
			}
			break;

		case IS_OBJECT:
			if (!php_text_init_from_text(Z_PHPTEXT_P(ZEND_THIS), Z_OBJ_P(z_text_str))) {
				RETURN_THROWS();
			}
			if (collation_str) {
				if (!php_text_attach_collation(Z_PHPTEXT_P(ZEND_THIS), collation_str)) {
					RETURN_THROWS();
				}
			} else if (!php_text_clone_collation(Z_PHPTEXT_P(ZEND_THIS), Z_OBJ_P(z_text_str))) {
				RETURN_THROWS();
			}
			break;

		default: {
			zend_argument_error(NULL, 1, "must be a valid string or Text object");
			RETURN_THROWS();
		}
	}
}


/* Text::__toString() */

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


/* Text::concat */

PHP_METHOD(Text, concat)
{
	int      argc, i;
	zval    *args = NULL;
	zval    *arg;
	int32_t  buffer_size_needed = 0;
	struct _php_icu_text *concat_text;

	ZEND_PARSE_PARAMETERS_START(0, -1)
		Z_PARAM_VARIADIC('*', args, argc)
	ZEND_PARSE_PARAMETERS_END();

	if (argc == 0) {
		zend_string *tmp_string = zend_string_init("", 0, 0);

		object_init_ex(return_value, text_ce);

		php_text_init_from_utf8_string(Z_PHPTEXT_P(return_value), tmp_string);
		php_text_attach_collation(Z_PHPTEXT_P(return_value), DEFAULT_COLLATION);

		zend_string_release(tmp_string);
		return;
	}

	PHP_ICU_TEXT_INIT(concat_text);

	for (i = 0; i < argc; i++) {
		zval *arg = args + i;

		if (Z_TYPE_P(arg) == IS_OBJECT) {
			if (!instanceof_function(Z_OBJ_P(arg)->ce, text_ce)) {
				zend_argument_type_error(i + 1, "must be of class Text or a string, %s given", zend_zval_value_name(arg));
				php_icu_text_dtor(concat_text);
				RETURN_THROWS();
			}
			PHP_ICU_TEXT_APPEND_UCHAR(
				concat_text,
				PHP_ICU_TEXT_VAL(Z_PHPTEXT_P(arg)->txt),
				PHP_ICU_TEXT_LEN(Z_PHPTEXT_P(arg)->txt)
			);
		} else if (Z_TYPE_P(arg) == IS_STRING) {
			UErrorCode error;
			struct _php_icu_text *str_text;

			str_text = php_icu_text_ctor_from_zstring(Z_STR_P(arg), &error);

			if (!str_text) {
				zend_argument_error(NULL, i + 1, "%s", u_errorName(error));
				php_icu_text_dtor(concat_text);
				RETURN_THROWS();
			}

			PHP_ICU_TEXT_APPEND_UCHAR(concat_text, PHP_ICU_TEXT_VAL(str_text), PHP_ICU_TEXT_LEN(str_text));
			php_icu_text_dtor(str_text);
		} else {
			zend_argument_type_error(i + 1, "must be of class Text or a string, %s given", zend_zval_value_name(arg));
			php_icu_text_dtor(concat_text);
			RETURN_THROWS();
		}
	}

	object_init_ex(return_value, text_ce);
	Z_PHPTEXT_P(return_value)->txt = php_icu_text_clone(concat_text);

	php_icu_text_dtor(concat_text);

	if (Z_TYPE_P(args) == IS_OBJECT) {
		php_text_clone_collation(Z_PHPTEXT_P(return_value), Z_OBJ_P(args));
	} else {
		php_text_attach_collation(Z_PHPTEXT_P(return_value), DEFAULT_COLLATION);
	}

	if (!php_text_normalize(Z_PHPTEXT_P(return_value))) {
		RETURN_THROWS();
	}
}

/* Text::join() */

struct _php_text_iterator_context
{
	struct _php_icu_text *text;
	struct _php_icu_text *separator;
	const char           *collation_name;
	bool                  error;
};

static int text_join_zval_elements(zval *data, void *puser)
{
	struct _php_text_iterator_context *context = (struct _php_text_iterator_context*)puser;
	struct _php_icu_text *value;

	if (EG(exception)) {
		return ZEND_HASH_APPLY_STOP;
	}
	if (data == NULL) {
		return ZEND_HASH_APPLY_STOP;
	}

	value = php_icu_text_ctor_from_zval_argument(0, data, context->collation_name ? NULL : &context->collation_name);
	if (!value) {
		context->error = true;
		return ZEND_HASH_APPLY_STOP;
	}

	if (!PHP_ICU_TEXT_IS_EMPTY(context->text)) {
		PHP_ICU_TEXT_APPEND_UCHAR(context->text, PHP_ICU_TEXT_VAL(context->separator), PHP_ICU_TEXT_LEN(context->separator));
	}

	PHP_ICU_TEXT_APPEND_UCHAR(context->text, PHP_ICU_TEXT_VAL(value), PHP_ICU_TEXT_LEN(value));

	php_icu_text_dtor(value);

	return ZEND_HASH_APPLY_KEEP;
}

static int text_join_elements(zend_object_iterator *iter, void *puser) /* {{{ */
{
	zval *data = iter->funcs->get_current_data(iter);

	return text_join_zval_elements(data, puser);
}

PHP_METHOD(Text, join)
{
	int      argc, i;
	zval    *args = NULL;
	zval    *arg;
	int32_t  buffer_size_needed = 0;
	UChar   *concat_text;
	zval    *iterator;
	zval    *separator;
	zend_string *collation = NULL;
	struct _php_text_iterator_context context;

	ZEND_PARSE_PARAMETERS_START(2, 3)
		Z_PARAM_ITERABLE(iterator)
		Z_PARAM_ZVAL(separator)
		Z_PARAM_OPTIONAL
		Z_PARAM_STR(collation)
	ZEND_PARSE_PARAMETERS_END();

	/* Setup context with separator */
	context.error = false;
	context.separator = php_icu_text_ctor_from_zval_argument(1, separator, NULL);
	context.text = php_icu_text_ctor_empty();
	context.collation_name = NULL;

	if (!context.separator) {
		RETURN_THROWS();
	}

	if (Z_TYPE_P(iterator) == IS_ARRAY) {
		zend_hash_apply_with_argument(HASH_OF(iterator), text_join_zval_elements, (void*) &context);
	} else {
		spl_iterator_apply(iterator, text_join_elements, (void*) &context);
	}

	if (context.error) {
		php_icu_text_dtor(context.separator);
		RETURN_THROWS();
	}

	object_init_ex(return_value, text_ce);
	Z_PHPTEXT_P(return_value)->txt = php_icu_text_clone(context.text);
	php_icu_text_dtor(context.separator);
	php_icu_text_dtor(context.text);

	if (collation) {
		php_text_attach_collation(Z_PHPTEXT_P(return_value), ZSTR_VAL(collation));
	} else if (context.collation_name) {
		php_text_attach_collation(Z_PHPTEXT_P(return_value), context.collation_name);
	} else {
		php_text_attach_collation(Z_PHPTEXT_P(return_value), DEFAULT_COLLATION);
	}

	if (!php_text_normalize(Z_PHPTEXT_P(return_value))) {
		RETURN_THROWS();
	}
}

/* Text::split() */

PHP_METHOD(Text, split)
{
	zval                 *z_separator;
	long                  limit = ZEND_LONG_MAX;
	struct _php_icu_text *separator;
	UChar                *p1, *endp, *p2;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_ZVAL(z_separator)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(limit)
	ZEND_PARSE_PARAMETERS_END();

	separator = php_icu_text_ctor_from_zval_argument(1, z_separator, NULL);

	p1 = PHP_ICU_TEXT_VAL(Z_PHPTEXT_P(ZEND_THIS)->txt);
	endp = p1 + PHP_ICU_TEXT_LEN(Z_PHPTEXT_P(ZEND_THIS)->txt);
	p2 = u_strFindFirst(p1, -1, PHP_ICU_TEXT_VAL(separator), PHP_ICU_TEXT_LEN(separator));

	array_init(return_value);
	zend_hash_real_init_packed(Z_ARRVAL_P(return_value));
	ZEND_HASH_FILL_PACKED(Z_ARRVAL_P(return_value)) {
		zval new_text;

		do {
			object_init_ex(&new_text, text_ce);
			php_text_init_from_uchar(Z_PHPTEXT(new_text), p1, p2 - p1);
			php_text_attach_collation(Z_PHPTEXT(new_text), Z_PHPTEXT_P(ZEND_THIS)->collation_name);

			ZEND_HASH_FILL_GROW();
			ZEND_HASH_FILL_SET(&new_text);
			ZEND_HASH_FILL_NEXT();

			p1 = p2 + PHP_ICU_TEXT_LEN(separator);
			p2 = u_strFindFirst(p1, -1, PHP_ICU_TEXT_VAL(separator), PHP_ICU_TEXT_LEN(separator));
		} while (p2 != NULL && --limit > 1);

		if (p1 <= endp) {
			object_init_ex(&new_text, text_ce);
			php_text_init_from_uchar(Z_PHPTEXT(new_text), p1, endp - p1);
			php_text_attach_collation(Z_PHPTEXT(new_text), Z_PHPTEXT_P(ZEND_THIS)->collation_name);

			ZEND_HASH_FILL_GROW();
			ZEND_HASH_FILL_SET(&new_text);
			ZEND_HASH_FILL_NEXT();
		}
	} ZEND_HASH_FILL_END();

	php_icu_text_dtor(separator);
}

/****************************************************************************
 * Extension Plumbing
 */

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
