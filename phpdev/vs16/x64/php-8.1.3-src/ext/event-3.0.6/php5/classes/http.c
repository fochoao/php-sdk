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
#include "http.h"
#include "zend_exceptions.h"

/* {{{ Private */

/* {{{ _new_http_cb
 * Allocate memory for new callback structure for the next HTTP server's URI */
static zend_always_inline php_event_http_cb_t *_new_http_cb(zval *zbase, zval *zarg, const zend_fcall_info *fci, const zend_fcall_info_cache *fcc TSRMLS_DC)
{
	php_event_http_cb_t *cb = emalloc(sizeof(php_event_http_cb_t));

	if (zarg) {
		Z_ADDREF_P(zarg);
	}
	cb->data = zarg;

	cb->base = zbase;
	Z_ADDREF_P(zbase);

	PHP_EVENT_COPY_FCALL_INFO(cb->fci, cb->fcc, fci, fcc);

	TSRMLS_SET_CTX(cb->thread_ctx);

	cb->next = NULL;

	return cb;
}
/* }}} */

/* {{{ _http_callback */
static void _http_callback(struct evhttp_request *req, void *arg)
{
	php_event_http_cb_t *cb = (php_event_http_cb_t *) arg;
	php_event_base_t *b;
	php_event_http_req_t *http_req;
	zend_fcall_info       *pfci;
	zend_fcall_info_cache *pfcc;
	zval  *arg_data;
	zval  *arg_req;
	zval **args[2];
	zval  *retval_ptr = NULL;
	PHP_EVENT_TSRM_DECL

	PHP_EVENT_ASSERT(cb);

	pfci = cb->fci;
	pfcc = cb->fcc;
	PHP_EVENT_ASSERT(pfci && pfcc);

	PHP_EVENT_TSRMLS_FETCH_FROM_CTX(cb->thread_ctx);

	/* Call userspace function according to
	 * proto void callback(EventHttpRequest req, mixed data);*/

	arg_data = cb->data;

	MAKE_STD_ZVAL(arg_req);
	PHP_EVENT_INIT_CLASS_OBJECT(arg_req, php_event_http_req_ce);
	PHP_EVENT_FETCH_HTTP_REQ(http_req, arg_req);
	http_req->ptr      = req;
#if 0
	http_req->internal = 1; /* Don't evhttp_request_free(req) */
	Z_ADDREF_P(arg_req);
#endif
	args[0] = &arg_req;

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
		if (EG(exception)) {
			PHP_EVENT_ASSERT(cb->base);
			PHP_EVENT_FETCH_BASE(b, cb->base);
			event_base_loopbreak(b->base);

			zval_ptr_dtor(&arg_req);
			zval_ptr_dtor(&arg_data);
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"An error occurred while invoking the http request callback");
		}
	}

	zval_ptr_dtor(&arg_req);
	zval_ptr_dtor(&arg_data);
}
/* }}} */

/* {{{ _http_default_callback */
static void _http_default_callback(struct evhttp_request *req, void *arg)
{
	php_event_http_t *http = (php_event_http_t *) arg;
	php_event_http_req_t *http_req;
	zend_fcall_info       *pfci;
	zend_fcall_info_cache *pfcc;
	zval  *arg_data;
	zval  *arg_req;
	zval **args[2];
	zval  *retval_ptr = NULL;
	php_event_base_t *b;
	PHP_EVENT_TSRM_DECL

	PHP_EVENT_ASSERT(http);

	pfci = http->fci;
	pfcc = http->fcc;
	PHP_EVENT_ASSERT(pfci && pfcc);

	PHP_EVENT_TSRMLS_FETCH_FROM_CTX(http->thread_ctx);

	/* Call userspace function according to
	 * proto void callback(EventHttpRequest req, mixed data);*/

	arg_data = http->data;

	MAKE_STD_ZVAL(arg_req);
	PHP_EVENT_INIT_CLASS_OBJECT(arg_req, php_event_http_req_ce);
	PHP_EVENT_FETCH_HTTP_REQ(http_req, arg_req);
	http_req->ptr      = req;
#if 0
	http_req->internal = 1; /* Don't evhttp_request_free(req) */
	Z_ADDREF_P(arg_req);
#endif
	args[0] = &arg_req;

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
		if (EG(exception)) {
			PHP_EVENT_ASSERT(http && http->base);
			PHP_EVENT_FETCH_BASE(b, http->base);
			event_base_loopbreak(b->base);

			zval_ptr_dtor(&arg_req);
			zval_ptr_dtor(&arg_data);
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"An error occurred while invoking the http request callback");
		}
	}

	zval_ptr_dtor(&arg_req);
	zval_ptr_dtor(&arg_data);
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

