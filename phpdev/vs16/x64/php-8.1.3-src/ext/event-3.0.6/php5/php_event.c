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

#include "src/common.h"
#include "src/util.h"
#include "src/priv.h"
#include "classes/http.h"
#include "zend_exceptions.h"

#if 0
ZEND_DECLARE_MODULE_GLOBALS(event)
static PHP_GINIT_FUNCTION(event);
#endif

extern const zend_property_info event_listener_property_entry_info[];
extern const php_event_property_entry_t event_listener_property_entries[];

zend_class_entry *php_event_ce;
zend_class_entry *php_event_base_ce;
zend_class_entry *php_event_config_ce;
zend_class_entry *php_event_bevent_ce;
zend_class_entry *php_event_buffer_ce;
zend_class_entry *php_event_util_ce;
#ifdef HAVE_EVENT_OPENSSL_LIB
zend_class_entry *php_event_ssl_context_ce;
#endif

#ifdef HAVE_EVENT_EXTRA_LIB
zend_class_entry *php_event_dns_base_ce;
zend_class_entry *php_event_listener_ce;
zend_class_entry *php_event_http_conn_ce;
zend_class_entry *php_event_http_ce;
zend_class_entry *php_event_http_req_ce;
#endif

static zend_class_entry *spl_ce_RuntimeException;
zend_class_entry *php_event_exception_ce;

static HashTable classes;

static HashTable event_properties;
static HashTable event_bevent_properties;
static HashTable event_buffer_properties;
static HashTable event_listener_properties;
#ifdef HAVE_EVENT_OPENSSL_LIB
static HashTable event_ssl_context_properties;
int php_event_ssl_data_index;
#endif


static zend_object_handlers object_handlers;

static const zend_module_dep event_deps[] = {
#ifdef PHP_EVENT_SOCKETS_SUPPORT
	ZEND_MOD_REQUIRED("sockets")
#endif
	{NULL, NULL, NULL}
};

/* {{{ event_module_entry */
zend_module_entry event_module_entry = {
#if ZEND_MODULE_API_NO >= 20050922
	STANDARD_MODULE_HEADER_EX,
	NULL,
	event_deps,
#elif ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"event",
	NULL, /*event_functions*/
	PHP_MINIT(event),
	PHP_MSHUTDOWN(event),
	NULL,
	NULL,
	PHP_MINFO(event),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_EVENT_VERSION,
#endif
#if 0
	PHP_MODULE_GLOBALS(event),
	PHP_GINIT(event),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_EVENT
ZEND_GET_MODULE(event)
#endif

zend_class_entry *php_event_get_exception(void)/*{{{*/
{
	return php_event_exception_ce;
}/*}}}*/


zend_class_entry *php_event_get_exception_base(int root TSRMLS_DC)/*{{{*/
{
#if can_handle_soft_dependency_on_SPL && defined(HAVE_SPL) && ((PHP_MAJOR_VERSION > 5) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1))
	if (!root) {
		if (!spl_ce_RuntimeException) {
			zend_class_entry **pce;

			if (zend_hash_find(CG(class_table), "runtimeexception", sizeof("RuntimeException"), (void **) &pce) == SUCCESS) {
				spl_ce_RuntimeException = *pce;
				return *pce;
			}
		} else {
			return spl_ce_RuntimeException;
		}
	}
#endif
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 2)
	return zend_exception_get_default();
#else
	return zend_exception_get_default(TSRMLS_C);
#endif
}/*}}}*/


/* {{{ Private functions */

static void event_http_conn_object_dtor(void *object, zend_object_handle handle TSRMLS_DC)/*{{{*/
{
	php_event_http_conn_t *evcon = (php_event_http_conn_t *) object;

	PHP_EVENT_ASSERT(evcon);

	if (evcon->self) {
		zval_ptr_dtor(&evcon->self);
		evcon->self = NULL;
	}

	if (evcon->data_closecb) {
		zval_ptr_dtor(&evcon->data_closecb);
		evcon->data_closecb = NULL;
	}

	if (evcon->base) {
		zval_ptr_dtor(&evcon->base);
		evcon->base = NULL;
	}

	if (evcon->dns_base) {
		zval_ptr_dtor(&evcon->dns_base);
		evcon->dns_base = NULL;
	}

	zend_objects_destroy_object(object, handle TSRMLS_CC);
}/*}}}*/

static void event_bevent_object_dtor(void *object, zend_object_handle handle TSRMLS_DC)/*{{{*/
{
	php_event_bevent_t *b = (php_event_bevent_t *) object;

	if (b) {
		if (b->data) {
			zval_ptr_dtor(&b->data);
			b->data = NULL;
		}

		PHP_EVENT_FREE_FCALL_INFO(b->fci_read,  b->fcc_read);
		PHP_EVENT_FREE_FCALL_INFO(b->fci_write, b->fcc_write);
		PHP_EVENT_FREE_FCALL_INFO(b->fci_event, b->fcc_event);

		/* XXX */
		if (b->self) {
			zval_ptr_dtor(&b->self);
			b->self = NULL;
		}

		if (b->base) {
			zval_ptr_dtor(&b->base);
			b->base = NULL;
		}

		if (b->input) {
			zval_ptr_dtor(&b->input);
			b->input = NULL;
		}

		if (b->output) {
			zval_ptr_dtor(&b->output);
			b->output= NULL;
		}
	}

	zend_objects_destroy_object(object, handle TSRMLS_CC);
}/*}}}*/

/* {{{ event_generic_object_free_storage */
static zend_always_inline void event_generic_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_abstract_object_t *obj;

	PHP_EVENT_ASSERT(ptr);

	obj = (php_event_abstract_object_t *) ptr;

	zend_object_std_dtor(&obj->zo TSRMLS_CC);

	efree(ptr);
}
/* }}} */

