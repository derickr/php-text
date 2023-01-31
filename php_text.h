#ifndef PHP_TEXT_H
#define PHP_TEXT_H

#include "php.h"

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

#endif
