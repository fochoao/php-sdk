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
#include "common.h"
#include "priv.h"
#include "util.h"

#define PHP_EVENT_PROP_REQUIRE(x) \
	do {                          \
		if (!(x)) return FAILURE; \
	} while (0);

static inline void _prop_write_zval(zval **ppz, zval *value)
{
#if 0
	if (!*ppz) {
		MAKE_STD_ZVAL(*ppz);
	}

	/* Make a copy of the zval, avoid direct binding to the address
	 * of value, since it breaks refcount in read_property()
	 * causing further leaks and memory access violations */
	REPLACE_ZVAL_VALUE(ppz, value, PZVAL_IS_REF((zval *)value));
#endif
	if (!*ppz) {
		/* if we assign referenced variable, we should separate it */
		Z_ADDREF_P(value);
		if (PZVAL_IS_REF(value)) {
			SEPARATE_ZVAL(&value);
		}
		*ppz = value;
	} else if (PZVAL_IS_REF(*ppz)) {
		zval garbage = **ppz; /* old value should be destroyed */

		/* To check: can't *ppz be some system variable like error_zval here? */
		Z_TYPE_PP(ppz) = Z_TYPE_P(value);
		(*ppz)->value = value->value;
		if (Z_REFCOUNT_P(value) > 0) {
			zval_copy_ctor(*ppz);
		}
		zval_dtor(&garbage);
	} else {
		zval *garbage = *ppz;

		/* if we assign referenced variable, we should separate it */
		Z_ADDREF_P(value);
		if (PZVAL_IS_REF(value)) {
			SEPARATE_ZVAL(&value);
		}
		*ppz = value;
		zval_ptr_dtor(&garbage);
	}
}

static inline void _prop_read_zval(zval *pz, zval **retval)
{
	if (!pz) {
		ALLOC_INIT_ZVAL(*retval);
		return;
	}

	MAKE_STD_ZVAL(*retval);
	ZVAL_ZVAL(*retval, pz, 1, 0);
}



#ifdef HAVE_EVENT_OPENSSL_LIB
/* {{{ get_ssl_option */
static zval **get_ssl_option(const HashTable *ht, ulong idx)
{
	zval **val;

	if (zend_hash_index_find(ht, idx, (void **) &val) == SUCCESS) {
		return val;
	}

	return NULL;
}
/* }}} */
#endif


/* {{{ event_pending_prop_read */
static int event_pending_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_t *e = (php_event_t *) obj;

	PHP_EVENT_PROP_REQUIRE(e->event);

	MAKE_STD_ZVAL(*retval);
	ZVAL_BOOL(*retval, (php_event_is_pending(e->event) ? 1 : 0));

	return SUCCESS;
}
/* }}} */


/* {{{ event_data_prop_get_ptr_ptr */
static zval **event_data_prop_get_ptr_ptr(php_event_abstract_object_t *obj TSRMLS_DC)
{
	php_event_t *e = (php_event_t *) obj;

	if (!e->event) return NULL;
	if (!e->data) {
		MAKE_STD_ZVAL(e->data);
	}
	return &e->data;
}
/* }}} */

/* {{{ event_data_prop_read  */
static int event_data_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_t *e = (php_event_t *) obj;

	PHP_EVENT_PROP_REQUIRE(e->event);

	_prop_read_zval(e->data, retval);

	return SUCCESS;
}
/* }}} */

/* {{{ event_data_prop_write */
static int event_data_prop_write(php_event_abstract_object_t *obj, zval *value TSRMLS_DC)
{
	php_event_t *e = (php_event_t *) obj;

	PHP_EVENT_PROP_REQUIRE(e->event);

	_prop_write_zval(&e->data, value);

	return SUCCESS;
}
/* }}} */


/* {{{ event_buffer_length_prop_read */
static int event_buffer_length_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_buffer_t *b = (php_event_buffer_t *) obj;

	PHP_EVENT_PROP_REQUIRE(b->buf);

	MAKE_STD_ZVAL(*retval);
	if (b && b->buf){
		ZVAL_LONG(*retval, evbuffer_get_length(b->buf));
	} else {
		ZVAL_LONG(*retval, 0);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ event_buffer_contiguous_space_prop_read */
static int event_buffer_contiguous_space_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_buffer_t *b = (php_event_buffer_t *) obj;

	PHP_EVENT_PROP_REQUIRE(b->buf);

	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, evbuffer_get_contiguous_space(b->buf));

	return SUCCESS;
}
/* }}} */

#ifdef HAVE_EVENT_EXTRA_LIB
/* {{{ event_listener_fd_prop_read */
static int event_listener_fd_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_listener_t *l = (php_event_listener_t *) obj;
	evutil_socket_t fd;

	MAKE_STD_ZVAL(*retval);

	if (!l->listener) {
		/* Uninitialized listener */
		ZVAL_NULL(*retval);
		return SUCCESS;
	}

	fd = evconnlistener_get_fd(l->listener);
	if (fd == -1) {
		ZVAL_NULL(*retval);
	} else {
		ZVAL_LONG(*retval, fd);
	}

	return SUCCESS;
}
/* }}} */
#endif