/* {{{ event_object_free_storage */
static void event_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_t *e = (php_event_t *) ptr;

	PHP_EVENT_ASSERT(e);

	if (e->event) {
		/* No need in
		 * event_del(e->event);
		 * since event_free makes event non-pending internally */
		event_free(e->event);
		e->event = NULL;
	}

	if (e->stream_id >= 0) { /* stdin fd == 0 */
		zend_list_delete(e->stream_id);
		e->stream_id = -1;
	}

	if (e->data) {
		zval_ptr_dtor(&e->data);
		e->data = NULL;
	}

	PHP_EVENT_FREE_FCALL_INFO(e->fci, e->fcc);

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_base_object_free_storage */
static void event_base_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_base_t *b = (php_event_base_t *) ptr;

	PHP_EVENT_ASSERT(b);

	if (!b->internal && b->base) {
		event_base_loopexit(b->base, NULL);
		event_base_free(b->base);
		b->base = NULL;
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_config_object_free_storage*/
static void event_config_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_config_t *cfg = (php_event_config_t *) ptr;

	PHP_EVENT_ASSERT(cfg);

	if (cfg->ptr) {
		event_config_free(cfg->ptr);
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_bevent_object_free_storage */
static void event_bevent_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_bevent_t *b = (php_event_bevent_t *) ptr;

	if (b) {
#if 0
		if (b->data) {
			zval_ptr_dtor(&b->data);
			b->data = NULL;
		}


		/* XXX */
		if (b->self) {
			zval_ptr_dtor(&b->self);
			b->self = NULL;
		}

		if (b->base) {
			zval_ptr_dtor(&b->base);
			b->base = NULL;
		}

		if (b->input) {
			zval_ptr_dtor(&b->input);
			b->input = NULL;
		}

		if (b->output) {
			zval_ptr_dtor(&b->output);
			b->output= NULL;
		}
#endif

		if (b->bevent) {
			bufferevent_free(b->bevent);
			b->bevent = NULL;
		}

	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_buffer_object_free_storage */
static void event_buffer_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_buffer_t *b = (php_event_buffer_t *) ptr;

	PHP_EVENT_ASSERT(b);

	/* If we got the buffer in, say, a read callback the buffer
	 * is destroyed when the callback is done as any normal variable.
	 * Zend MM calls destructor which eventually calls this function.
	 * We'll definitely crash, if we call evbuffer_free() on an internal
	 * bufferevent buffer. */

	if (!b->internal && b->buf) {
		evbuffer_free(b->buf);
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

#ifdef HAVE_EVENT_EXTRA_LIB

/* {{{ event_dns_base_object_free_storage */
static void event_dns_base_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_dns_base_t *dnsb = (php_event_dns_base_t *) ptr;

	PHP_EVENT_ASSERT(dnsb);

	if (dnsb->dns_base) {
		/* Setting fail_requests to 1 makes all in-flight requests get
		 * their callbacks invoked with a canceled error code before it
		 * frees the base*/
		evdns_base_free(dnsb->dns_base, 1);
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_listener_object_free_storage */
static void event_listener_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_listener_t *l = (php_event_listener_t *) ptr;

	PHP_EVENT_ASSERT(l);

	if (l->data) {
		zval_ptr_dtor(&l->data);
		l->data = NULL;
	}

	if (l->self) {
		zval_ptr_dtor(&l->self);
		l->self = NULL;
	}

	PHP_EVENT_FREE_FCALL_INFO(l->fci, l->fcc);
	PHP_EVENT_FREE_FCALL_INFO(l->fci_err, l->fcc_err);

	if (l->listener) {
		evconnlistener_free(l->listener);
		l->listener = NULL;
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_conn_object_free_storage */
static void event_http_conn_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_http_conn_t *evcon = (php_event_http_conn_t *) ptr;

	PHP_EVENT_ASSERT(evcon);

	PHP_EVENT_FREE_FCALL_INFO(evcon->fci_closecb, evcon->fcc_closecb);

#if 0
	if (Z_REFCOUNT_P(evcon->self) > 1) {
		zval_ptr_dtor(&evcon->self);
		evcon->self = NULL;
	}

	if (evcon->data_closecb) {
		zval_ptr_dtor(&evcon->data_closecb);
		evcon->data_closecb = NULL;
	}

	if (evcon->base) {
		zval_ptr_dtor(&evcon->base);
		evcon->base = NULL;
	}

	if (evcon->dns_base) {
		zval_ptr_dtor(&evcon->dns_base);
		evcon->dns_base = NULL;
	}
#endif

	if (!evcon->internal && evcon->conn) {
		evhttp_connection_free(evcon->conn);
		evcon->conn = NULL;
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_object_free_storage */
static void event_http_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_http_t *http = (php_event_http_t *) ptr;
	php_event_http_cb_t *cb, *cb_next;

	PHP_EVENT_ASSERT(http);

	PHP_EVENT_FREE_FCALL_INFO(http->fci, http->fcc);

	/* Free attached callbacks */
	/*PHP_EVENT_ASSERT(http->cb_head);*/
	cb = http->cb_head;
	while (cb) {
		cb_next = cb->next;
		_php_event_free_http_cb(cb);
		cb = cb_next;
	}

	if (http->data) {
		zval_ptr_dtor(&http->data);
		http->data = NULL;
	}

	if (http->base) {
		zval_ptr_dtor(&http->base);
		http->base = NULL;
	}

	if (http->ptr) {
		evhttp_free(http->ptr);
		http->ptr = NULL;
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_req_object_free_storage */
static void event_http_req_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_http_req_t *http_req = (php_event_http_req_t *) ptr;

	PHP_EVENT_ASSERT(http_req);

	PHP_EVENT_FREE_FCALL_INFO(http_req->fci, http_req->fcc);

	if (http_req->self) {
		zval_ptr_dtor(&http_req->self);
		http_req->self = NULL;
	}
	if (http_req->data) {
		zval_ptr_dtor(&http_req->data);
		http_req->data = NULL;
	}

#if 0
	/*
	Libevent cleans up http_req->ptr despite the ownership of the pointer
	(evhttp_request_own()). So we'll get SEGFAULT here if we call
	evhttp_request_free().
	*/

	if (!http_req->internal && http_req->ptr) {
		evhttp_request_free(http_req->ptr);
		http_req->ptr = NULL;
	}
#endif

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

#endif /* HAVE_EVENT_EXTRA_LIB */


#ifdef HAVE_EVENT_OPENSSL_LIB
/* {{{ event_ssl_context_object_free_storage */
static void event_ssl_context_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_ssl_context_t *ectx = (php_event_ssl_context_t *) ptr;

	if (ectx->ctx) {
		SSL_CTX_free(ectx->ctx);
		ectx->ctx = NULL;
	}

	if (ectx->ht) {
		zend_hash_destroy(ectx->ht);
		FREE_HASHTABLE(ectx->ht);
		ectx->ht = NULL;
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */
#endif


/* {{{ register_object */
static zend_always_inline zend_object_value register_object(zend_class_entry *ce, void *obj, zend_objects_store_dtor_t func_dtor, zend_objects_free_object_storage_t func_free_storage TSRMLS_DC)
{
	zend_object_value retval;

	retval.handle   = zend_objects_store_put(obj, func_dtor, func_free_storage, NULL TSRMLS_CC);
	retval.handlers = &object_handlers;

	return retval;
}
/* }}} */

/* {{{ object_new
 * Allocates new object with it's properties.
 * size is a size of struct implementing php_event_abstract_object_t */
static void *object_new(zend_class_entry *ce, size_t size TSRMLS_DC)
{
	php_event_abstract_object_t *obj;
	zend_class_entry *ce_parent = ce;

	obj = ecalloc(1, size);

	while (ce_parent->type != ZEND_INTERNAL_CLASS && ce_parent->parent != NULL) {
		ce_parent = ce_parent->parent;
	}
	zend_hash_find(&classes, ce_parent->name, ce_parent->name_length + 1,
			(void **) &obj->prop_handler);

	zend_object_std_init(&obj->zo, ce TSRMLS_CC);
	object_properties_init(&obj->zo, ce);

	return (void *) obj;
}
/* }}} */


/* {{{ event_object_create
 * Event object ctor */
static zend_object_value event_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, (zend_objects_store_dtor_t) zend_objects_destroy_object,
			event_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_base_object_create
 * EventBase object ctor */
static zend_object_value event_base_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_base_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, (zend_objects_store_dtor_t) zend_objects_destroy_object,
			event_base_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_config_object_create
 * EventConfig object ctor */
static zend_object_value event_config_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_config_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, (zend_objects_store_dtor_t) zend_objects_destroy_object,
			event_config_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_bevent_object_create
 * EventBufferEvent object ctor */
static zend_object_value event_bevent_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_bevent_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, (zend_objects_store_dtor_t) event_bevent_object_dtor,
			event_bevent_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_buffer_object_create
 * EventBuffer object ctor */
static zend_object_value event_buffer_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_buffer_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, (zend_objects_store_dtor_t) zend_objects_destroy_object,
			event_buffer_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_util_object_create
 * EventUtil object ctor */
static zend_object_value event_util_object_create(zend_class_entry *ce TSRMLS_DC)
{
#if 0
	php_event_abstract_object_t *obj;

	/* EventUtil is a singleton. This function must never be called */
	PHP_EVENT_ASSERT(0);

	obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_abstract_object_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, (zend_objects_store_dtor_t) zend_objects_destroy_object,
			event_generic_object_free_storage TSRMLS_CC);
#endif

	zend_object *object;
	zend_object_value value;

	value = zend_objects_new(&object, ce TSRMLS_CC);
	value.handlers = zend_get_std_object_handlers();

	object_properties_init(object, ce);

	return value;
}
/* }}} */

#ifdef HAVE_EVENT_OPENSSL_LIB
/* {{{ event_ssl_context_object_create
 * EventSslContext object ctor */
static zend_object_value event_ssl_context_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *)
		object_new(ce, sizeof(php_event_ssl_context_t) TSRMLS_CC);

	return register_object(ce, (void *) obj,
			(zend_objects_store_dtor_t) zend_objects_destroy_object,
			event_ssl_context_object_free_storage TSRMLS_CC);
}
/* }}} */
#endif

#if HAVE_EVENT_EXTRA_LIB

/* {{{ event_dns_base_object_create
 * EventDnsBase object ctor */
static zend_object_value event_dns_base_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_dns_base_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, (zend_objects_store_dtor_t) zend_objects_destroy_object,
			event_dns_base_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_listener_object_create
 * EventListener object ctor */
static zend_object_value event_listener_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_listener_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, (zend_objects_store_dtor_t) zend_objects_destroy_object,
			event_listener_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_conn_object_create
 * EventHttpConnection object ctor */
static zend_object_value event_http_conn_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_http_conn_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, (zend_objects_store_dtor_t) event_http_conn_object_dtor,
			event_http_conn_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_object_create
 * EventHttp object ctor */
static zend_object_value event_http_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_http_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, (zend_objects_store_dtor_t) zend_objects_destroy_object,
			event_http_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_req_object_create
 * EventHttpRequest object ctor */
static zend_object_value event_http_req_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_http_req_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, (zend_objects_store_dtor_t) zend_objects_destroy_object,
			event_http_req_object_free_storage TSRMLS_CC);
}
/* }}} */

#endif /* HAVE_EVENT_EXTRA_LIB */


/* {{{ fatal_error_cb
 * Is called when Libevent detects a non-recoverable internal error. */
static void fatal_error_cb(int err)
{
	TSRMLS_FETCH();

	php_error_docref(NULL TSRMLS_CC, E_ERROR,
			"libevent detected a non-recoverable internal error, code: %d", err);
}
/* }}} */


#if LIBEVENT_VERSION_NUMBER < 0x02001900
# define PHP_EVENT_LOG_CONST(name) _ ## name
#else
# define PHP_EVENT_LOG_CONST(name) name
#endif

/* {{{ log_cb
 * Overrides libevent's default error logging(it logs to stderr) */
static void log_cb(int severity, const char *msg)
{
	int error_type;

	/* TSRMLS_FETCH consumes a fair amount of resources.  But a ready-to-use
	 * program shouldn't get any error logs. Nevertheless, we have no other way
	 * to fetch TSRMLS. */
	TSRMLS_FETCH();

	switch (severity) {
		case PHP_EVENT_LOG_CONST(EVENT_LOG_DEBUG):
			error_type = E_STRICT;
		case PHP_EVENT_LOG_CONST(EVENT_LOG_MSG):
			error_type = E_NOTICE;
		case PHP_EVENT_LOG_CONST(EVENT_LOG_WARN):
			error_type = E_WARNING;
		case PHP_EVENT_LOG_CONST(EVENT_LOG_ERR):
			error_type = E_ERROR;
		default:
			error_type = E_NOTICE;
	}

	php_error_docref(NULL TSRMLS_CC, error_type, "%s", msg);
}
/* }}} */


/* {{{ read_property_default */
static int read_property_default(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	*retval = NULL;
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot read property");
	return FAILURE;
}
/* }}} */

/* {{{ write_property_default */
static int write_property_default(php_event_abstract_object_t *obj, zval *newval TSRMLS_DC)
{
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot write property");
	return FAILURE;
}
/* }}} */

/* {{{ add_property */
static void add_property(HashTable *h, const char *name, size_t name_len, php_event_prop_read_t read_func, php_event_prop_write_t write_func, php_event_prop_get_prop_ptr_ptr_t get_ptr_ptr_func TSRMLS_DC) {
	php_event_prop_handler_t p;

	p.name             = (char *) name;
	p.name_len         = name_len;
	p.read_func        = (read_func) ? read_func : read_property_default;
	p.write_func       = (write_func) ? write_func: write_property_default;
	p.get_ptr_ptr_func = get_ptr_ptr_func;
	zend_hash_add(h, name, name_len + 1, &p, sizeof(php_event_prop_handler_t), NULL);
}
/* }}} */

/* {{{ read_property */
static zval *read_property(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC)
{
	zval                         tmp_member;
	zval                        *retval;
	php_event_abstract_object_t *obj;
	php_event_prop_handler_t    *hnd;
	int                          ret;

	ret = FAILURE;
	obj = (php_event_abstract_object_t *) zend_objects_get_address(object TSRMLS_CC);

	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	if (obj->prop_handler != NULL) {
		ret = zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
	}

	if (ret == SUCCESS) {
		ret = hnd->read_func(obj, &retval TSRMLS_CC);
		if (ret == SUCCESS) {
			/* ensure we're creating a temporary variable */
			Z_SET_REFCOUNT_P(retval, 0);
		} else {
			retval = EG(uninitialized_zval_ptr);
		}
	} else {
		zend_object_handlers * std_hnd = zend_get_std_object_handlers();
		retval = std_hnd->read_property(object, member, type, key TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}

	return(retval);
}
/* }}} */

/* {{{ write_property */
static void write_property(zval *object, zval *member, zval *value, const zend_literal *key TSRMLS_DC)
{
	zval                         tmp_member;
	php_event_abstract_object_t *obj;
	php_event_prop_handler_t    *hnd;
	int                          ret;

	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	ret = FAILURE;
	obj = (php_event_abstract_object_t *) zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
		ret = zend_hash_find((HashTable *) obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
	}
	if (ret == SUCCESS) {
		hnd->write_func(obj, value TSRMLS_CC);
	} else {
		zend_object_handlers * std_hnd = zend_get_std_object_handlers();
		std_hnd->write_property(object, member, value, key TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}
}
/* }}} */

/* {{{ object_has_property */
static int object_has_property(zval *object, zval *member, int has_set_exists, const zend_literal *key TSRMLS_DC)
{
	php_event_abstract_object_t *obj;
	int                          ret = 0;
	php_event_prop_handler_t    p;

	obj = (php_event_abstract_object_t *) zend_objects_get_address(object TSRMLS_CC);


	if (obj->prop_handler) {
		if (zend_hash_find(obj->prop_handler, Z_STRVAL_P(member),
					Z_STRLEN_P(member) + 1, (void **) &p) == SUCCESS) {
			switch (has_set_exists) {
				case 2:
					ret = 1;
					break;
				case 1: {
							zval *value = read_property(object, member, BP_VAR_IS, key TSRMLS_CC);
							if (value != EG(uninitialized_zval_ptr)) {
								convert_to_boolean(value);
								ret = Z_BVAL_P(value)? 1:0;
								/* refcount is 0 */
								Z_ADDREF_P(value);
								zval_ptr_dtor(&value);
							}
							break;
						}
				case 0:{
						   zval *value = read_property(object, member, BP_VAR_IS, key TSRMLS_CC);
						   if (value != EG(uninitialized_zval_ptr)) {
							   ret = Z_TYPE_P(value) != IS_NULL? 1:0;
							   /* refcount is 0 */
							   Z_ADDREF_P(value);
							   zval_ptr_dtor(&value);
						   }
						   break;
					   }
				default:
					   php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid value for has_set_exists");
			}
		} else {
			zend_object_handlers *std_hnd = zend_get_std_object_handlers();
			ret = std_hnd->has_property(object, member, has_set_exists, key TSRMLS_CC);
		}
	}
	return ret;
}
/* }}} */

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
/* {{{ object_get_debug_info */
static HashTable *object_get_debug_info(zval *object, int *is_temp TSRMLS_DC)
{
	php_event_abstract_object_t *obj;
	HashTable                   *retval;
	HashTable                   *props;
	HashPosition                 pos;
	php_event_prop_handler_t *entry;

	obj   = (php_event_abstract_object_t *) zend_objects_get_address(object TSRMLS_CC);
	props = obj->prop_handler;

	ALLOC_HASHTABLE(retval);

	if (!props) {
		ZEND_INIT_SYMTABLE_EX(retval, 1, 0);
		return retval;
	}

	ZEND_INIT_SYMTABLE_EX(retval, zend_hash_num_elements(props) + 1, 0);

	zend_hash_internal_pointer_reset_ex(props, &pos);
	while (zend_hash_get_current_data_ex(props, (void **) &entry, &pos) == SUCCESS) {
		zval member;
		zval *value;

		INIT_ZVAL(member);
		ZVAL_STRINGL(&member, entry->name, entry->name_len, 0);

		value = read_property(object, &member, BP_VAR_IS, 0 TSRMLS_CC);
		if (value != EG(uninitialized_zval_ptr)) {
			Z_ADDREF_P(value);
			zend_hash_add(retval, entry->name, entry->name_len + 1, &value, sizeof(zval *) , NULL);
		}

		zend_hash_move_forward_ex(props, &pos);
	}

	*is_temp = 1;

	return retval;
}
/* }}} */
#endif

/* {{{ get_property_ptr_ptr */
#if PHP_VERSION_ID >= 50500
static zval **get_property_ptr_ptr(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC)
#else
static zval **get_property_ptr_ptr(zval *object, zval *member, const zend_literal *key TSRMLS_DC)
#endif
{
	php_event_abstract_object_t  *obj;
	zval                          tmp_member;
	zval                        **retval     = NULL;
	php_event_prop_handler_t     *hnd;
	int                           ret        = FAILURE;

	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	obj = (php_event_abstract_object_t *) zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
		ret = zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member) + 1, (void **) &hnd);
	}

	if (ret == FAILURE) {
#if PHP_VERSION_ID >= 50500
		retval = zend_get_std_object_handlers()->get_property_ptr_ptr(object, member, type, key TSRMLS_CC);
#else
		retval = zend_get_std_object_handlers()->get_property_ptr_ptr(object, member, key TSRMLS_CC);
#endif
	} else if (hnd->get_ptr_ptr_func) {
		retval = hnd->get_ptr_ptr_func(obj TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}

	return retval;
}
/* }}} */


#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 4
/* {{{ get_properties
   Returns all object properties. */
static HashTable *get_properties(zval *object TSRMLS_DC)
{
	php_event_abstract_object_t *obj;
	php_event_prop_handler_t    *hnd;
	HashTable                   *props;
	zval                        *val;
	char                        *key;
	uint                         key_len;
	HashPosition                 pos;
	ulong                        num_key;

	obj = (php_event_abstract_object_t *) zend_objects_get_address(object TSRMLS_CC);
	/* Don't get obj->zo.properties; directly!
	 * Otherwise serialization functions will cause SEGFAULTs */
	props = zend_std_get_properties(object TSRMLS_CC);

	if (obj->prop_handler) {
		zend_hash_internal_pointer_reset_ex(obj->prop_handler, &pos);

		while (zend_hash_get_current_data_ex(obj->prop_handler,
					(void **) &hnd, &pos) == SUCCESS) {
			zend_hash_get_current_key_ex(obj->prop_handler,
					&key, &key_len, &num_key, 0, &pos);
			if (!hnd->read_func || hnd->read_func(obj, &val TSRMLS_CC) != SUCCESS) {
				val = EG(uninitialized_zval_ptr);
				Z_ADDREF_P(val);
			}
			zend_hash_update(props, key, key_len, (void *) &val, sizeof(zval *), NULL);
			zend_hash_move_forward_ex(obj->prop_handler, &pos);
		}
	}

	return obj->zo.properties;
}
/* }}} */
#endif

static HashTable *get_gc(zval *object, zval ***table, int *n TSRMLS_DC)
{
	*table = NULL;
	*n = 0;
	return zend_std_get_properties(object TSRMLS_CC);
}


#define PHP_EVENT_ADD_CLASS_PROPERTIES(a, b)                                           \
{                                                                                      \
	int i = 0;                                                                         \
	while (b[i].name != NULL) {                                                        \
		add_property((a), (b)[i].name, (b)[i].name_length,                             \
				(php_event_prop_read_t)(b)[i].read_func,                               \
				(php_event_prop_write_t)(b)[i].write_func,                             \
				(php_event_prop_get_prop_ptr_ptr_t)(b)[i].get_ptr_ptr_func TSRMLS_CC); \
		i++;                                                                           \
	}                                                                                  \
}

#define PHP_EVENT_DECL_CLASS_PROPERTIES(a, b)                            \
{                                                                        \
	int i = 0;                                                           \
	while (b[i].name != NULL) {                                          \
		zend_declare_property_null((a), (b)[i].name, (b)[i].name_length, \
				ZEND_ACC_PUBLIC TSRMLS_CC);                              \
		i++;                                                             \
	}                                                                    \
}

/* {{{ register_classes */
static zend_always_inline void register_classes(TSRMLS_D)
{
	zend_class_entry *ce;
	zend_class_entry ce_exception;

	PHP_EVENT_REGISTER_CLASS("Event", event_object_create, php_event_ce, php_event_ce_functions);
	ce = php_event_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;
	zend_hash_init(&event_properties, 2, NULL, NULL, 1);
	PHP_EVENT_ADD_CLASS_PROPERTIES(&event_properties, event_property_entries);
	PHP_EVENT_DECL_CLASS_PROPERTIES(ce, event_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &event_properties,
			sizeof(event_properties), NULL);

	PHP_EVENT_REGISTER_CLASS("EventBase", event_base_object_create, php_event_base_ce,
			php_event_base_ce_functions);
	ce = php_event_base_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	PHP_EVENT_REGISTER_CLASS("EventConfig", event_config_object_create, php_event_config_ce,
			php_event_config_ce_functions);
	ce = php_event_config_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	PHP_EVENT_REGISTER_CLASS("EventBufferEvent", event_bevent_object_create, php_event_bevent_ce,
			php_event_bevent_ce_functions);
	ce = php_event_bevent_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;
	zend_hash_init(&event_bevent_properties, 4, NULL, NULL, 1);
	PHP_EVENT_ADD_CLASS_PROPERTIES(&event_bevent_properties, event_bevent_property_entries);
	PHP_EVENT_DECL_CLASS_PROPERTIES(ce, event_bevent_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &event_bevent_properties,
			sizeof(event_bevent_properties), NULL);

	PHP_EVENT_REGISTER_CLASS("EventBuffer", event_buffer_object_create, php_event_buffer_ce,
			php_event_buffer_ce_functions);
	ce = php_event_buffer_ce;
	/*ce->ce_flags |= ZEND_ACC_FINAL_CLASS;*/
	zend_hash_init(&event_buffer_properties, 2, NULL, NULL, 1);
	PHP_EVENT_ADD_CLASS_PROPERTIES(&event_buffer_properties, event_buffer_property_entries);
	PHP_EVENT_DECL_CLASS_PROPERTIES(ce, event_buffer_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &event_buffer_properties,
			sizeof(event_buffer_properties), NULL);

#if HAVE_EVENT_EXTRA_LIB
	PHP_EVENT_REGISTER_CLASS("EventDnsBase", event_dns_base_object_create, php_event_dns_base_ce,
			php_event_dns_base_ce_functions);
	ce = php_event_dns_base_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	PHP_EVENT_REGISTER_CLASS("EventListener", event_listener_object_create, php_event_listener_ce,
			php_event_listener_ce_functions);
	ce = php_event_listener_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;
	zend_hash_init(&event_listener_properties, 1, NULL, NULL, 1);
	PHP_EVENT_ADD_CLASS_PROPERTIES(&event_listener_properties, event_listener_property_entries);
	PHP_EVENT_DECL_CLASS_PROPERTIES(ce, event_listener_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &event_listener_properties,
			sizeof(event_listener_properties), NULL);

	PHP_EVENT_REGISTER_CLASS("EventHttpConnection", event_http_conn_object_create,
			php_event_http_conn_ce,
			php_event_http_conn_ce_functions);
	ce = php_event_http_conn_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	PHP_EVENT_REGISTER_CLASS("EventHttp", event_http_object_create, php_event_http_ce,
			php_event_http_ce_functions);
	ce = php_event_http_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	PHP_EVENT_REGISTER_CLASS("EventHttpRequest", event_http_req_object_create, php_event_http_req_ce,
			php_event_http_req_ce_functions);
	ce = php_event_http_req_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

#endif /* HAVE_EVENT_EXTRA_LIB */

	PHP_EVENT_REGISTER_CLASS("EventUtil", event_util_object_create, php_event_util_ce,
			php_event_util_ce_functions);
	ce = php_event_util_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

#ifdef HAVE_EVENT_OPENSSL_LIB
	PHP_EVENT_REGISTER_CLASS("EventSslContext", event_ssl_context_object_create,
			php_event_ssl_context_ce,
			php_event_ssl_context_ce_functions);
	ce = php_event_ssl_context_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;
	zend_hash_init(&event_ssl_context_properties, 0, NULL, NULL, 1);
	PHP_EVENT_ADD_CLASS_PROPERTIES(&event_ssl_context_properties, event_ssl_context_property_entries);
	PHP_EVENT_DECL_CLASS_PROPERTIES(ce, event_ssl_context_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &event_ssl_context_properties,
			sizeof(event_ssl_context_properties), NULL);
#endif /* HAVE_EVENT_OPENSSL_LIB */

#ifdef PHP_EVENT_NS
	INIT_NS_CLASS_ENTRY(ce_exception, PHP_EVENT_NS, "EventException", NULL);
#else
	INIT_CLASS_ENTRY(ce_exception, "EventException", NULL);
#endif
	php_event_exception_ce = zend_register_internal_class_ex(&ce_exception, php_event_get_exception_base(0 TSRMLS_CC), NULL TSRMLS_CC);
	zend_declare_property_null(php_event_exception_ce, "errorInfo", sizeof("errorInfo") - 1, ZEND_ACC_PUBLIC TSRMLS_CC);
}
/* }}} */

/* Private functions }}} */

#define REGISTER_EVENT_CLASS_CONST_LONG(pce, const_name, value) \
	zend_declare_class_constant_long((pce), #const_name,        \
			sizeof(#const_name) - 1, (long) value TSRMLS_CC)

#define REGISTER_EVENT_CLASS_CONST_STRING(pce, const_name, value) \
    zend_declare_class_constant_stringl((pce), #const_name, \
            sizeof(#const_name) - 1, value, sizeof(value) - 1 TSRMLS_CC)

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(event)
{
	zend_object_handlers *std_hnd = zend_get_std_object_handlers();

	memcpy(&object_handlers, std_hnd, sizeof(zend_object_handlers));

	object_handlers.clone_obj            = NULL;
	object_handlers.read_property        = read_property;
	object_handlers.write_property       = write_property;
	object_handlers.get_property_ptr_ptr = get_property_ptr_ptr;
	object_handlers.has_property         = object_has_property;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	object_handlers.get_debug_info       = object_get_debug_info;
#endif
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 4
	object_handlers.get_properties       = get_properties;
#endif
	object_handlers.get_gc               = get_gc;

	spl_ce_RuntimeException = NULL;

#ifdef PHP_EVENT_NS
	REGISTER_STRING_CONSTANT("EVENT_NS", PHP_EVENT_NS, CONST_CS | CONST_PERSISTENT);
#else
	REGISTER_STRING_CONSTANT("EVENT_NS", "", CONST_CS | CONST_PERSISTENT);
#endif

	zend_hash_init(&classes, 8, NULL, NULL, 1);
	register_classes(TSRMLS_C);

	/* Loop flags */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, LOOP_ONCE,     EVLOOP_ONCE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, LOOP_NONBLOCK, EVLOOP_NONBLOCK);

	/* Run-time flags of event base usually passed to event_config_set_flag */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, NOLOCK,               EVENT_BASE_FLAG_NOLOCK);
#if LIBEVENT_VERSION_NUMBER >= 0x02000301
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, STARTUP_IOCP,         EVENT_BASE_FLAG_STARTUP_IOCP);
#endif
#if LIBEVENT_VERSION_NUMBER >= 0x02000901
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, NO_CACHE_TIME,        EVENT_BASE_FLAG_NO_CACHE_TIME);
#endif
#if LIBEVENT_VERSION_NUMBER >= 0x02000301
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, EPOLL_USE_CHANGELIST, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, IGNORE_ENV,           EVENT_BASE_FLAG_IGNORE_ENV);
#endif
#if LIBEVENT_VERSION_NUMBER >= 0x02010201
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, PRECISE_TIMER,        EVENT_BASE_FLAG_PRECISE_TIMER);
#endif

	/* Event flags */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ce, ET,      EV_ET);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ce, PERSIST, EV_PERSIST);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ce, READ,    EV_READ);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ce, WRITE,   EV_WRITE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ce, SIGNAL,  EV_SIGNAL);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ce, TIMEOUT, EV_TIMEOUT);

	/* XXX define on the fly by calling event_base_get_features() first */
	/* Features of event_base usually passed to event_config_require_features */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_config_ce, FEATURE_ET,  EV_FEATURE_ET);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_config_ce, FEATURE_O1,  EV_FEATURE_O1);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_config_ce, FEATURE_FDS, EV_FEATURE_FDS);

	/* Buffer event flags */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, READING,   BEV_EVENT_READING);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, WRITING,   BEV_EVENT_WRITING);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, EOF,       BEV_EVENT_EOF);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, ERROR,     BEV_EVENT_ERROR);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, TIMEOUT,   BEV_EVENT_TIMEOUT);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, CONNECTED, BEV_EVENT_CONNECTED);

	/* Option flags for bufferevents */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, OPT_CLOSE_ON_FREE,    BEV_OPT_CLOSE_ON_FREE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, OPT_THREADSAFE,       BEV_OPT_THREADSAFE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, OPT_DEFER_CALLBACKS,  BEV_OPT_DEFER_CALLBACKS);
