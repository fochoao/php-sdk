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
#include "../src/common.h"
#include "../src/util.h"
#include "../src/priv.h"
#include "zend_exceptions.h"

/* {{{ Private */

static void _conn_close_cb(struct evhttp_connection *conn, void *arg)/* {{{ */
{
	php_event_http_conn_t *evcon = (php_event_http_conn_t *) arg;
	zend_fcall_info       *pfci;
	zend_fcall_info_cache *pfcc;
	zval  *arg_data;
	zval  *arg_conn;
	zval **args[2];
	zval  *retval_ptr = NULL;
	PHP_EVENT_TSRM_DECL

	PHP_EVENT_ASSERT(evcon && conn);

	pfci = evcon->fci_closecb;
	pfcc = evcon->fcc_closecb;
	if (!(pfci && pfcc)) {
		return;
	}
	PHP_EVENT_ASSERT(pfci && pfcc);

	PHP_EVENT_TSRMLS_FETCH_FROM_CTX(evcon->thread_ctx);

	/* Call userspace function according to
	 * proto void callback(EventHttpConnection conn, mixed data); */

	arg_data = evcon->data_closecb;

	arg_conn = evcon->self;
	if (conn == NULL || !arg_conn) {
		ALLOC_INIT_ZVAL(arg_conn);
	} else {
		/* Let it auto-destroy */
#if 0
		Z_ADDREF_P(arg_conn);
#endif
	}
	args[0] = &arg_conn;

	if (arg_data) {
		Z_ADDREF_P(arg_data);
	} else {
		ALLOC_INIT_ZVAL(arg_data);
	}
	args[1] = &arg_data;

	pfci->params		 = args;
	pfci->retval_ptr_ptr = &retval_ptr;
	pfci->param_count	 = 2;
	pfci->no_separation  = 1;

	if (zend_call_function(pfci, pfcc TSRMLS_CC) == SUCCESS && retval_ptr) {
		zval_ptr_dtor(&retval_ptr);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"An error occurred while invoking the http connection close callback");
	}

	zval_ptr_dtor(args[1]);
	zval_ptr_dtor(args[0]);
}/* }}} */

/* Private }}} */


/* {{{ proto EventHttpConnection EventHttpConnection::__construct(EventBase base, EventDnsBase dns_base, string address, int port[, EventSslContext ctx = NULL]);
 * If <parameter>dns_base</parameter> is &null;, hostname resolution will block.
 *
 * If <parameter>ctx</parameter> is available since Libevent-2.1.0-alpha.
 */
PHP_METHOD(EventHttpConnection, __construct)
{
	zval                     *zself       = getThis();
	zval                     *zbase;
	php_event_base_t         *b;
	zval                     *zdns_base   = NULL;
	php_event_dns_base_t     *dnsb = NULL;
	char                     *address;
	int                       address_len;
	long                      port;
	php_event_http_conn_t    *evcon;
	struct evhttp_connection *conn;

#if LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
	php_event_ssl_context_t *ectx;
	zval                    *zctx    = NULL;
	struct bufferevent      *bevent  = NULL;
	long                     options;
	SSL                     *ssl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO!sl|O!",
				&zbase, php_event_base_ce, &zdns_base, php_event_dns_base_ce,
				&address, &address_len, &port,
				&zctx, php_event_ssl_context_ce) == FAILURE) {
		return;
	}
#else /* < Libevent-2.1.0-alpha */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO!sl",
				&zbase, php_event_base_ce, &zdns_base, php_event_dns_base_ce,
				&address, &address_len, &port) == FAILURE) {
		return;
	}
#endif

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (zdns_base) {
		PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zself);

#if LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
	if (zctx) {
		PHP_EVENT_FETCH_SSL_CONTEXT(ectx, zctx);
		PHP_EVENT_ASSERT(ectx->ctx);

		ssl = SSL_new(ectx->ctx);
		if (!ssl) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to create SSL handle");
			return;
		}
		/* Attach ectx to ssl for callbacks */
		SSL_set_ex_data(ssl, php_event_ssl_data_index, ectx);

#ifdef HAVE_EVENT_PTHREADS_LIB
		options = BEV_OPT_DEFER_CALLBACKS | BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE;
#else
		options = BEV_OPT_DEFER_CALLBACKS | BEV_OPT_CLOSE_ON_FREE;
#endif

		bevent = bufferevent_openssl_socket_new(b->base, -1, ssl, BUFFEREVENT_SSL_CONNECTING, options);
		if (!bevent) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Failed to allocate bufferevent filter");
			return;
		}
	}

	/* bevent (if not NULL) will be freed when the connection closes (see the doc for
	 * evhttp_connection_base_bufferevent_new() */
	conn = evhttp_connection_base_bufferevent_new(b->base,
			(zdns_base ? dnsb->dns_base : NULL),
			bevent,
			address,
			(unsigned short) port);
#else /* < Libevent 2.1.0-alpha */
	conn = evhttp_connection_base_new(b->base,
			(zdns_base ? dnsb->dns_base : NULL),
			address,
			(unsigned short) port);