/* {{{ event_bevent_priority_prop_write*/
static int event_bevent_priority_prop_write(php_event_abstract_object_t *obj, zval *value TSRMLS_DC)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) obj;
	long priority           = Z_LVAL_P(value);

	if (bufferevent_priority_set(bev->bevent, priority)) {
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ event_bevent_priority_prop_read */
static int event_bevent_priority_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	ALLOC_INIT_ZVAL(*retval);
	return SUCCESS;
}
/* }}} */

/* {{{ event_bevent_fd_prop_read */
static int event_bevent_fd_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_bevent_t *b = (php_event_bevent_t *) obj;
	evutil_socket_t fd;

	MAKE_STD_ZVAL(*retval);

	/* Uninitialized / free'd */
#if 0
	if (!b->bevent) {
		ZVAL_NULL(*retval);
		return SUCCESS;
	}
#endif
	PHP_EVENT_PROP_REQUIRE(b->bevent);

	fd = bufferevent_getfd(b->bevent);
	if (fd == -1) {
		ZVAL_NULL(*retval);
	} else {
		ZVAL_LONG(*retval, fd);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ event_bevent_input_prop_read */
static int event_bevent_input_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) obj;

	if (!bev->bevent) {
		return FAILURE;
	}

	if (!bev->input) {
		php_event_buffer_t *b;

		MAKE_STD_ZVAL(bev->input);
		PHP_EVENT_INIT_CLASS_OBJECT(bev->input, php_event_buffer_ce);
		PHP_EVENT_FETCH_BUFFER(b, bev->input);

		b->buf      = bufferevent_get_input(bev->bevent);
		b->internal = 1;
	}

	MAKE_STD_ZVAL(*retval);

	ZVAL_ZVAL(*retval, bev->input, 1, 0);
	Z_SET_ISREF_P(*retval);
	Z_ADDREF_P(*retval);
	return SUCCESS;
}
/* }}} */

/* {{{ event_bevent_output_prop_read */
static int event_bevent_output_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) obj;

	if (!bev->bevent) {
		return FAILURE;
	}

	if (!bev->output) {
		php_event_buffer_t *b;

		MAKE_STD_ZVAL(bev->output);
		PHP_EVENT_INIT_CLASS_OBJECT(bev->output, php_event_buffer_ce);
		PHP_EVENT_FETCH_BUFFER(b, bev->output);

		b->buf      = bufferevent_get_output(bev->bevent);
		b->internal = 1;
	}

	MAKE_STD_ZVAL(*retval);

	ZVAL_ZVAL(*retval, bev->output, 1, 0);
	Z_SET_ISREF_P(*retval);
	Z_ADDREF_P(*retval);
	return SUCCESS;
}
/* }}} */


/* {{{ event_bevent_input_prop_ptr_ptr */
static zval **event_bevent_input_prop_ptr_ptr(php_event_abstract_object_t *obj TSRMLS_DC)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) obj;

	return bev->input ? &bev->input : NULL;
}
/* }}} */

/* {{{ event_bevent_output_prop_ptr_ptr */
static zval **event_bevent_output_prop_ptr_ptr(php_event_abstract_object_t *obj TSRMLS_DC)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) obj;

	return bev->output ? &bev->output : NULL;
}
/* }}} */


#if LIBEVENT_VERSION_NUMBER >= 0x02010100
/* {{{ event_bevent_allow_ssl_dirty_shutdown_prop_write*/
static int event_bevent_allow_ssl_dirty_shutdown_prop_write(php_event_abstract_object_t *obj, zval *value TSRMLS_DC)
{
	php_event_bevent_t *bev      = (php_event_bevent_t *) obj;
	int allow_ssl_dirty_shutdown = (int) Z_BVAL_P(value);

	bufferevent_openssl_set_allow_dirty_shutdown(bev->bevent, allow_ssl_dirty_shutdown);
	return SUCCESS;
}
/* }}} */

/* {{{ event_bevent_allow_ssl_dirty_shutdown_prop_read */
static int event_bevent_allow_ssl_dirty_shutdown_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_bevent_t *bev = (php_event_bevent_t *)obj;

	MAKE_STD_ZVAL(*retval);
	ZVAL_BOOL(*retval, (zend_bool) bufferevent_openssl_get_allow_dirty_shutdown(bev->bevent));
	return SUCCESS;
}
/* }}} */
#endif

#ifdef HAVE_EVENT_OPENSSL_LIB
#include "../classes/ssl_context.h"