#if LIBEVENT_VERSION_NUMBER >= 0x02000500
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, OPT_UNLOCK_CALLBACKS, BEV_OPT_UNLOCK_CALLBACKS);
#endif
#ifdef HAVE_EVENT_OPENSSL_LIB
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, SSL_OPEN,       BUFFEREVENT_SSL_OPEN);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, SSL_CONNECTING, BUFFEREVENT_SSL_CONNECTING);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, SSL_ACCEPTING,  BUFFEREVENT_SSL_ACCEPTING);
#endif

	/* Address families */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, AF_INET,   AF_INET);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, AF_INET6,  AF_INET6);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, AF_UNIX,   AF_UNIX);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, AF_UNSPEC, AF_UNSPEC);

	/* Socket options */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_DEBUG,     SO_DEBUG);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_REUSEADDR, SO_REUSEADDR);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_KEEPALIVE, SO_KEEPALIVE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_DONTROUTE, SO_DONTROUTE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_LINGER,    SO_LINGER);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_BROADCAST, SO_BROADCAST);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_OOBINLINE, SO_OOBINLINE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_SNDBUF,    SO_SNDBUF);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_RCVBUF,    SO_RCVBUF);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_SNDLOWAT,  SO_SNDLOWAT);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_RCVLOWAT,  SO_RCVLOWAT);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_SNDTIMEO,  SO_SNDTIMEO);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_RCVTIMEO,  SO_RCVTIMEO);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_TYPE,      SO_TYPE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SO_ERROR,     SO_ERROR);
