#ifndef PHP_TEXT_H
#define PHP_TEXT_H

#include "php.h"

#include "unicode/ustring.h"
#include "unicode/ucol.h"

extern zend_module_entry text_module_entry;
#define phpext_text_ptr &text_module_entry

#ifdef PHP_WIN32
#define PHP_TEXT_API __declspec(dllexport)
#else
#define PHP_TEXT_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(text)
ZEND_END_MODULE_GLOBALS(text)

typedef struct _php_text_obj php_text_obj;

struct _php_text_obj {
	UChar        *text;
	int32_t       text_len; /* Without trailing \0 */
	UCollator    *collation;
	zend_object   std;
};

static inline php_text_obj *php_text_obj_from_obj(zend_object *obj) {
	return (php_text_obj*)((char*)(obj) - XtOffsetOf(php_text_obj, std));
}
#define Z_PHPTEXT_P(zv)  php_text_obj_from_obj(Z_OBJ_P((zv)))

#endif
