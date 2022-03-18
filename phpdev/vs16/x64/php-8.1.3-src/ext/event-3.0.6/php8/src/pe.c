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
#include "common.h"
#include "priv.h"
#include "util.h"

#define PHP_EVENT_PROP_REQUIRE(x) do { \
	if (UNEXPECTED(!(x))) {            \
		return NULL;                   \
	}                                  \
} while (0);

#define PHP_EVENT_PROP_WRITE_REQUIRE(x) do { \
	if (UNEXPECTED(!(x))) return FAILURE;    \
} while (0)


static zend_always_inline void _prop_write_zval(zval *pz, zval *value)
{
	ZVAL_ZVAL(pz, value, 1, 0);
}

static zend_always_inline void _prop_read_zval(zval *pz, zval *retval)
{
	if (Z_ISUNDEF_P(pz)) {
		ZVAL_NULL(retval);
	} else {
		ZVAL_COPY(retval, pz);
	}
}

static zend_always_inline zval * get_ssl_option(const HashTable *ht, zend_ulong idx)/*{{{*/
{
	return zend_hash_index_find(ht, idx);
}/*}}}*/


static zval * event_pending_prop_read(void *obj, zval *retval)/*{{{*/
{
	php_event_t *e = (php_event_t *)obj;

	PHP_EVENT_PROP_REQUIRE(e->event);
	ZVAL_BOOL(retval, (php_event_is_pending(e->event) ? TRUE : FALSE));
	return retval;
}/*}}}*/

static zval * event_data_prop_get_ptr_ptr(void *obj)/*{{{*/
{
	php_event_t *e = (php_event_t *)obj;
	return (EXPECTED(e && e->event) ? &e->data : NULL);
}/*}}}*/

static zval * event_data_prop_read(void *obj, zval *retval)/*{{{*/
{
	php_event_t *e = (php_event_t *)obj;

	PHP_EVENT_PROP_REQUIRE(e->event);
	_prop_read_zval(&e->data, retval);
	return retval;
}/*}}}*/

static int event_data_prop_write(void *obj, zval *value)/*{{{*/
{
	php_event_t *e = (php_event_t *)obj;

	PHP_EVENT_PROP_WRITE_REQUIRE(e->event);
	_prop_write_zval(&e->data, value);
	return SUCCESS;
}/*}}}*/


static zval * event_buffer_length_prop_read(void *obj, zval *retval)/*{{{*/
{
	php_event_buffer_t *b = (php_event_buffer_t *)obj;

	PHP_EVENT_PROP_REQUIRE(b);
	ZVAL_LONG(retval, (b->buf ? evbuffer_get_length(b->buf) : 0));
	return retval;
}/*}}}*/

static zval * event_buffer_contiguous_space_prop_read(void *obj, zval *retval)/*{{{*/
{
	php_event_buffer_t *b = (php_event_buffer_t *)obj;

	PHP_EVENT_PROP_REQUIRE(b->buf);
	ZVAL_LONG(retval, evbuffer_get_contiguous_space(b->buf));
	return retval;
}/*}}}*/

#ifdef HAVE_EVENT_EXTRA_LIB
static zval * event_listener_fd_prop_read(void *obj, zval *retval)/*{{{*/
{
	php_event_listener_t *l = (php_event_listener_t *)obj;
	evutil_socket_t fd;

	if (!l->listener) {
		/* Uninitialized listener */
		ZVAL_NULL(retval);
	} else {
		fd = evconnlistener_get_fd(l->listener);
		if (fd == -1) {
			ZVAL_NULL(retval);
		} else {
			ZVAL_LONG(retval, fd);
		}
	}

	return retval;
}/*}}}*/
#endif

static int event_bevent_priority_prop_write(void *obj, zval *value)/*{{{*/
{
	php_event_bevent_t *bev = (php_event_bevent_t *)obj;

	PHP_EVENT_ASSERT(bev && bev->bevent);
	return (UNEXPECTED(bev == NULL || bev->bevent == NULL)
			|| bufferevent_priority_set(bev->bevent, Z_LVAL_P(value)) ? FAILURE : SUCCESS);
}/*}}}*/

static zval * event_bevent_priority_prop_read(void *obj, zval *retval)/*{{{*/
{
	ZVAL_NULL(retval);
	return retval;
}/*}}}*/

static zval * event_bevent_fd_prop_read(void *obj, zval *retval)/*{{{*/
{
	evutil_socket_t fd;
	php_event_bevent_t *b = (php_event_bevent_t *)obj;

	PHP_EVENT_PROP_REQUIRE(b->bevent);

	fd = bufferevent_getfd(b->bevent);
	if (fd == -1) {
		ZVAL_NULL(retval);
	} else {
		ZVAL_LONG(retval, fd);
	}

	return retval;
}/*}}}*/