#ifdef TCP_NODELAY
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, TCP_NODELAY, TCP_NODELAY);
#endif

	/* Socket protocol levels */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SOL_SOCKET, SOL_SOCKET);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SOL_TCP,    IPPROTO_TCP);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SOL_UDP,    IPPROTO_UDP);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, IPPROTO_IP, IPPROTO_IP);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, SOCK_RAW,   SOCK_RAW);
#if HAVE_IPV6
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, IPPROTO_IPV6, IPPROTO_IPV6);
#endif




#ifdef HAVE_EVENT_EXTRA_LIB
	/* DNS options */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_dns_base_ce, OPTION_SEARCH,      DNS_OPTION_SEARCH);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_dns_base_ce, OPTION_NAMESERVERS, DNS_OPTION_NAMESERVERS);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_dns_base_ce, OPTION_MISC,        DNS_OPTION_MISC);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_dns_base_ce, OPTION_HOSTSFILE,   DNS_OPTION_HOSTSFILE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_dns_base_ce, OPTIONS_ALL,        DNS_OPTIONS_ALL);

	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_LEAVE_SOCKETS_BLOCKING, LEV_OPT_LEAVE_SOCKETS_BLOCKING);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_CLOSE_ON_FREE,          LEV_OPT_CLOSE_ON_FREE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_CLOSE_ON_EXEC,          LEV_OPT_CLOSE_ON_EXEC);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_REUSEABLE,              LEV_OPT_REUSEABLE);
