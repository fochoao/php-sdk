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
#ifndef PHP_EVENT_UTIL_H
#define PHP_EVENT_UTIL_H

#include "common.h"

#ifdef PHP_WIN32
# ifdef EINPROGRESS
#  undef EINPROGRESS
# endif
# define EINPROGRESS WSAEWOULDBLOCK
#endif

#define PHP_EVENT_CAT_(x, y) x ## y
#define PHP_EVENT_CAT(x, y) PHP_EVENT_CAT_(x, y)
#define PHP_EVENT_MN(x) ZEND_MN(x)

#if defined(PHP_EVENT_NS) && defined(PHP_EVENT_NS_RAW)
# define PHP_EVENT_METHOD(classname, name) \
    ZEND_NAMED_FUNCTION(PHP_EVENT_MN(PHP_EVENT_CAT(PHP_EVENT_NS_RAW, PHP_EVENT_CAT(_, PHP_EVENT_CAT(classname, PHP_EVENT_CAT(_, name))))))
#else
# define PHP_EVENT_METHOD(class_name, method) PHP_METHOD(class_name, method)
#endif

php_socket_t php_event_zval_to_fd(zval *pfd);
int _php_event_getsockname(evutil_socket_t fd, zval *pzaddr, zval *pzport);

static zend_always_inline void php_event_init_callback(php_event_callback_t *cb) {/*{{{*/
	ZVAL_UNDEF(&cb->func_name);
	cb->fci_cache = empty_fcall_info_cache;
}/*}}}*/

static zend_always_inline void php_event_free_callback(php_event_callback_t *cb) {/*{{{*/
	if (!Z_ISUNDEF(cb->func_name)) {
		zval_ptr_dtor(&cb->func_name);
	}
}/*}}}*/

static zend_always_inline void php_event_copy_callback(php_event_callback_t *cb, zval *zcb)/*{{{*/
{
	ZVAL_COPY(&cb->func_name, zcb);
	cb->fci_cache = empty_fcall_info_cache;
}/*}}}*/

static zend_always_inline void php_event_replace_callback(php_event_callback_t *cb, zval *zcb)/*{{{*/
{
	php_event_free_callback(cb);
	php_event_copy_callback(cb, zcb);
}/*}}}*/

static zend_always_inline void php_event_copy_zval(zval *zdst, zval *zsrc) {/*{{{*/
	if (zsrc) {
		ZVAL_COPY(zdst, zsrc);
	} else {
		ZVAL_UNDEF(zdst);
	}
}/*}}}*/

static zend_always_inline void php_event_replace_zval(zval *zdst, zval *zsrc) {/*{{{*/
	if (zsrc) {
		if (!Z_ISUNDEF_P(zdst)) {
			zval_ptr_dtor(zdst);
		}
		php_event_copy_zval(zdst, zsrc);
	}
}/*}}}*/

#define php_event_is_pending(e) \
	event_pending((e), EV_READ | EV_WRITE | EV_SIGNAL | EV_TIMEOUT, NULL)

#ifdef PHP_EVENT_NS
# define PHP_EVENT_INIT_CLASS(tmp_ce, name, ce_functions) \
	INIT_NS_CLASS_ENTRY(tmp_ce, PHP_EVENT_NS, name, ce_functions)
#else
# define PHP_EVENT_INIT_CLASS(tmp_ce, name, ce_functions) \
	INIT_CLASS_ENTRY(tmp_ce, name, ce_functions)
#endif

#define PHP_EVENT_REGISTER_CLASS(name, create_func, ce, ce_functions) \
{                                                                     \
	zend_class_entry tmp_ce;                                          \
	PHP_EVENT_INIT_CLASS(tmp_ce, name, ce_functions);                 \
	tmp_ce.create_object = create_func;                               \
	ce = zend_register_internal_class(&tmp_ce);                       \
}

#define PHP_EVENT_DECL_PROP_NULL(ce, name, attr) \
	zend_declare_property_null(ce, #name, sizeof(#name) - 1, attr)

#define PHP_EVENT_INIT_CLASS_OBJECT(pz, pce) object_init_ex((pz), (pce))


#define PHP_EVENT_REG_CLASS_CONST_LONG(pce, const_name, value) \
	zend_declare_class_constant_long((pce), #const_name, sizeof(#const_name) - 1, (zend_long) value)

#define PHP_EVENT_REG_CLASS_CONST_STRING(pce, const_name, value) \
	zend_declare_class_constant_stringl((pce), #const_name, sizeof(#const_name) - 1, value, sizeof(value) - 1);

#define PHP_EVENT_X_OBJ_HANDLERS(x) event_ ## x ## _object_handlers

#define PHP_EVENT_SET_X_OBJ_HANDLER(x, name) \
	PHP_EVENT_X_OBJ_HANDLERS(x).name = php_event_ ## x ## _ ## name

#define PHP_EVENT_SET_X_OBJ_HANDLERS(x) do { \
	PHP_EVENT_X_OBJ_HANDLERS(x).offset = XtOffsetOf(Z_EVENT_X_OBJ_T(x), zo); \
	PHP_EVENT_X_OBJ_HANDLERS(x).get_gc = get_gc; \
	PHP_EVENT_X_OBJ_HANDLERS(x).clone_obj = NULL; \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, free_obj); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, dtor_obj); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, read_property); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, write_property); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, get_property_ptr_ptr); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, has_property); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, get_debug_info); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, get_properties); \
} while (0)

