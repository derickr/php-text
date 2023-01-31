PHP_ARG_ENABLE(text, whether to enable Unicode Text Processing support,
[  --enable-text            Enable text support])

if test "$PHP_TEXT" != "no"; then
  PHP_SETUP_ICU(TEXT_SHARED_LIBADD)
  PHP_SUBST(TEXT_SHARED_LIBADD)

  TEXT_COMMON_FLAGS="$ICU_CFLAGS -Wno-write-strings -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1"
  PHP_NEW_EXTENSION(text, text.c, $ext_shared)
fi
