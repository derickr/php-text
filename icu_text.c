#include "php.h"
#include "unicode/unorm2.h"

#include "php_text.h"
#include "icu_text.h"

extern zend_class_entry *text_ce;

struct _php_icu_text* php_icu_text_ctor_from_zstring(const zend_string *text_str, UErrorCode *ret_error)
{
	UErrorCode            error    = U_ZERO_ERROR;
	int32_t               dest_len = 0;

	struct _php_icu_text* new = ecalloc(1, sizeof(struct _php_icu_text));
	PHP_ICU_TEXT_ADDREF(new);

	/* Allocate — guess that the buffer is the length of text_str_len + \0 */
	PHP_ICU_TEXT_ALLOC(new, ZSTR_LEN(text_str));

	/* Preflighting */
	u_strFromUTF8(PHP_ICU_TEXT_VAL(new), ZSTR_LEN(text_str) + 1, &dest_len, ZSTR_VAL(text_str), ZSTR_LEN(text_str), &error);

	if (error == U_ZERO_ERROR) {
		PHP_ICU_TEXT_SETLEN(new, dest_len);
		return new;
	}

	if (error != U_BUFFER_OVERFLOW_ERROR && error != U_STRING_NOT_TERMINATED_WARNING) {
		php_icu_text_dtor(new);
		*ret_error = error;
		return NULL;
	}

	/* Clean up earlier allocate (too small) buffer, and retry with exact right sized buffer */
	PHP_ICU_TEXT_FREE(new);

	PHP_ICU_TEXT_ALLOC(new, dest_len);
	error = U_ZERO_ERROR;

	u_strFromUTF8(PHP_ICU_TEXT_VAL(new), ZSTR_LEN(text_str) + 1, NULL, ZSTR_VAL(text_str), ZSTR_LEN(text_str), &error);

	if (U_FAILURE(error)) {
		php_icu_text_dtor(new);
		*ret_error = error;
		return NULL;
	}

	PHP_ICU_TEXT_SETLEN(new, dest_len);

	return new;
}

struct _php_icu_text* php_icu_text_ctor_empty(void)
{
	struct _php_icu_text* new = ecalloc(1, sizeof(struct _php_icu_text));
	PHP_ICU_TEXT_ADDREF(new);

	PHP_ICU_TEXT_ALLOC(new, 0);

	return new;
}

struct _php_icu_text* php_icu_text_ctor_from_uchar(UChar *val, int32_t len)
{
	struct _php_icu_text* new = ecalloc(1, sizeof(struct _php_icu_text));
	PHP_ICU_TEXT_ADDREF(new);

	PHP_ICU_TEXT_ALLOC(new, len);
	PHP_ICU_TEXT_APPEND_UCHAR(new, val, len);

	return new;
}

/**
 * Creates a php_icu_text struct from a zval.
 *
 * If the zval contains IS_STRING, convert from UTF-8.
 * If the zval contains IS_OBJECT/Text, clone that.
 *
 * Otherwise, throw an exception, and return NULL.
 */
struct _php_icu_text* php_icu_text_ctor_from_zval_argument(int i, zval *arg, const char **collation_name)
{
	if (Z_TYPE_P(arg) == IS_OBJECT) {
		if (!instanceof_function(Z_OBJ_P(arg)->ce, text_ce)) {
			zend_argument_type_error(i + 1, "must be of class Text or a string, %s given", zend_zval_value_name(arg));
			return NULL;
		}
		if (collation_name) {
			*collation_name = php_text_obj_from_obj(Z_OBJ_P(arg))->collation_name;
		}
		return php_icu_text_clone(php_text_obj_from_obj(Z_OBJ_P(arg))->txt);
	} else if (Z_TYPE_P(arg) == IS_STRING) {
		UErrorCode error;
		struct _php_icu_text *converted;

		converted = php_icu_text_ctor_from_zstring(Z_STR_P(arg), &error);

		if (!converted) {
			zend_argument_error(NULL, i + 1, "%s", u_errorName(error));
			return NULL;
		}

		return converted;
	} else {
		zend_argument_type_error(i + 1, "must be of class Text or a string, %s given", zend_zval_value_name(arg));

		return NULL;
	}
}

struct _php_icu_text* php_icu_text_ctor_from_text_normalize(struct _php_icu_text *old, UErrorCode *ret_error)
{
	UErrorCode error = U_ZERO_ERROR;
	const UNormalizer2 *nfc = unorm2_getNFCInstance(&error);
	int32_t new_len;
	struct _php_icu_text* new;

	PHP_ICU_TEXT_INIT(new);

	/* Assume that by default normalisation to NFC does not increase the number of code points */
	PHP_ICU_TEXT_ALLOC(new, PHP_ICU_TEXT_LEN(old));

	/* Normalize */
	new_len = unorm2_normalize(nfc, PHP_ICU_TEXT_VAL(old), PHP_ICU_TEXT_LEN(old), PHP_ICU_TEXT_VAL(new), PHP_ICU_TEXT_LEN(old) + 1, &error);

	/* Replace if OK */
	if (error == U_ZERO_ERROR) {
		PHP_ICU_TEXT_SETLEN(new, new_len);
		return new;
	}

	/* If there isn't a buffer size issue, bail */
	if (error != U_BUFFER_OVERFLOW_ERROR && error != U_STRING_NOT_TERMINATED_WARNING) {
		php_icu_text_dtor(new);
		*ret_error = error;
		return NULL;
	}

	/* Clean up earlier allocate (too small) buffer, and retry with exact right sized buffer */
	PHP_ICU_TEXT_FREE(new);

	PHP_ICU_TEXT_ALLOC(new, new_len);
	error = U_ZERO_ERROR;

	new->tlen = unorm2_normalize(nfc, PHP_ICU_TEXT_VAL(old), PHP_ICU_TEXT_LEN(old), PHP_ICU_TEXT_VAL(new), new_len + 1, &error);

	if (U_FAILURE(error)) {
		php_icu_text_dtor(new);
		*ret_error = error;
		return NULL;
	}

	return new;
}

struct _php_icu_text* php_icu_text_clone(struct _php_icu_text *old)
{
	struct _php_icu_text *new;
	PHP_ICU_TEXT_INIT(new);

	PHP_ICU_TEXT_ALLOC(new, PHP_ICU_TEXT_LEN(old));
	PHP_ICU_TEXT_APPEND_UCHAR(new, PHP_ICU_TEXT_VAL(old), PHP_ICU_TEXT_LEN(old));

	return new;
}

void php_icu_text_dtor(struct _php_icu_text *t)
{
	if (!PHP_ICU_TEXT_DELREF(t)) {
		efree(t->tval);
		efree(t);
	}
}