# if LIBEVENT_VERSION_NUMBER >= 0x02010100
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_DISABLED,               LEV_OPT_DISABLED);
# endif
# if LIBEVENT_VERSION_NUMBER >= 0x02000800
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_THREADSAFE,             LEV_OPT_THREADSAFE);
#endif
# if LIBEVENT_VERSION_NUMBER >= 0x02010100
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_DEFERRED_ACCEPT,        LEV_OPT_DEFERRED_ACCEPT);
# endif

	/* EventHttpRequest command types */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_http_req_ce, CMD_GET,     EVHTTP_REQ_GET);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_http_req_ce, CMD_POST,    EVHTTP_REQ_POST);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_http_req_ce, CMD_HEAD,    EVHTTP_REQ_HEAD);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_http_req_ce, CMD_PUT,     EVHTTP_REQ_PUT);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_http_req_ce, CMD_DELETE,  EVHTTP_REQ_DELETE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_http_req_ce, CMD_OPTIONS, EVHTTP_REQ_OPTIONS);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_http_req_ce, CMD_TRACE,   EVHTTP_REQ_TRACE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_http_req_ce, CMD_CONNECT, EVHTTP_REQ_CONNECT);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_http_req_ce, CMD_PATCH,   EVHTTP_REQ_PATCH);

	/* EventHttpRequest header types */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_http_req_ce, INPUT_HEADER,  PHP_EVENT_REQ_HEADER_INPUT);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_http_req_ce, OUTPUT_HEADER, PHP_EVENT_REQ_HEADER_OUTPUT);

