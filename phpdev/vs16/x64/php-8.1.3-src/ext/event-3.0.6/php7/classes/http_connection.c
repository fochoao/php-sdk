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
#include "../src/common.h"
#include "../src/util.h"
#include "../src/priv.h"
#include "zend_exceptions.h"

/* {{{ Private */

static void _conn_close_cb(struct evhttp_connection *conn, void *arg)/* {{{ */
{
	php_event_http_conn_t *evcon   = (php_event_http_conn_t *)arg;
	zend_fcall_info        fci;
	zval                   argv[2];
	zval                   retval;
	zval                   zcallable;

	PHP_EVENT_ASSERT(evcon && conn);

	/* Protect against accidental destruction of the func name before zend_call_function() finished */
	ZVAL_COPY(&zcallable, &evcon->cb_close.func_name);

	/* Call userspace function according to
	 * proto void callback(EventHttpConnection conn, mixed data); */

	if (conn == NULL || Z_ISUNDEF(evcon->self)) {
		ZVAL_NULL(&argv[0]);
	} else {
		ZVAL_COPY(&argv[0], &evcon->self);
	}

	if (Z_ISUNDEF(evcon->data_closecb)) {
		ZVAL_NULL(&argv[1]);
	} else {
		ZVAL_COPY(&argv[1], &evcon->data_closecb);
	}

	fci.size = sizeof(fci);
#ifdef HAVE_PHP_ZEND_FCALL_INFO_FUNCTION_TABLE
	fci.function_table = EG(function_table); /* XXX fetch TSRMLS_CACHE? */
#endif
	ZVAL_COPY_VALUE(&fci.function_name, &zcallable);
	fci.object        = NULL;
	fci.retval        = &retval;
	fci.param_count   = 2;
	fci.params        = argv;
	fci.no_separation = 1;
#ifdef HAVE_PHP_ZEND_FCALL_INFO_SYMBOL_TABLE
	fci.symbol_table  = NULL;
#endif

	if (zend_call_function(&fci, &evcon->cb_close.fci_cache) == SUCCESS) {
		if (!Z_ISUNDEF(retval)) {
			zval_ptr_dtor(&retval);
		}
	} else {
		php_error_docref(NULL, E_WARNING, "Failed to invoke http connection close callback");
	}

	zval_ptr_dtor(&zcallable);

	zval_ptr_dtor(&argv[0]);
	zval_ptr_dtor(&argv[1]);
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
	php_event_dns_base_t     *dnsb        = NULL;
	char                     *address;
	size_t                    address_len;
	zend_long                 port;
	php_event_http_conn_t    *evcon;
	struct evhttp_connection *conn;

#if LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
	php_event_ssl_context_t *ectx;
	zval                    *zctx    = NULL;
	struct bufferevent      *bevent  = NULL;
	zend_long                    options;
	SSL                     *ssl;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "OO!sl|O!",
				&zbase, php_event_base_ce, &zdns_base, php_event_dns_base_ce,
				&address, &address_len, &port,
				&zctx, php_event_ssl_context_ce) == FAILURE) {
		return;
	}
#else /* < Libevent-2.1.0-alpha */
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "OO!sl",
				&zbase, php_event_base_ce, &zdns_base, php_event_dns_base_ce,
				&address, &address_len, &port) == FAILURE) {
		return;
	}
#endif

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	b = Z_EVENT_BASE_OBJ_P(zbase);

	if (zdns_base) {
		dnsb = Z_EVENT_DNS_BASE_OBJ_P(zdns_base);
	}

	evcon = Z_EVENT_HTTP_CONN_OBJ_P(zself);