/* {{{ event_ssl_context_local_cert_prop_write*/
static int event_ssl_context_local_cert_prop_write(php_event_abstract_object_t *obj, zval *value TSRMLS_DC)
{
	php_event_ssl_context_t *ectx = (php_event_ssl_context_t *) obj;
	zval **val                    = get_ssl_option(ectx->ht, PHP_EVENT_OPT_LOCAL_PK);
	char *private_key             = val ? Z_STRVAL_PP(val) : NULL;

	if (_php_event_ssl_ctx_set_local_cert(ectx->ctx, Z_STRVAL_P(value), private_key TSRMLS_CC)) {
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ event_ssl_context_local_cert_prop_read */
static int event_ssl_context_local_cert_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_ssl_context_t *ectx = (php_event_ssl_context_t *) obj;
	zval **val                    = get_ssl_option(ectx->ht, PHP_EVENT_OPT_LOCAL_CERT);

	if (val) {
		MAKE_STD_ZVAL(*retval);
		ZVAL_STRINGL(*retval, Z_STRVAL_PP(val), Z_STRLEN_PP(val), 1);
	} else {
		ALLOC_INIT_ZVAL(*retval);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ event_ssl_context_local_pk_prop_write */
static int event_ssl_context_local_pk_prop_write(php_event_abstract_object_t *obj, zval *value TSRMLS_DC)
{
	php_event_ssl_context_t *ectx = (php_event_ssl_context_t *) obj;

	if (_php_event_ssl_ctx_set_private_key(ectx->ctx, Z_STRVAL_P(value) TSRMLS_CC)) {
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ event_ssl_context_local_pk_prop_read */
static int event_ssl_context_local_pk_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_ssl_context_t *ectx = (php_event_ssl_context_t *) obj;
	zval **val                    = get_ssl_option(ectx->ht, PHP_EVENT_OPT_LOCAL_PK);

	if (val) {
		MAKE_STD_ZVAL(*retval);
		ZVAL_STRINGL(*retval, Z_STRVAL_PP(val), Z_STRLEN_PP(val), 1);
	} else {
		ALLOC_INIT_ZVAL(*retval);
	}

	return SUCCESS;
}
/* }}} */
#endif


const php_event_property_entry_t event_property_entries[] = {
	{"pending", sizeof("pending") - 1, event_pending_prop_read, NULL,                  NULL},
	{"data",    sizeof("data")    - 1, event_data_prop_read,    event_data_prop_write, event_data_prop_get_ptr_ptr},
	{NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_bevent_property_entries[] = {
	{"priority", sizeof("priority") - 1, event_bevent_priority_prop_read, event_bevent_priority_prop_write, NULL                               },
	{"fd",       sizeof("fd")       - 1, event_bevent_fd_prop_read,       NULL,                             NULL                               },
	{"input",    sizeof("input")    - 1, event_bevent_input_prop_read,    NULL,                             event_bevent_input_prop_ptr_ptr},
	{"output",   sizeof("output")   - 1, event_bevent_output_prop_read,   NULL,                             event_bevent_output_prop_ptr_ptr},

#if LIBEVENT_VERSION_NUMBER >= 0x02010100
	{"allow_ssl_dirty_shutdown", sizeof("allow_ssl_dirty_shutdown") - 1,
		event_bevent_allow_ssl_dirty_shutdown_prop_read,
		event_bevent_allow_ssl_dirty_shutdown_prop_write, NULL },
#endif
	{NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_buffer_property_entries[] = {
	{"length",           sizeof("length")           - 1, event_buffer_length_prop_read,           NULL, NULL},
	{"contiguous_space", sizeof("contiguous_space") - 1, event_buffer_contiguous_space_prop_read, NULL, NULL},
	{NULL, 0, NULL, NULL, NULL}
};
#ifdef HAVE_EVENT_EXTRA_LIB
const php_event_property_entry_t event_listener_property_entries[] = {
	{"fd", sizeof("fd") - 1, event_listener_fd_prop_read, NULL, NULL},
	{NULL, 0, NULL, NULL, NULL}
};
#endif
#ifdef HAVE_EVENT_OPENSSL_LIB
const php_event_property_entry_t event_ssl_context_property_entries[] = {
	{"local_cert", sizeof("local_cert") - 1, event_ssl_context_local_cert_prop_read, event_ssl_context_local_cert_prop_write, NULL},
	{"local_pk", sizeof("local_pk") - 1, event_ssl_context_local_pk_prop_read, event_ssl_context_local_pk_prop_write, NULL},
	{NULL, 0, NULL, NULL, NULL}
};
#endif

const zend_property_info event_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "pending", sizeof("pending") - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "data",    sizeof("data")    - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_bevent_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "priority", sizeof("priority") - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "fd",       sizeof("fd")       - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "input",    sizeof("input")    - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "output",   sizeof("output")   - 1, -1, 0, NULL, 0, NULL},
#if LIBEVENT_VERSION_NUMBER >= 0x02010100
	{ZEND_ACC_PUBLIC, "allow_ssl_dirty_shutdown", sizeof("allow_ssl_dirty_shutdown") - 1, -1, 0, NULL, 0, NULL},
#endif
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_buffer_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "length",           sizeof("length")           - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "contiguous_space", sizeof("contiguous_space") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
#ifdef HAVE_EVENT_EXTRA_LIB
const zend_property_info event_listener_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "fd", sizeof("fd") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
#endif
#ifdef HAVE_EVENT_OPENSSL_LIB
const zend_property_info event_ssl_context_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "local_cert", sizeof("local_cert") - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "local_pk", sizeof("local_pk") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
