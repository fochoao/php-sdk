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
#include "http.h"
#include "zend_exceptions.h"

/* {{{ Private */

/* {{{ _new_http_cb
 * Allocate memory for new callback structure for the next HTTP server's URI */
static zend_always_inline php_event_http_cb_t * _new_http_cb(zval *zbase, zval *zarg, zval *zcb)
{
	php_event_http_cb_t *cb = ecalloc(1, sizeof(php_event_http_cb_t));

	PHP_EVENT_ASSERT(cb);

	if (zarg) {
		ZVAL_COPY(&cb->data, zarg);
	} else {
		ZVAL_UNDEF(&cb->data);
	}

	ZVAL_COPY(&cb->base, zbase);

	php_event_copy_callback(&cb->cb, zcb);

	/* ecalloc() already did it
	cb->next = NULL; */

	return cb;
}
/* }}} */

/* {{{ _http_callback */
static void _http_callback(struct evhttp_request *req, void *arg)
{
	php_event_http_cb_t *cb;
	zend_fcall_info      fci;
	zval                 argv[2];
	zval                 retval;
	zval                 zcallable;
	zend_string         *func_name;
	Z_EVENT_X_OBJ_T(base) *b;
	Z_EVENT_X_OBJ_T(http_req) *http_req;

	cb = (php_event_http_cb_t *)arg;
	PHP_EVENT_ASSERT(cb);

	/* Protect against accidental destruction of the func name before zend_call_function() finished */
	ZVAL_COPY(&zcallable, &cb->cb.func_name);

	if (!zend_is_callable(&zcallable, IS_CALLABLE_STRICT, &func_name)) {
		zend_string_release(func_name);
		return;
	}
	zend_string_release(func_name);

	/* Call userspace function according to
	 * proto void callback(EventHttpRequest req, mixed data);*/

	PHP_EVENT_INIT_CLASS_OBJECT(&argv[0], php_event_http_req_ce);
	http_req = Z_EVENT_HTTP_REQ_OBJ_P(&argv[0]);
	http_req->ptr = req;
	ZVAL_UNDEF(&http_req->self);
	ZVAL_UNDEF(&http_req->data);
	php_event_init_callback(&http_req->cb);
	http_req->internal = 1; /* Don't evhttp_request_free(req) */

	if (Z_ISUNDEF(cb->data)) {
		ZVAL_NULL(&argv[1]);
	} else {
		ZVAL_COPY(&argv[1], &cb->data);
	}

	fci.size = sizeof(fci);
#ifdef HAVE_PHP_ZEND_FCALL_INFO_FUNCTION_TABLE
	fci.function_table = EG(function_table);
#endif
	ZVAL_COPY_VALUE(&fci.function_name, &zcallable);
	fci.object = NULL;
	fci.retval = &retval;
	fci.params = argv;
	fci.param_count = 2;
	fci.no_separation = 1;
#ifdef HAVE_PHP_ZEND_FCALL_INFO_SYMBOL_TABLE
	fci.symbol_table = NULL;
#endif

	if (zend_call_function(&fci, &cb->cb.fci_cache) == SUCCESS) {
		if (!Z_ISUNDEF(retval)) {
			zval_ptr_dtor(&retval);
		}
	} else {
		if (EG(exception)) {
			b = Z_EVENT_BASE_OBJ_P(&cb->base);
			PHP_EVENT_ASSERT(b && b->base);
			event_base_loopbreak(b->base);

			if (!Z_ISUNDEF(argv[0])) {
				zval_ptr_dtor(&argv[0]);
			}
			if (!Z_ISUNDEF(argv[1])) {
				zval_ptr_dtor(&argv[1]);
			}
		} else {
			php_error_docref(NULL, E_WARNING, "Failed to invoke the http request callback");
		}
	}

	zval_ptr_dtor(&zcallable);

	if (!Z_ISUNDEF(argv[0])) {
		zval_ptr_dtor(&argv[0]);
	}
	if (!Z_ISUNDEF(argv[1])) {
		zval_ptr_dtor(&argv[1]);
	}
}
/* }}} */

