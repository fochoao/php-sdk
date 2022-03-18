/*
   +----------------------------------------------------------------------+
   | PHP Version 8                                                        |
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

zend_class_entry *php_event_get_exception_base(int root);
zend_class_entry *php_event_get_exception(void);


PHP_MINIT_FUNCTION(event);
PHP_MSHUTDOWN_FUNCTION(event);
PHP_RINIT_FUNCTION(event);
PHP_RSHUTDOWN_FUNCTION(event);
PHP_MINFO_FUNCTION(event);

#if defined(COMPILE_DL_EVENT)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

#ifdef ZTS
# define EVENT_G(v) TSRMG(event_globals_id, zend_event_globals *, v)
#else
# define EVENT_G(v) (event_globals.v)
#endif

#ifdef PHP_EVENT_NS
# define PHP_EVENT_NS_NAME(name) PHP_EVENT_NS "\\" #name
#else
# define PHP_EVENT_NS_NAME(name) #name
#endif

#ifdef PHP_EVENT_DEBUG
# define PHP_EVENT_ASSERT(x) assert(x)
#else
# define PHP_EVENT_ASSERT(x)
#endif

#define PHP_EVENT_LIBEVENT_VERSION_REQUIRED(func, v)                                           \
        php_error_docref(NULL, E_ERROR, #func ##                                     \
                " function requires libevent " ## v ## " or greater. "                         \
                "If you already upgraded libevent, please re-install `event' PECL extension"); \

#endif /* PHP_EVENT_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
