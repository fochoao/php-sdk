/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2020 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Ruslan Osmanov <osmanov@php.net>                             |
   +----------------------------------------------------------------------+
*/

#ifndef PHP_EVENT_H
#define PHP_EVENT_H

#define PHP_EVENT_VERSION "3.0.6"

#define PHP_EVENT_SUN_PREFIX "unix:"

extern zend_module_entry event_module_entry;
#define phpext_event_ptr &event_module_entry

#include "src/common.h"

#if PHP_MAJOR_VERSION > 5 || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1)
# define can_handle_soft_dependency_on_SPL 1
#endif

zend_class_entry *php_event_get_exception(void);
zend_class_entry *php_event_get_exception_base(int root TSRMLS_DC);

PHP_MINIT_FUNCTION(event);
PHP_MSHUTDOWN_FUNCTION(event);
PHP_RINIT_FUNCTION(event);
PHP_RSHUTDOWN_FUNCTION(event);
PHP_MINFO_FUNCTION(event);

#if 0
ZEND_BEGIN_MODULE_GLOBALS(event)
	php_stream *ssl_dummy_stream;
ZEND_END_MODULE_GLOBALS(event)
ZEND_EXTERN_MODULE_GLOBALS(event)
#endif

#ifdef ZTS
# define EVENT_G(v) TSRMG(event_globals_id, zend_event_globals *, v)
# define TSRMLS_FETCH_FROM_CTX(ctx) void ***tsrm_ls = (void ***) ctx
# define TSRMLS_SET_CTX(ctx)        ctx = (void ***) tsrm_ls
#else
# define EVENT_G(v) (event_globals.v)
# define TSRMLS_FETCH_FROM_CTX(ctx)
# define TSRMLS_SET_CTX(ctx)
#endif

#ifdef PHP_EVENT_DEBUG
# define PHP_EVENT_ASSERT(x) assert(x)
#else
# define PHP_EVENT_ASSERT(x)
#endif

#ifdef PHP_EVENT_NS
# define PHP_EVENT_NS_NAME(name) PHP_EVENT_NS "\\" #name
#else
# define PHP_EVENT_NS_NAME(name) #name
#endif

#if PHP_VERSION_ID >= 50300
# define PHP_EVENT_FCI_ADDREF(pfci)       \
{                                         \
    Z_ADDREF_P(pfci->function_name);      \
    if (pfci->object_ptr) {               \
        Z_ADDREF_P(pfci->object_ptr);     \
    }                                     \
}
# define PHP_EVENT_FCI_DELREF(pfci)       \
{                                         \
    zval_ptr_dtor(&pfci->function_name);  \
    if (pfci->object_ptr) {               \
        zval_ptr_dtor(&pfci->object_ptr); \
    }                                     \
}
#else
# define PHP_EVENT_FCI_ADDREF(pfci) Z_ADDREF_P(pfci_dst->function_name)
# define PHP_EVENT_FCI_DELREF(pfci) zval_ptr_dtor(&pfci->function_name)
#endif

#define PHP_EVENT_COPY_FCALL_INFO(pfci_dst, pfcc_dst, pfci, pfcc)                                \
    if (ZEND_FCI_INITIALIZED(*pfci)) {                                                           \
        pfci_dst = (zend_fcall_info *) safe_emalloc(1, sizeof(zend_fcall_info), 0);              \
        pfcc_dst = (zend_fcall_info_cache *) safe_emalloc(1, sizeof(zend_fcall_info_cache), 0);  \
                                                                                                 \
        memcpy(pfci_dst, pfci, sizeof(zend_fcall_info));                                         \
        memcpy(pfcc_dst, pfcc, sizeof(zend_fcall_info_cache));                                   \
                                                                                                 \
        PHP_EVENT_FCI_ADDREF(pfci_dst);                                                          \
    } else {                                                                                     \
        pfci_dst = NULL;                                                                         \
        pfcc_dst = NULL;                                                                         \
    }                                                                                            \

#define PHP_EVENT_FREE_FCALL_INFO(pfci, pfcc)                                                    \
    if (pfci && pfcc) {                                                                          \
        efree(pfcc);                                                                             \
        pfcc = NULL;                                                                             \
                                                                                                 \
        if (ZEND_FCI_INITIALIZED(*pfci)) {                                                       \
            PHP_EVENT_FCI_DELREF(pfci);                                                          \
        }                                                                                        \
        efree(pfci);                                                                             \
        pfci = NULL;                                                                             \
    }                                                                                            \

#define PHP_EVENT_LIBEVENT_VERSION_REQUIRED(func, v)                                           \
        php_error_docref(NULL TSRMLS_CC, E_ERROR, #func ##                                     \
                " function requires libevent " ## v ## " or greater. "                         \
                "If you already upgraded libevent, please re-install `event' PECL extension"); \

#endif	/* PHP_EVENT_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