/* {{{ _http_default_callback */
static void _http_default_callback(struct evhttp_request *req, void *arg)
{
	php_event_http_t *http      = (php_event_http_t *) arg;
	zend_fcall_info   fci;
	zval              argv[2];
	zval              retval;
	zval              zcallable;
	zend_string      *func_name;
	Z_EVENT_X_OBJ_T(base) *b;
	Z_EVENT_X_OBJ_T(http_req) *http_req;

	PHP_EVENT_ASSERT(http);

	/* Protect against accidental destruction of the func name before zend_call_function() finished */
	ZVAL_COPY(&zcallable, &http->cb.func_name);

	if (!zend_is_callable(&zcallable, IS_CALLABLE_STRICT, &func_name)) {
		zend_string_release(func_name);
		return;
	}
	zend_string_release(func_name);

	/* Call userspace function according to
	 * proto void callback(EventHttpRequest req, mixed data);*/

	PHP_EVENT_INIT_CLASS_OBJECT(&argv[0], php_event_http_req_ce);
	http_req = Z_EVENT_HTTP_REQ_OBJ_P(&argv[0]);
	http_req->ptr = req;
	ZVAL_UNDEF(&http_req->self);
	ZVAL_UNDEF(&http_req->data);
	php_event_init_callback(&http_req->cb);
	http_req->internal = 1; /* Don't evhttp_request_free(req) */

	if (Z_ISUNDEF(http->data)) {
		ZVAL_NULL(&argv[1]);
	} else {
		ZVAL_COPY(&argv[1], &http->data);
	}

	fci.size = sizeof(fci);
#ifdef HAVE_PHP_ZEND_FCALL_INFO_FUNCTION_TABLE
	fci.function_table = EG(function_table);
#endif
	ZVAL_COPY_VALUE(&fci.function_name, &zcallable);
	fci.object = NULL;
	fci.retval = &retval;
	fci.params = argv;
	fci.param_count = 2;
	fci.no_separation  = 1;
#ifdef HAVE_PHP_ZEND_FCALL_INFO_SYMBOL_TABLE
	fci.symbol_table = NULL;
#endif

	if (zend_call_function(&fci, &http->cb.fci_cache) == SUCCESS) {
		if (!Z_ISUNDEF(retval)) {
			zval_ptr_dtor(&retval);
		}
	} else {
		if (EG(exception)) {
			PHP_EVENT_ASSERT(http && !Z_ISUNDEF(http->base));
			b = Z_EVENT_BASE_OBJ_P(&http->base);
			event_base_loopbreak(b->base);

			if (!Z_ISUNDEF(argv[0])) {
				zval_ptr_dtor(&argv[0]);
			}
			if (!Z_ISUNDEF(argv[1])) {
				zval_ptr_dtor(&argv[1]);
			}
		} else {
			php_error_docref(NULL, E_WARNING, "Failed to invoke http request callback");
		}
	}

	zval_ptr_dtor(&zcallable);

	if (!Z_ISUNDEF(argv[0])) {
		zval_ptr_dtor(&argv[0]);
	}
	if (!Z_ISUNDEF(argv[1])) {
		zval_ptr_dtor(&argv[1]);
	}
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
/*{{{ _bev_ssl_callback
 *
 * This callback is responsible for creating a new SSL connection
 * and wrapping it in an OpenSSL bufferevent.  This is the way
 * we implement an https server instead of a plain old http server.
 * (borrowed from https://github.com/ppelleti/https-example/blob/master/https-server.c)
 */
static struct bufferevent* _bev_ssl_callback(struct event_base *base, void *arg) {
	struct bufferevent* bev;
	SSL_CTX *ctx = (SSL_CTX *) arg;

	bev = bufferevent_openssl_socket_new(base,
			-1,
			SSL_new(ctx),
			BUFFEREVENT_SSL_ACCEPTING,
			BEV_OPT_CLOSE_ON_FREE);
	return bev;
}
/*}}}*/
#endif

/* }}} */

/*{{{ proto int EventHttp::__sleep */
PHP_METHOD(EventHttp, __sleep)
{
	zend_throw_exception_ex(php_event_get_exception(), 0, "EventHttp instances are not serializable");
}
/*}}}*/

/*{{{ proto int EventHttp::__wakeup */
PHP_METHOD(EventHttp, __wakeup)
{
	zend_throw_exception_ex(php_event_get_exception(), 0, "EventHttp instances are not serializable");
}
/*}}}*/

void _php_event_free_http_cb(php_event_http_cb_t *http_cb)/*{{{*/
{
	if (!Z_ISUNDEF(http_cb->data)) {
		zval_ptr_dtor(&http_cb->data);
	}
	if (!Z_ISUNDEF(http_cb->base)) {
		zval_ptr_dtor(&http_cb->base);
	}

	php_event_free_callback(&http_cb->cb);

	efree(http_cb);
} /*}}}*/

/* {{{ proto EventHttp EventHttp::__construct(EventBase base[, EventSslContext ctx = NULL]);
 * Creates new http server object.
 */
PHP_METHOD(EventHttp, __construct)
{
	zval             *zbase;
	php_event_base_t *b;
	php_event_http_t *http;
	struct evhttp    *http_ptr;

#if LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
	php_event_ssl_context_t *ectx;
	zval                    *zctx = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O|O!",
				&zbase, php_event_base_ce,
				&zctx, php_event_ssl_context_ce) == FAILURE) {
		return;
	}