#endif /* HAVE_EVENT_EXTRA_LIB */

	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, LIBEVENT_VERSION_NUMBER, LIBEVENT_VERSION_NUMBER);

	REGISTER_EVENT_CLASS_CONST_LONG(php_event_buffer_ce, EOL_ANY,         EVBUFFER_EOL_ANY);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_buffer_ce, EOL_CRLF,        EVBUFFER_EOL_CRLF);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_buffer_ce, EOL_CRLF_STRICT, EVBUFFER_EOL_CRLF_STRICT);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_buffer_ce, EOL_LF,          EVBUFFER_EOL_LF);
#if LIBEVENT_VERSION_NUMBER >= 0x02010100
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_buffer_ce, EOL_NUL,         EVBUFFER_EOL_NUL);
#endif
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_buffer_ce, PTR_SET,         EVBUFFER_PTR_SET);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_buffer_ce, PTR_ADD,         EVBUFFER_PTR_ADD);

#ifdef HAVE_EVENT_OPENSSL_LIB
# ifdef HAVE_SSL2
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, SSLv2_CLIENT_METHOD,  PHP_EVENT_SSLv2_CLIENT_METHOD);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, SSLv23_CLIENT_METHOD, PHP_EVENT_SSLv23_CLIENT_METHOD);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, SSLv2_SERVER_METHOD,  PHP_EVENT_SSLv2_SERVER_METHOD);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, SSLv23_SERVER_METHOD, PHP_EVENT_SSLv23_SERVER_METHOD);
# endif
# ifdef HAVE_SSL3
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, SSLv3_CLIENT_METHOD,  PHP_EVENT_SSLv3_CLIENT_METHOD);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, SSLv3_SERVER_METHOD,  PHP_EVENT_SSLv3_SERVER_METHOD);
# endif

	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, TLS_CLIENT_METHOD,    PHP_EVENT_TLS_CLIENT_METHOD);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, TLS_SERVER_METHOD,    PHP_EVENT_TLS_SERVER_METHOD);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, TLSv11_CLIENT_METHOD, PHP_EVENT_TLSv11_CLIENT_METHOD);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, TLSv11_SERVER_METHOD, PHP_EVENT_TLSv11_SERVER_METHOD);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, TLSv12_CLIENT_METHOD, PHP_EVENT_TLSv12_CLIENT_METHOD);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, TLSv12_SERVER_METHOD, PHP_EVENT_TLSv12_SERVER_METHOD);

	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_LOCAL_CERT,               PHP_EVENT_OPT_LOCAL_CERT);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_LOCAL_PK,                 PHP_EVENT_OPT_LOCAL_PK);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_PASSPHRASE,               PHP_EVENT_OPT_PASSPHRASE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_CA_FILE,                  PHP_EVENT_OPT_CA_FILE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_CA_PATH,                  PHP_EVENT_OPT_CA_PATH);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_ALLOW_SELF_SIGNED,        PHP_EVENT_OPT_ALLOW_SELF_SIGNED);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_VERIFY_PEER,              PHP_EVENT_OPT_VERIFY_PEER);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_VERIFY_DEPTH,             PHP_EVENT_OPT_VERIFY_DEPTH);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_CIPHERS,                  PHP_EVENT_OPT_CIPHERS);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_NO_SSLv2,                 PHP_EVENT_OPT_NO_SSLv2);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_NO_SSLv3,                 PHP_EVENT_OPT_NO_SSLv3);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_NO_TLSv1,                 PHP_EVENT_OPT_NO_TLSv1);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_NO_TLSv1_1,               PHP_EVENT_OPT_NO_TLSv1_1);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_NO_TLSv1_2,               PHP_EVENT_OPT_NO_TLSv1_2);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_CIPHER_SERVER_PREFERENCE, PHP_EVENT_OPT_CIPHER_SERVER_PREFERENCE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_REQUIRE_CLIENT_CERT,      PHP_EVENT_OPT_REQUIRE_CLIENT_CERT);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, OPT_VERIFY_CLIENT_ONCE,       PHP_EVENT_OPT_VERIFY_CLIENT_ONCE);

	REGISTER_EVENT_CLASS_CONST_STRING(php_event_ssl_context_ce, OPENSSL_VERSION_TEXT,   OPENSSL_VERSION_TEXT);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce,   OPENSSL_VERSION_NUMBER, OPENSSL_VERSION_NUMBER);
