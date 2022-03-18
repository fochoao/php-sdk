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
#ifndef PHP_EVENT_UTIL_H
#define PHP_EVENT_UTIL_H

#include "common.h"

#ifdef PHP_WIN32
# ifdef EINPROGRESS
#  undef EINPROGRESS
# endif
# define EINPROGRESS WSAEWOULDBLOCK
#endif

php_socket_t php_event_zval_to_fd(zval **ppfd TSRMLS_DC);
int _php_event_getsockname(evutil_socket_t fd, zval **ppzaddress, zval **ppzport TSRMLS_DC);

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
	ce = zend_register_internal_class(&tmp_ce TSRMLS_CC);             \
	ce->create_object = create_func;                                  \
}

#define PHP_EVENT_INIT_CLASS_OBJECT(pz, pce) \
	do {                                     \
		Z_TYPE_P((pz)) = IS_OBJECT;          \
		object_init_ex((pz), (pce));         \
		Z_SET_REFCOUNT_P((pz), 1);           \
		Z_SET_ISREF_P((pz));                 \
	} while (0)

#define PHP_EVENT_FETCH_EVENT(e, ze) \
	e = (php_event_t *) zend_object_store_get_object(ze TSRMLS_CC)

#define PHP_EVENT_FETCH_BASE(base, zbase) \
	base = (php_event_base_t *) zend_object_store_get_object(zbase TSRMLS_CC)

#define PHP_EVENT_FETCH_CONFIG(cfg, zcfg) \
	cfg = (php_event_config_t *) zend_object_store_get_object(zcfg TSRMLS_CC)

#define PHP_EVENT_FETCH_BEVENT(b, zb) \
	b = (php_event_bevent_t *) zend_object_store_get_object(zb TSRMLS_CC)

#define PHP_EVENT_FETCH_BUFFER(b, zb) \
	b = (php_event_buffer_t *) zend_object_store_get_object(zb TSRMLS_CC)

#define PHP_EVENT_FETCH_DNS_BASE(b, zb) \
	b = (php_event_dns_base_t *) zend_object_store_get_object(zb TSRMLS_CC)

#define PHP_EVENT_FETCH_LISTENER(b, zb) \
	b = (php_event_listener_t *) zend_object_store_get_object(zb TSRMLS_CC)

#define PHP_EVENT_FETCH_HTTP_CONN(b, zb) \
	b = (php_event_http_conn_t *) zend_object_store_get_object(zb TSRMLS_CC)

#define PHP_EVENT_FETCH_HTTP(b, zb) \
	b = (php_event_http_t *) zend_object_store_get_object(zb TSRMLS_CC)

#define PHP_EVENT_FETCH_HTTP_REQ(b, zb) \
	b = (php_event_http_req_t *) zend_object_store_get_object(zb TSRMLS_CC)

#define PHP_EVENT_FETCH_BUFFER_POS(p, zp) \
	p = (php_event_buffer_pos_t *) zend_object_store_get_object(zp TSRMLS_CC)

#define PHP_EVENT_FETCH_SSL_CONTEXT(p, zp) \
	p = (php_event_ssl_context_t *) zend_object_store_get_object(zp TSRMLS_CC)

#define PHP_EVENT_TIMEVAL_SET(tv, t)                 \
	do {                                             \
		tv.tv_sec  = (long) t;                       \
		tv.tv_usec = (long) ((t - tv.tv_sec) * 1e6); \
	} while (0)

#define PHP_EVENT_TIMEVAL_TO_DOUBLE(tv) (tv.tv_sec + tv.tv_usec * 1e-6)

#define PHP_EVENT_SOCKETS_REQUIRED_NORET                                       \
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "`sockets' extension required. " \
			"If you have `sockets' installed, rebuild `event' extension")

#define PHP_EVENT_SOCKETS_REQUIRED_RET    \
	do {                                  \
		PHP_EVENT_SOCKETS_REQUIRED_NORET; \
		RETURN_FALSE;                     \
	} while (0)

#define PHP_EVENT_REQUIRE_BASE_BY_REF(zbase)                  \
	do {                                                      \
		if (!Z_ISREF_P((zbase)) || Z_REFCOUNT_P(zbase) < 2) { \
			php_error_docref(NULL TSRMLS_CC, E_ERROR,         \
					"EventBase must be passed by reference"); \
		}                                                     \
	} while (0)

#if defined(PHP_WIN32)
#if defined(ZTS)
#  define PHP_EVENT_TSRMLS_FETCH_FROM_CTX(ctx) tsrm_ls = (void ***)ctx
#  define PHP_EVENT_TSRM_DECL void ***tsrm_ls;
# else
#  define PHP_EVENT_TSRMLS_FETCH_FROM_CTX(ctx)
#  define PHP_EVENT_TSRM_DECL
# endif
#else
# define PHP_EVENT_TSRMLS_FETCH_FROM_CTX(ctx) TSRMLS_FETCH_FROM_CTX(ctx)
# define PHP_EVENT_TSRM_DECL
#endif

#endif /* PHP_EVENT_UTIL_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