#if LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
	if (zctx) {
		ectx = Z_EVENT_SSL_CONTEXT_OBJ_P(zctx);
		PHP_EVENT_ASSERT(ectx->ctx);

		ssl = SSL_new(ectx->ctx);
		if (!ssl) {
			php_error_docref(NULL, E_WARNING, "Failed to create SSL handle");
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
			php_error_docref(NULL, E_ERROR, "Failed to allocate bufferevent filter");
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

	ZVAL_COPY_VALUE(&evcon->self, zself);
	ZVAL_COPY(&evcon->base, zbase);

	if (zdns_base) {
		ZVAL_COPY(&evcon->dns_base, zdns_base);
	} else {
		ZVAL_UNDEF(&evcon->dns_base);
	}

	ZVAL_UNDEF(&evcon->data_closecb);
}
/* }}} */

/*{{{ proto int EventHttpConnection::__sleep */
PHP_METHOD(EventHttpConnection, __sleep)
{
	zend_throw_exception_ex(php_event_get_exception(), 0, "EventHttpConnection instances are not serializable");
}
/*}}}*/

/*{{{ proto int EventHttpConnection::__wakeup */
PHP_METHOD(EventHttpConnection, __wakeup)
{
	zend_throw_exception_ex(php_event_get_exception(), 0, "EventHttpConnection instances are not serializable");
}
/*}}}*/

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

	evcon = Z_EVENT_HTTP_CONN_OBJ_P(zevcon);

	/*
	 * No sense in this call
	 * base = evhttp_connection_get_base(evcon->con);
	 */

	if (!Z_ISUNDEF(evcon->base)) {
		RETURN_ZVAL(&evcon->base, 1, 0);
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

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "zz",
				&zaddress, &zport) == FAILURE) {
		return;
	}

	if (! (Z_ISREF_P(zaddress) && Z_ISREF_P(zport))) {
		/* Was not passed by reference */
		return;
	}

	evcon = Z_EVENT_HTTP_CONN_OBJ_P(zevcon);

	evhttp_connection_get_peer(evcon->conn, &address, &port);

	ZVAL_STRING(zaddress, address);
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
	size_t                 address_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s",
				&address, &address_len) == FAILURE) {
		return;
	}

	evcon = Z_EVENT_HTTP_CONN_OBJ_P(zevcon);

	evhttp_connection_set_local_address(evcon->conn, address);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setLocalPort(int port);
 * Sets the port from which http connections are made */
PHP_METHOD(EventHttpConnection, setLocalPort)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	zend_long                  port;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&port) == FAILURE) {
		return;
	}

	evcon = Z_EVENT_HTTP_CONN_OBJ_P(zevcon);

	evhttp_connection_set_local_port(evcon->conn, port);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setTimeout(int timeout);
 */
PHP_METHOD(EventHttpConnection, setTimeout)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	zend_long                  timeout;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&timeout) == FAILURE) {
		return;
	}

	evcon = Z_EVENT_HTTP_CONN_OBJ_P(zevcon);

	evhttp_connection_set_timeout(evcon->conn, timeout);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setMaxHeadersSize(int max_size);
 */
PHP_METHOD(EventHttpConnection, setMaxHeadersSize)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	zend_long                  max_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&max_size) == FAILURE) {
		return;
	}

	evcon = Z_EVENT_HTTP_CONN_OBJ_P(zevcon);

	evhttp_connection_set_max_headers_size(evcon->conn, (ev_ssize_t) max_size);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setMaxBodySize(int max_size);
 */
PHP_METHOD(EventHttpConnection, setMaxBodySize)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	zend_long                  max_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&max_size) == FAILURE) {
		return;
	}

	evcon = Z_EVENT_HTTP_CONN_OBJ_P(zevcon);

	evhttp_connection_set_max_body_size(evcon->conn, (ev_ssize_t) max_size);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setRetries(int retries);
 */
PHP_METHOD(EventHttpConnection, setRetries)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	zend_long                  retries;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&retries) == FAILURE) {
		return;
	}

	evcon = Z_EVENT_HTTP_CONN_OBJ_P(zevcon);

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
	zend_long              type;
	char                  *uri;
	size_t                 uri_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Ols",
				&zreq, php_event_http_req_ce, &type, &uri, &uri_len) == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(zreq);
	if (!http_req->ptr) {
		php_error_docref(NULL, E_WARNING,
				"Unconfigured HTTP request object passed");
		return;
	}

	evcon = Z_EVENT_HTTP_CONN_OBJ_P(zevcon);

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
	php_event_http_conn_t *evcon;
	zval                  *zcb;
	zval                  *zarg  = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|z!", &zcb, &zarg) == FAILURE) {
		return;
	}

	evcon = Z_EVENT_HTTP_CONN_OBJ_P(getThis());
	PHP_EVENT_ASSERT(evcon);

	php_event_replace_callback(&evcon->cb_close, zcb);
	php_event_replace_zval(&evcon->data_closecb, zarg);

	evhttp_connection_set_closecb(evcon->conn, _conn_close_cb, (void *)evcon);
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