/* php_event_x_fetch_object(zend_object *obj) */
#define Z_EVENT_X_FETCH_OBJ(x, pzo) php_event_ ## x ## _fetch_object(pzo)
#define Z_EVENT_X_FETCH_OBJ_DECL(x) \
	static zend_always_inline Z_EVENT_X_OBJ_T(x) * Z_EVENT_X_FETCH_OBJ(x, zend_object *obj) { \
		return (EXPECTED(obj) ? (Z_EVENT_X_OBJ_T(x) *)((char *)obj - XtOffsetOf(Z_EVENT_X_OBJ_T(x), zo)) : NULL); \
	}

#define Z_EVENT_X_OBJ_P(x, zv) (EXPECTED(zv) ? Z_EVENT_X_FETCH_OBJ(x, Z_OBJ_P(zv)) : NULL)

Z_EVENT_X_FETCH_OBJ_DECL(base)
Z_EVENT_X_FETCH_OBJ_DECL(event)
Z_EVENT_X_FETCH_OBJ_DECL(config)
Z_EVENT_X_FETCH_OBJ_DECL(buffer)
Z_EVENT_X_FETCH_OBJ_DECL(bevent)

#define Z_EVENT_BASE_OBJ_P(zv)   Z_EVENT_X_OBJ_P(base,   zv)
#define Z_EVENT_EVENT_OBJ_P(zv)  Z_EVENT_X_OBJ_P(event,  zv)
#define Z_EVENT_CONFIG_OBJ_P(zv) Z_EVENT_X_OBJ_P(config, zv)
#define Z_EVENT_BUFFER_OBJ_P(zv) Z_EVENT_X_OBJ_P(buffer, zv)
#define Z_EVENT_BEVENT_OBJ_P(zv) Z_EVENT_X_OBJ_P(bevent, zv)

#ifdef HAVE_EVENT_EXTRA_LIB
Z_EVENT_X_FETCH_OBJ_DECL(dns_base)
Z_EVENT_X_FETCH_OBJ_DECL(listener)
Z_EVENT_X_FETCH_OBJ_DECL(http)
Z_EVENT_X_FETCH_OBJ_DECL(http_conn)
Z_EVENT_X_FETCH_OBJ_DECL(http_req)

#define Z_EVENT_DNS_BASE_OBJ_P(zv)  Z_EVENT_X_OBJ_P(dns_base,  zv)
#define Z_EVENT_LISTENER_OBJ_P(zv)  Z_EVENT_X_OBJ_P(listener,  zv)
#define Z_EVENT_HTTP_OBJ_P(zv)      Z_EVENT_X_OBJ_P(http,      zv)
#define Z_EVENT_HTTP_CONN_OBJ_P(zv) Z_EVENT_X_OBJ_P(http_conn, zv)
#define Z_EVENT_HTTP_REQ_OBJ_P(zv)  Z_EVENT_X_OBJ_P(http_req,  zv)
#endif /* HAVE_EVENT_EXTRA_LIB */

#define Z_EVENT_STD_OBJ_DTOR(o) zend_object_std_dtor(&o->zo)

#ifdef HAVE_EVENT_OPENSSL_LIB
Z_EVENT_X_FETCH_OBJ_DECL(ssl_context)

#define Z_EVENT_SSL_CONTEXT_OBJ_P(zv) Z_EVENT_X_OBJ_P(ssl_context, zv)
#endif /* HAVE_EVENT_OPENSSL_LIB */

static zend_always_inline void init_properties(zend_object *pzo, zend_class_entry *ce)/*{{{*/
{
	zend_object_std_init(pzo, ce);
	object_properties_init(pzo, ce);
}/*}}}*/

static zend_always_inline HashTable * find_prop_handler(HashTable *classes, zend_class_entry *ce)/*{{{*/
{
	zend_class_entry *ce_parent = ce;

	while (ce_parent->type != ZEND_INTERNAL_CLASS && ce_parent->parent != NULL) {
		ce_parent = ce_parent->parent;
	}

	return zend_hash_find_ptr(classes, ce_parent->name);
} /*}}}*/

#define PHP_EVENT_OBJ_ALLOC(obj, ce, t)                                \
	do {                                                               \
		obj = ecalloc(1, sizeof(t) + zend_object_properties_size(ce)); \
		obj->prop_handler = find_prop_handler(&classes, ce);           \
		zend_object_std_init(&obj->zo, ce);                            \
		object_properties_init(&obj->zo, ce);                          \
	} while (0)

#define PHP_EVENT_TIMEVAL_SET(tv, t)                     \
	do {                                                 \
		tv.tv_sec  = (zend_long)t;                       \
		tv.tv_usec = (zend_long)((t - tv.tv_sec) * 1e6); \
	} while (0)

#define PHP_EVENT_TIMEVAL_TO_DOUBLE(tv) (tv.tv_sec + tv.tv_usec * 1e-6)

#define PHP_EVENT_SOCKETS_REQUIRED_NORET \
	php_error_docref(NULL, E_ERROR, "`sockets' extension required. " \
			"If you have `sockets' installed, rebuild `event' extension")

#define PHP_EVENT_SOCKETS_REQUIRED_RET    \
	do {                                  \
		PHP_EVENT_SOCKETS_REQUIRED_NORET; \
		RETURN_FALSE;                     \
	} while (0)

/* Obsolete */
#define PHP_EVENT_REQUIRE_BASE_BY_REF(zbase)

#endif /* PHP_EVENT_UTIL_H */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