/* {{{  _php_event_free_http_cb */
void _php_event_free_http_cb(php_event_http_cb_t *cb)
{
	if (cb->data) {
		zval_ptr_dtor(&cb->data);
		cb->data = NULL;
	}
	if (cb->base) {
		zval_ptr_dtor(&cb->base);
		cb->base = NULL;
	}

	PHP_EVENT_FREE_FCALL_INFO(cb->fci, cb->fcc);

	efree(cb);
}
/* }}} */

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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|O!",
				&zbase, php_event_base_ce,
				&zctx, php_event_ssl_context_ce) == FAILURE) {
		return;
	}
#else
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&zbase, php_event_base_ce) == FAILURE) {
		return;
	}
#endif

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	PHP_EVENT_FETCH_BASE(b, zbase);

	PHP_EVENT_FETCH_HTTP(http, getThis());

	http_ptr = evhttp_new(b->base);
	if (!http_ptr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Failed to allocate space for new HTTP server(evhttp_new)");
		return;
	}
	http->ptr = http_ptr;

	http->base = zbase;
	Z_ADDREF_P(zbase);

	http->fci     = NULL;
	http->fcc     = NULL;
	http->data    = NULL;
	http->cb_head = NULL;

	TSRMLS_SET_CTX(http->thread_ctx);

#if LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
	if (zctx) {
		PHP_EVENT_FETCH_SSL_CONTEXT(ectx, zctx);
		PHP_EVENT_ASSERT(ectx->ctx);
		evhttp_set_bevcb(http_ptr, _bev_ssl_callback, ectx->ctx);
	}
#endif
}
/* }}} */

/* {{{ proto int EventHttp::__wakeup()
   Prevents use of a EventHttp instance that has been unserialized */
PHP_METHOD(EventHttp, __wakeup)
{
	zend_throw_exception_ex(php_event_get_exception(), 0 TSRMLS_CC, "EventHttp instances are not serializable");
}
/* }}} */

/* {{{ proto int EventHttp::__sleep()
   Prevents serialization of a EventHttp instance */
PHP_METHOD(EventHttp, __sleep)
{
	zend_throw_exception_ex(php_event_get_exception(), 0 TSRMLS_CC, "EventHttp instances are not serializable");
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
	zval             **ppzfd;
	evutil_socket_t    fd;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z",
				&ppzfd) == FAILURE) {
		return;
	}

	fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}
	evutil_make_socket_nonblocking(fd);

	PHP_EVENT_FETCH_HTTP(http, zhttp);

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
	int               address_len;
	long              port;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl",
				&address, &address_len, &port) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

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
	zval                  *zhttp    = getThis();
	php_event_http_t      *http;
	char                  *path;
	int                    path_len;
	zend_fcall_info        fci      = empty_fcall_info;
	zend_fcall_info_cache  fcc      = empty_fcall_info_cache;
	zval                  *zarg     = NULL;
	int                    res;
	php_event_http_cb_t   *cb;
	php_event_http_cb_t   *cb_head;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sf|z!",
				&path, &path_len, &fci, &fcc, &zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

	cb = _new_http_cb(http->base, zarg, &fci, &fcc TSRMLS_CC);
	PHP_EVENT_ASSERT(cb);

	res = evhttp_set_cb(http->ptr, path, _http_callback, (void *) cb);
	if (res == -2) {
		_php_event_free_http_cb(cb);

		RETURN_FALSE;
	}
	if (res == -1) { // the callback existed already
		_php_event_free_http_cb(cb);

		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"The callback already exists");
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
	zval                  *zhttp    = getThis();
	php_event_http_t      *http;
	zend_fcall_info        fci      = empty_fcall_info;
	zend_fcall_info_cache  fcc      = empty_fcall_info_cache;
	zval                  *zarg     = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f|z!",
				&fci, &fcc, &zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

	if (http->fci) {
		PHP_EVENT_FREE_FCALL_INFO(http->fci, http->fcc);
	}
	PHP_EVENT_COPY_FCALL_INFO(http->fci, http->fcc, &fci, &fcc);

	if (zarg) {
		Z_ADDREF_P(zarg);
	}
	http->data = zarg;

	evhttp_set_gencb(http->ptr, _http_default_callback, (void *) http);
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
	zval             *zhttp   = getThis();
	php_event_http_t *http;
	long              methods;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&methods) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

	evhttp_set_allowed_methods(http->ptr, methods);
}
/* }}} */

/* {{{ proto void EventHttp::setMaxBodySize(int value);
 */
PHP_METHOD(EventHttp, setMaxBodySize)
{
	zval             *zhttp   = getThis();
	php_event_http_t *http;
	long              value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&value) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

	evhttp_set_max_body_size(http->ptr, value);
}
/* }}} */

/* {{{ proto void EventHttp::setMaxHeadersSize(int value);
 */
PHP_METHOD(EventHttp, setMaxHeadersSize)
{
	zval             *zhttp   = getThis();
	php_event_http_t *http;
	long              value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&value) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

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
	long              value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&value) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

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
	int               alias_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&alias, &alias_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

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
	int               alias_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&alias, &alias_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

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