#endif
	if (!conn) {
		return;
	}
	evcon->conn = conn;

	evcon->self = zself;
	Z_ADDREF_P(zself);

	evcon->base = zbase;
	Z_ADDREF_P(zbase);

	evcon->dns_base = zdns_base;
	if (zdns_base) {
		Z_ADDREF_P(zdns_base);
	}
}
/* }}} */

/* {{{ proto int EventHttpConnection::__wakeup()
   Prevents use of a EventHttpConnection instance that has been unserialized */
PHP_METHOD(EventHttpConnection, __wakeup)
{
	zend_throw_exception_ex(php_event_get_exception(), 0 TSRMLS_CC, "EventHttpConnection instances are not serializable");
}
/* }}} */

/* {{{ proto int EventHttpConnection::__sleep()
   Prevents serialization of a EventHttpConnection instance */
PHP_METHOD(EventHttpConnection, __sleep)
{
	zend_throw_exception_ex(php_event_get_exception(), 0 TSRMLS_CC, "EventHttpConnection instances are not serializable");
}
/* }}} */

/* {{{ proto EventBase EventHttpConnection::getBase(void);
 *
 * Get event base associated with the http connection.
 */
PHP_METHOD(EventHttpConnection, getBase)
{
	zval                 *zevcon = getThis();
	php_event_http_conn_t *evcon;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	/*
	 * No sense in this call
	 * base = evhttp_connection_get_base(evcon->con);
	 */

	if (evcon->base) {
		RETURN_ZVAL(evcon->base, 1, 0);
	}

	RETVAL_FALSE;
}
/* }}} */

/* {{{ proto void EventHttpConnection::getPeer(string &address, int &port);
 * Get the remote address and port associated with this connection. */
PHP_METHOD(EventHttpConnection, getPeer)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	zval                  *zaddress;
	zval                  *zport;

	char *address;
	unsigned short port;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz",
				&zaddress, &zport) == FAILURE) {
		return;
	}

	if (! (Z_ISREF_P(zaddress) && Z_ISREF_P(zport))) {
		/* Was not passed by reference */
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_get_peer(evcon->conn, &address, &port);

	ZVAL_STRING(zaddress, address, 1);
	ZVAL_LONG(zport, port);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setLocalAddress(string address);
 * Sets the ip address from which http connections are made */
PHP_METHOD(EventHttpConnection, setLocalAddress)
{
	zval                  *zevcon      = getThis();
	php_event_http_conn_t *evcon;
	char                  *address;
	int                    address_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&address, &address_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_local_address(evcon->conn, address);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setLocalPort(int port);
 * Sets the port from which http connections are made */
PHP_METHOD(EventHttpConnection, setLocalPort)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	long                   port;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&port) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_local_port(evcon->conn, port);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setTimeout(int timeout);
 */
PHP_METHOD(EventHttpConnection, setTimeout)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	long                   timeout;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&timeout) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_timeout(evcon->conn, timeout);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setMaxHeadersSize(int max_size);
 */
PHP_METHOD(EventHttpConnection, setMaxHeadersSize)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	long                   max_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&max_size) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_max_headers_size(evcon->conn, (ev_ssize_t) max_size);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setMaxBodySize(int max_size);
 */
PHP_METHOD(EventHttpConnection, setMaxBodySize)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	long                   max_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&max_size) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_max_body_size(evcon->conn, (ev_ssize_t) max_size);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setRetries(int retries);
 */
PHP_METHOD(EventHttpConnection, setRetries)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	long                   retries;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&retries) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_retries(evcon->conn, retries);
}
/* }}} */

/* {{{ proto bool EventHttpConnection::makeRequest(EventHttpRequest req, int type, string uri);
 * Makes an HTTP request over the specified connection.
 * <parameter>type</parameter> is one of <literal>EventHttpRequest::CMD_*</literal> constants.
 */
PHP_METHOD(EventHttpConnection, makeRequest)
{
	zval                  *zevcon   = getThis();
	php_event_http_conn_t *evcon;
	zval                  *zreq;
	php_event_http_req_t  *http_req;
	long                   type;
	char                  *uri;
	int                    uri_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ols",
				&zreq, php_event_http_req_ce, &type, &uri, &uri_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, zreq);
	if (!http_req->ptr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Unconfigured HTTP request object passed");
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	if (evhttp_make_request(evcon->conn, http_req->ptr, type, uri)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ void EventHttpConnection::setCloseCallback(callable callback[, mixed data]);
 * Set callback for connection close. */
PHP_METHOD(EventHttpConnection, setCloseCallback)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	zend_fcall_info        fci    = empty_fcall_info;
	zend_fcall_info_cache  fcc    = empty_fcall_info_cache;
	zval                  *zarg   = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f|z!",
				&fci, &fcc, &zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	PHP_EVENT_FREE_FCALL_INFO(evcon->fci_closecb, evcon->fcc_closecb);
	PHP_EVENT_COPY_FCALL_INFO(evcon->fci_closecb, evcon->fcc_closecb, &fci, &fcc);

	if (zarg) {
		if (evcon->data_closecb) {
			zval_ptr_dtor(&evcon->data_closecb);
		}
		evcon->data_closecb = zarg;
		Z_ADDREF_P(zarg);
	}

	TSRMLS_SET_CTX(evcon->thread_ctx);

	evhttp_connection_set_closecb(evcon->conn, _conn_close_cb, (void *) evcon);
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
