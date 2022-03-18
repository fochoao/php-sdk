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
#ifndef PHP_EVENT_ARGINFO_LKS393_H
#define PHP_EVENT_ARGINFO_LKS393_H

#include "src/common.h"
#include "src/util.h"

# define PHP_EVENT_ARG_OBJ_INFO(pass_by_ref, name, classname, allow_null) \
	{ #name, ZEND_TYPE_INIT_CLASS_CONST(PHP_EVENT_NS_NAME(classname), allow_null, _ZEND_ARG_INFO_FLAGS(pass_by_ref, 0)), NULL },

# define PHP_EVENT_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(pass_by_ref, name, classname, allow_null, default_value) \
	{ #name, ZEND_TYPE_INIT_CLASS_CONST(PHP_EVENT_NS_NAME(classname), allow_null, _ZEND_ARG_INFO_FLAGS(pass_by_ref, 0)), default_value },

#include "php_event_arginfo.h" /* Generated arginfo */

#if defined(PHP_EVENT_NS) && defined(PHP_EVENT_NS_RAW)
# define PHP_EVENT_METHODS(classname) PHP_EVENT_CAT(class_, PHP_EVENT_CAT(PHP_EVENT_NS_RAW, PHP_EVENT_CAT(_, PHP_EVENT_CAT(classname, _methods))))
#else
# define PHP_EVENT_METHODS(classname) class_##classname##_methods
#endif /* PHP_EVENT_NS_RAW */

#endif /* PHP_EVENT_ARGINFO_LKS393_H */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