#ifdef LIBRESSL_VERSION_NUMBER
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce,   LIBRESSL_VERSION_NUMBER,     LIBRESSL_VERSION_NUMBER);
#endif
#ifdef LIBRESSL_VERSION_TEXT
	REGISTER_EVENT_CLASS_CONST_STRING(php_event_ssl_context_ce, LIBRESSL_VERSION_TEXT,       LIBRESSL_VERSION_TEXT);
#endif

	/* Constants for EventSslContext::setMinProtoVersion */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, SSL3_VERSION,    SSL3_VERSION);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, TLS1_VERSION,    TLS1_VERSION);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, TLS1_1_VERSION,  TLS1_1_VERSION);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, TLS1_2_VERSION,  TLS1_2_VERSION);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, DTLS1_VERSION,   DTLS1_VERSION);
# ifdef DTLS1_2_VERSION /* May be missing in libressl*/
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ssl_context_ce, DTLS1_2_VERSION, DTLS1_2_VERSION);
# endif
#endif

	/* Initialize openssl library */
	SSL_library_init();
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	/* Create new index which will be used to retreive custom data of the OpenSSL callbacks */
	php_event_ssl_data_index = SSL_get_ex_new_index(0, "PHP EventSslContext index", NULL, NULL, NULL);
#endif /* HAVE_EVENT_OPENSSL_LIB */