#else
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O",
				&zbase, php_event_base_ce) == FAILURE) {
		return;
	}
#endif

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	b = Z_EVENT_BASE_OBJ_P(zbase);

	http = Z_EVENT_HTTP_OBJ_P(getThis());

	http_ptr = evhttp_new(b->base);
	if (!http_ptr) {
		php_error_docref(NULL, E_WARNING,
				"Failed to allocate space for new HTTP server(evhttp_new)");
		return;
	}
	http->ptr = http_ptr;

	ZVAL_COPY(&http->base, zbase);

	ZVAL_UNDEF(&http->cb.func_name);
	ZVAL_UNDEF(&http->data);
	http->cb_head = NULL;

#if LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
	if (zctx) {
		ectx = Z_EVENT_SSL_CONTEXT_OBJ_P(zctx);
		PHP_EVENT_ASSERT(ectx->ctx);
		evhttp_set_bevcb(http_ptr, _bev_ssl_callback, ectx->ctx);
	}
#endif
}
/* }}} */

/* {{{ proto bool EventHttp::accept(mixed socket);
 *
 * Makes an HTTP server accept connections on the specified socket stream or resource.
 * The socket should be ready to accept connections.
 * Can be called multiple times to accept connections on different sockets. */
PHP_METHOD(EventHttp, accept)
{
	php_event_http_t  *http;
	zval              *zhttp = getThis();
	zval              *pzfd;
	evutil_socket_t    fd;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &pzfd) == FAILURE) {
		return;
	}

	fd = (evutil_socket_t)php_event_zval_to_fd(pzfd);
	if (fd < 0) {
		RETURN_FALSE;
	}
	evutil_make_socket_nonblocking(fd);

	http = Z_EVENT_HTTP_OBJ_P(zhttp);

	if (evhttp_accept_socket(http->ptr, fd)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventHttp::bind(string address, int port);
 *
 * Binds an HTTP server on the specified address and port.
 * Can be called multiple times to bind the same http server to multiple different ports. */
PHP_METHOD(EventHttp, bind)
{
	zval             *zhttp       = getThis();
	php_event_http_t *http;
	char             *address;
	size_t            address_len;
	zend_long         port;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "sl",
				&address, &address_len, &port) == FAILURE) {
		return;
	}

	http = Z_EVENT_HTTP_OBJ_P(zhttp);

	/* XXX Call evhttp_bind_socket_with_handle instead, and store the bound
	 * socket in the internal struct for further useful API? */
	if (evhttp_bind_socket(http->ptr, address, port)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventHttp::setCallback(string path, callable cb[, mixed arg = NULL]);
 * Sets a callback for specified URI.
 */
PHP_METHOD(EventHttp, setCallback)
{
	php_event_http_t    *http;
	char                *path;
	size_t               path_len;
	int                  res;
	zval                *zcb;
	zval                *zarg     = NULL;
	php_event_http_cb_t *cb;
	php_event_http_cb_t *cb_head;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "sz|z!",
				&path, &path_len, &zcb, &zarg) == FAILURE) {
		return;
	}

	http = Z_EVENT_HTTP_OBJ_P(getThis());

	cb = _new_http_cb(&http->base, zarg, zcb);

	res = evhttp_set_cb(http->ptr, path, _http_callback, (void *)cb);
	if (res == -2) {
		_php_event_free_http_cb(cb);
		RETURN_FALSE;
	}
	if (res == -1) {
		_php_event_free_http_cb(cb);
		php_error_docref(NULL, E_WARNING, "The callback already exists");
		RETURN_FALSE;
	}

	cb_head       = http->cb_head;
	http->cb_head = cb;
	cb->next      = cb_head;

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto void EventHttp::setDefaultCallback(callable cb[, mixed arg = NULL]);
 * Sets default callback to handle requests that are not caught by specific callbacks
 */
