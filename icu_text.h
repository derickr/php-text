#ifndef PHP_ICU_TEXT_H
#define PHP_ICU_TEXT_H

#include "unicode/unorm2.h"

struct _php_icu_text {
	UChar        *tval;
	int32_t       tlen; /* Without trailing \0 */
	int32_t       bufs; /* Buffer size of UChars in *val */
	uint32_t      rc;
};

#define PHP_ICU_TEXT_ADDREF(t) ++(t)->rc
#define PHP_ICU_TEXT_DELREF(t) (--(t)->rc)
#define PHP_ICU_TEXT_VAL(t) (t)->tval
#define PHP_ICU_TEXT_LEN(t) (t)->tlen
#define PHP_ICU_TEXT_ALLOC(t,l) { (t)->tval = ecalloc(sizeof(UChar), (l)+1); (t)->bufs = (l); }

#define PHP_ICU_TEXT_REALLOC(t,l) _php_icu_text_realloc((t), (l))

inline static void _php_icu_text_realloc(struct _php_icu_text *t, int32_t l)
{
	(t)->tval = erealloc((t)->tval, sizeof(UChar) * (l));
	memset((t)->tval + (t)->tlen, 0, sizeof(UChar) * ((l) - (t)->tlen));
	(t)->bufs = l;
}

#define PHP_ICU_TEXT_SETLEN(t,l) { (t)->tlen = l; }
#define PHP_ICU_TEXT_APPEND_UCHAR(t,v,l) _php_icu_text_append_uchar((t), (v), (l))
#define PHP_ICU_TEXT_FREE(t) { efree((t)->tval); (t)->bufs = 0; }

#define PHP_ICU_TEXT_INIT(t) { t = ecalloc(1, sizeof(struct _php_icu_text)); (t)->rc = 1; }

#define PHP_ICU_TEXT_IS_EMPTY(t) ((t)->tlen == 0)

inline static void _php_icu_text_append_uchar(struct _php_icu_text *t, UChar *v, int32_t l)
{
	if (t->tlen + l > t->bufs) {
		PHP_ICU_TEXT_REALLOC(t, (t->bufs + l) * 2);
	}
	memcpy(t->tval + t->tlen, v, l * sizeof(UChar));
	t->tlen += l;
}

struct _php_icu_text* php_icu_text_ctor_empty(void);
struct _php_icu_text* php_icu_text_ctor_from_text_normalize(struct _php_icu_text *old, UErrorCode *ret_error);
struct _php_icu_text* php_icu_text_ctor_from_uchar(UChar *val, int32_t len);
struct _php_icu_text* php_icu_text_ctor_from_zstring(const zend_string *text_str, UErrorCode *ret_error);
struct _php_icu_text* php_icu_text_ctor_from_zval_argument(int i, zval *arg, const char **collation_name);

struct _php_icu_text* php_icu_text_clone(struct _php_icu_text *old);

void php_icu_text_dtor(struct _php_icu_text *t);


#endif