#ifdef PHP_EVENT_DEBUG
	event_enable_debug_mode();
#endif

#ifdef HAVE_EVENT_PTHREADS_LIB
# ifdef WIN32
	evthread_use_windows_threads();
# else
	if (evthread_use_pthreads()) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"evthread_use_pthreads failed, submit a bug");
	}
# endif
#endif

	/* Handle libevent's error logging more gracefully than it's default
	 * logging to stderr, or calling abort()/exit() */
	event_set_fatal_callback(fatal_error_cb);
	event_set_log_callback(log_cb);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(event)
{
#ifdef HAVE_EVENT_OPENSSL_LIB
	/* Removes memory allocated when loading digest and cipher names
	 * in the OpenSSL_add_all_ family of functions */
	EVP_cleanup();
#endif

#if LIBEVENT_VERSION_NUMBER >= 0x02010000
	/* libevent_global_shutdown is available since libevent 2.1.0-alpha.
	 *
	 * Make sure that libevent has released all internal library-global data
	 * structures. Don't call any of libevent functions below! */
	libevent_global_shutdown();
#endif

	zend_hash_destroy(&event_properties);
	zend_hash_destroy(&event_bevent_properties);
	zend_hash_destroy(&event_buffer_properties);
	zend_hash_destroy(&event_listener_properties);
#ifdef HAVE_EVENT_OPENSSL_LIB
	zend_hash_destroy(&event_ssl_context_properties);
#endif

	zend_hash_destroy(&classes);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(event)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Event support", "enabled");
#ifdef PHP_EVENT_SOCKETS_SUPPORT
	php_info_print_table_header(2, "Sockets support", "enabled");
#else
	php_info_print_table_header(2, "Sockets support", "disabled");
#endif
#ifdef PHP_EVENT_DEBUG
	php_info_print_table_row(2, "Debug support", "enabled");
#else
	php_info_print_table_row(2, "Debug support", "disabled");
#endif
#ifdef HAVE_EVENT_EXTRA_LIB
	php_info_print_table_row(2, "Extra functionality support including HTTP, DNS, and RPC", "enabled");
#else
	php_info_print_table_row(2, "Extra functionality support including HTTP, DNS, and RPC", "disabled");
#endif
#ifdef HAVE_EVENT_OPENSSL_LIB
	php_info_print_table_row(2, "OpenSSL support", "enabled");
#else
	php_info_print_table_row(2, "OpenSSL support", "disabled");
#endif
#ifdef HAVE_EVENT_PTHREADS_LIB
	php_info_print_table_row(2, "Thread safety support", "enabled");
#else
	php_info_print_table_row(2, "Thread safety support", "disabled");
#endif

	php_info_print_table_row(2, "Extension version", PHP_EVENT_VERSION);
	php_info_print_table_row(2, "libevent2 headers version", LIBEVENT_VERSION);
	php_info_print_table_end();
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