PHP_METHOD(EventHttp, setDefaultCallback)
{
	php_event_http_t *http;
	zval             *zcb;
	zval             *zarg  = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|z!", &zcb, &zarg) == FAILURE) {
		return;
	}

	http = Z_EVENT_HTTP_OBJ_P(getThis());

	php_event_replace_callback(&http->cb, zcb);

	if (zarg) {
		ZVAL_COPY(&http->data, zarg);
	} else {
		ZVAL_UNDEF(&http->data);
	}

	evhttp_set_gencb(http->ptr, _http_default_callback, (void *)http);
}
/* }}} */

/* {{{ proto void EventHttp::setAllowedMethods(int methods);
 * Sets the what HTTP methods are supported in requests accepted by this
 * server, and passed to user callbacks.
 *
 * If not supported they will generate a <literal>"405 Method not
 * allowed"</literal> response.
 *
 * By default this includes the following methods: GET, POST, HEAD, PUT, DELETE.
 * See <literal>EventHttpRequest::CMD_*</literal> constants.
 */
PHP_METHOD(EventHttp, setAllowedMethods)
{
	php_event_http_t *http;
	zend_long         methods;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &methods) == FAILURE) {
		return;
	}

	http = Z_EVENT_HTTP_OBJ_P(getThis());

	evhttp_set_allowed_methods(http->ptr, methods);
}
/* }}} */

/* {{{ proto void EventHttp::setMaxBodySize(int value);
 */
PHP_METHOD(EventHttp, setMaxBodySize)
{
	zval             *zhttp   = getThis();
	php_event_http_t *http;
	zend_long             value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&value) == FAILURE) {
		return;
	}

	http = Z_EVENT_HTTP_OBJ_P(zhttp);

	evhttp_set_max_body_size(http->ptr, value);
}
/* }}} */

/* {{{ proto void EventHttp::setMaxHeadersSize(int value);
 */
PHP_METHOD(EventHttp, setMaxHeadersSize)
{
	zval             *zhttp   = getThis();
	php_event_http_t *http;
	zend_long             value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&value) == FAILURE) {
		return;
	}

	http = Z_EVENT_HTTP_OBJ_P(zhttp);

	evhttp_set_max_headers_size(http->ptr, value);
}
/* }}} */

/* {{{ proto void EventHttp::setTimeout(int value);
 * Sets timeout for an HTTP request
 */
PHP_METHOD(EventHttp, setTimeout)
{
	zval             *zhttp   = getThis();
	php_event_http_t *http;
	zend_long             value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&value) == FAILURE) {
		return;
	}

	http = Z_EVENT_HTTP_OBJ_P(zhttp);

	evhttp_set_timeout(http->ptr, value);
}
/* }}} */

/* {{{ proto bool EventHttp::addServerAlias(string alias);
 * Adds a server alias to the object.
 */
PHP_METHOD(EventHttp, addServerAlias)
{
	zval             *zhttp     = getThis();
	php_event_http_t *http;
	char             *alias;
	size_t            alias_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s",
				&alias, &alias_len) == FAILURE) {
		return;
	}

	http = Z_EVENT_HTTP_OBJ_P(zhttp);

	if (evhttp_add_server_alias(http->ptr, alias)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventHttp::removeServerAlias(string alias);
 * Removes a server alias from the object.
 */
PHP_METHOD(EventHttp, removeServerAlias)
{
	zval             *zhttp     = getThis();
	php_event_http_t *http;
	char             *alias;
	size_t            alias_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s",
				&alias, &alias_len) == FAILURE) {
		return;
	}

	http = Z_EVENT_HTTP_OBJ_P(zhttp);

	if (evhttp_remove_server_alias(http->ptr, alias)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
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