static zval * event_bevent_input_prop_read(void *obj, zval *retval)/*{{{*/
{
	php_event_bevent_t *bev = (php_event_bevent_t *)obj;

	PHP_EVENT_PROP_REQUIRE(bev->bevent);

	if (Z_ISUNDEF(bev->input)) {
		php_event_buffer_t *b;

		PHP_EVENT_INIT_CLASS_OBJECT(&bev->input, php_event_buffer_ce);
		b = Z_EVENT_BUFFER_OBJ_P(&bev->input);

		b->buf      = bufferevent_get_input(bev->bevent);
		b->internal = 1;
	}

	ZVAL_COPY(retval, &bev->input);
	return retval;
}/*}}}*/

static zval * event_bevent_output_prop_read(void *obj, zval *retval)/*{{{*/
{
	php_event_bevent_t *bev = (php_event_bevent_t *)obj;

	PHP_EVENT_PROP_REQUIRE(bev->bevent);

	if (Z_ISUNDEF(bev->output)) {
		php_event_buffer_t *b;

		PHP_EVENT_INIT_CLASS_OBJECT(&bev->output, php_event_buffer_ce);
		b = Z_EVENT_BUFFER_OBJ_P(&bev->output);

		b->buf      = bufferevent_get_output(bev->bevent);
		b->internal = 1;
	}

	ZVAL_COPY(retval, &bev->output);
	return retval;
}/*}}}*/

static zval * event_bevent_input_prop_ptr_ptr(void *obj)/*{{{*/
{
	php_event_bevent_t *bev = (php_event_bevent_t *)obj;

	if (EXPECTED(bev) && !Z_ISUNDEF(bev->input)) {
		return &bev->input;
	}

	return NULL;
}/*}}}*/

static zval * event_bevent_output_prop_ptr_ptr(void *obj)/*{{{*/
{
	php_event_bevent_t *bev = (php_event_bevent_t *)obj;
	if (EXPECTED(bev) && !Z_ISUNDEF(bev->output)) {
		return &bev->output;
	}

	return NULL;
}/*}}}*/


#if LIBEVENT_VERSION_NUMBER >= 0x02010100 && defined(HAVE_EVENT_OPENSSL_LIB)
static int event_bevent_allow_ssl_dirty_shutdown_prop_write(void *obj, zval *value)/*{{{*/
{
	php_event_bevent_t *bev = (php_event_bevent_t *)obj;

	PHP_EVENT_PROP_WRITE_REQUIRE(bev && bev->bevent);

	convert_to_boolean(value);
	bufferevent_openssl_set_allow_dirty_shutdown(bev->bevent, (int)(Z_TYPE_INFO_P(value) == IS_TRUE));

	return SUCCESS;
}/*}}}*/

static zval * event_bevent_allow_ssl_dirty_shutdown_prop_read(void *obj, zval *retval)/*{{{*/
{
	php_event_bevent_t *bev = (php_event_bevent_t *)obj;

	ZVAL_BOOL(retval, bev->bevent ? (zend_bool)bufferevent_openssl_get_allow_dirty_shutdown(bev->bevent) : FALSE);
	return retval;
}/*}}}*/
#endif

#ifdef HAVE_EVENT_OPENSSL_LIB
#include "../classes/ssl_context.h"

static int event_ssl_context_local_cert_prop_write(void *obj, zval *value)/*{{{*/
{
	php_event_ssl_context_t *ectx = (php_event_ssl_context_t *)obj;
	zval *val                    = get_ssl_option(ectx->ht, PHP_EVENT_OPT_LOCAL_PK);
	char *private_key            = val ? Z_STRVAL_P(val) : NULL;

	return (_php_event_ssl_ctx_set_local_cert(ectx->ctx, Z_STRVAL_P(value), private_key) ? FAILURE : SUCCESS);
}/*}}}*/

static zval * event_ssl_context_local_cert_prop_read(void *obj, zval *retval)/*{{{*/
{
	php_event_ssl_context_t *ectx = (php_event_ssl_context_t *)obj;
	zval *val                     = get_ssl_option(ectx->ht, PHP_EVENT_OPT_LOCAL_CERT);

	if (val) {
		ZVAL_STRINGL(retval, Z_STRVAL_P(val), Z_STRLEN_P(val));
	} else {
		ZVAL_NULL(retval);
	}

	return retval;
}/*}}}*/

static int event_ssl_context_local_pk_prop_write(void *obj, zval *value)/*{{{*/
{
	php_event_ssl_context_t *ectx = (php_event_ssl_context_t *)obj;
	return (_php_event_ssl_ctx_set_private_key(ectx->ctx, Z_STRVAL_P(value)) ? FAILURE : SUCCESS);
}/*}}}*/

static zval * event_ssl_context_local_pk_prop_read(void *obj, zval *retval)/*{{{*/
{
	php_event_ssl_context_t *ectx = (php_event_ssl_context_t *)obj;
	zval *val                     = get_ssl_option(ectx->ht, PHP_EVENT_OPT_LOCAL_PK);

	if (val) {
		ZVAL_STRINGL(retval, Z_STRVAL_P(val), Z_STRLEN_P(val));
	} else {
		ZVAL_NULL(retval);
	}

	return retval;
}/*}}}*/
#endif /* HAVE_EVENT_OPENSSL_LIB */


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

#if LIBEVENT_VERSION_NUMBER >= 0x02010100 && defined(HAVE_EVENT_OPENSSL_LIB)
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

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
