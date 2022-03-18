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

/* {{{ _check_http_req_ptr */
#define _check_http_req_ptr(http_req)               \
{                                                   \
    if (!http_req->ptr) {                           \
        php_error_docref(NULL, E_WARNING, \
                "Invalid HTTP request object");     \
        RETURN_FALSE;                               \
    }                                               \
}
/* }}} */

/* {{{ _check_http_req_type */
#define _check_http_req_type(type)                                            \
{                                                                             \
    if (type & ~(PHP_EVENT_REQ_HEADER_INPUT | PHP_EVENT_REQ_HEADER_OUTPUT)) { \
        php_error_docref(NULL, E_WARNING,                           \
                "Invalid HTTP request type passed: %ld", type);               \
        RETURN_FALSE;                                                         \
    }                                                                         \
}
/* }}} */

/* {{{ _get_http_req_headers */
static zend_always_inline struct evkeyvalq *_get_http_req_headers(const php_event_http_req_t *http_req, const zend_long type)
{
	struct evkeyvalq *headers;

	if (type == PHP_EVENT_REQ_HEADER_OUTPUT) {
		headers = evhttp_request_get_output_headers(http_req->ptr);
	} else {
		headers = evhttp_request_get_input_headers(http_req->ptr);
	}

	return headers;
}
/* }}} */

/* {{{ _req_handler */
static void _req_handler(struct evhttp_request *req, void *arg)
{
	php_event_http_req_t *http_req  = (php_event_http_req_t *)arg;
	zend_fcall_info       fci;
	zval                  argv[2];
	zval                  retval;
	zval                  zcallable;
	zend_string          *func_name;

	PHP_EVENT_ASSERT(http_req && http_req->ptr);

	/* Protect against accidental destruction of the func name before zend_call_function() finished */
	ZVAL_COPY(&zcallable, &http_req->cb.func_name);

	if (!zend_is_callable(&zcallable, IS_CALLABLE_STRICT, &func_name)) {
		zend_string_release(func_name);
		return;
	}
	zend_string_release(func_name);

	/* Call userspace function according to
	 * proto void callback(EventHttpRequest req, mixed data); */

	/* req == NULL means timeout */
	if (req == NULL || Z_ISUNDEF(http_req->self)) {
		ZVAL_NULL(&argv[0]);
	} else {
		ZVAL_COPY(&argv[0], &http_req->self);
	}

	if (Z_ISUNDEF(http_req->data)) {
		ZVAL_NULL(&argv[1]);
	} else {
		ZVAL_COPY(&argv[1], &http_req->data);
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

	/* Tell Libevent that we will free the request ourselves(evhttp_request_free in the free-storage handler)*/
	/*evhttp_request_own(http_req->ptr);*/

	if (zend_call_function(&fci, &http_req->cb.fci_cache) == SUCCESS) {
		if (!Z_ISUNDEF(retval)) {
			zval_ptr_dtor(&retval);
		}
	} else {
		php_error_docref(NULL, E_WARNING, "Failed to invoke http request handler");
	}

	zval_ptr_dtor(&zcallable);

	zval_ptr_dtor(&argv[0]);
	zval_ptr_dtor(&argv[1]);
}
/* }}} */

/* }}} */

/*{{{ proto int EventHttpRequest::__sleep */
PHP_METHOD(EventHttpRequest, __sleep)
{
	zend_throw_exception_ex(php_event_get_exception(), 0, "EventHttpRequest instances are not serializable");
}
/*}}}*/

/*{{{ proto int EventHttpRequest::__wakeup */
PHP_METHOD(EventHttpRequest, __wakeup)
{
	zend_throw_exception_ex(php_event_get_exception(), 0, "EventHttpRequest instances are not serializable");
}
/*}}}*/

/* {{{ proto EventHttpRequest::__construct(callable callback[, mixed data = NULL]); */
PHP_METHOD(EventHttpRequest, __construct)
{
	zval                  *zself    = getThis();
	php_event_http_req_t  *http_req;
	zval                  *zcb;
	zval                  *zarg     = NULL;
	struct evhttp_request *req;


	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|z!", &zcb, &zarg) == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(zself);

	req = evhttp_request_new(_req_handler, (void *)http_req);
	PHP_EVENT_ASSERT(req);

	/* Tell Libevent that we will free the request ourselves(evhttp_request_free in the free-storage handler)
	 * XXX Not sure if it's really needed here though. */
	evhttp_request_own(req);
	http_req->ptr = req;

	ZVAL_COPY(&http_req->self, zself);
	if (zarg) {
		ZVAL_COPY(&http_req->data, zarg);
	} else {
		ZVAL_UNDEF(&http_req->data);
	}
	php_event_copy_callback(&http_req->cb, zcb);
}
/* }}} */

/* {{{ proto void EventHttpRequest::free(void);
 * Frees the object and removes associated events. */
PHP_METHOD(EventHttpRequest, free)
{
	php_event_http_req_t *http_req;

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());
	PHP_EVENT_ASSERT(http_req);

	if (!http_req->ptr || http_req->internal) {
		return;
	}

	if (http_req->ptr) {
		/*
		 * We're not calling evhttp_request_free(http_req->ptr) because AFAIK
		 * Libevent handles the memory of evhttp_request all right.
		 *
		 * It just so happens that libevent invokes the function itself in
		 * evhttp_connection_cb_cleanup() despite the ownership of the request
		 * (thus causing a SEGFAULT). See bitbucket issue #3.
		 *
		 * By marking http_req as `internal` we prevent calling evhttp_request_free()
		 * within event_http_req_object_free_storage().
		 */
		http_req->internal = 1;
		/*http_req->ptr = NULL;*/
	}

	/* Do it once */
	if (!Z_ISUNDEF(http_req->self)) {
		zval_ptr_dtor(&http_req->self);
		ZVAL_UNDEF(&http_req->self);
	}
}
/* }}} */

/* {{{ proto int EventHttpRequest::getCommand(void);
 * Returns the request command, one of EventHttpRequest::CMD_* constants. XXX Make property? */
PHP_METHOD(EventHttpRequest, getCommand)
{
	php_event_http_req_t *http_req;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	RETVAL_LONG(evhttp_request_get_command(http_req->ptr));
}
/* }}} */

/* {{{ proto string EventHttpRequest::getHost(void);
 * Returns the request host. XXX make a property? */
PHP_METHOD(EventHttpRequest, getHost)
{
	php_event_http_req_t *http_req;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	RETVAL_STRING(evhttp_request_get_host(http_req->ptr));
}
/* }}} */

/* {{{ proto int EventHttpRequest::getUri(void);
 * Returns the request URI. XXX make a property? */
PHP_METHOD(EventHttpRequest, getUri)
{
	php_event_http_req_t *http_req;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	RETVAL_STRING(evhttp_request_get_uri(http_req->ptr));
}
/* }}} */

/* {{{ proto int EventHttpRequest::getResponseCode(void);
 * Returns the the response code. XXX make a property? */
PHP_METHOD(EventHttpRequest, getResponseCode)
{
	php_event_http_req_t *http_req;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	RETVAL_LONG(evhttp_request_get_response_code(http_req->ptr));
}
/* }}} */

/* {{{ proto array EventHttpRequest::getInputHeaders(void);
 * Returns associative array of the input headers. */
PHP_METHOD(EventHttpRequest, getInputHeaders)
{
	php_event_http_req_t *http_req;
	struct evkeyvalq     *headers;
	struct evkeyval      *header;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	array_init(return_value);

	headers = evhttp_request_get_input_headers(http_req->ptr);
	for (header = headers->tqh_first; header;
			header = header->next.tqe_next) {
		add_assoc_string(return_value, header->key, header->value);
	}


}
/* }}} */

/* {{{ proto array EventHttpRequest::getOutputHeaders(void);
 * Returns associative array of the output headers. */
PHP_METHOD(EventHttpRequest, getOutputHeaders)
{
	php_event_http_req_t *http_req;
	struct evkeyvalq     *headers;
	struct evkeyval      *header;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	array_init(return_value);

	headers = evhttp_request_get_output_headers(http_req->ptr);
	for (header = headers->tqh_first; header;
			header = header->next.tqe_next) {
		add_assoc_string(return_value, header->key, header->value);
	}
}
/* }}} */

/* {{{ proto EventBuffer EventHttpRequest::getInputBuffer(void);
 * Returns input buffer. */
PHP_METHOD(EventHttpRequest, getInputBuffer)
{
	php_event_http_req_t *http_req;
	php_event_buffer_t   *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_buffer_ce);
	b = Z_EVENT_BUFFER_OBJ_P(return_value);
	b->buf      = evhttp_request_get_input_buffer(http_req->ptr);
	b->internal = 1;
}
/* }}} */

/* {{{ proto EventBuffer EventHttpRequest::getOutputBuffer(void);
 * Returns output buffer. */
PHP_METHOD(EventHttpRequest, getOutputBuffer)
{
	php_event_http_req_t *http_req;
	php_event_buffer_t   *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_buffer_ce);
	b = Z_EVENT_BUFFER_OBJ_P(return_value);
	b->buf      = evhttp_request_get_output_buffer(http_req->ptr);
	b->internal = 1;
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02001100
/* {{{ proto EventBufferEvent EventHttpRequest::getBufferEvent(void);
 * Returns EventBufferEvent object on success, otherwise &null. */
PHP_METHOD(EventHttpRequest, getBufferEvent)
{
	php_event_http_req_t     *http_req;
	struct evhttp_connection *conn;
	php_event_bevent_t       *bev;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	conn = evhttp_request_get_connection(http_req->ptr);
	if (conn == NULL) {
		RETURN_NULL();
	}

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_bevent_ce);
	bev = Z_EVENT_BEVENT_OBJ_P(return_value);

	bev->bevent = evhttp_connection_get_bufferevent(conn);
	ZVAL_COPY(&bev->self, return_value);
	ZVAL_UNDEF(&bev->input);
	ZVAL_UNDEF(&bev->output);
	bev->_internal = 1;
}
/* }}} */
#endif

/* {{{ proto EventHttpConnection EventHttpRequest::getConnection(void);
 * Returns EventHttpConnection object.
 *
 * Warning! Libevent API allows http request objects not bound to any http connection.
 * Therefore we can't unambiguously associate EventHttpRequest with EventHttpConnection.
 * Thus, we construct EventHttpConnection object on-the-fly. Having no information about
 * base, dns_base and connection-close callback, we just leave these fields unset.
 *
 * If somebody finds some way to return full-value EventHttpConnection object,
 * please don't hesitate to make a pull request.
 */
PHP_METHOD(EventHttpRequest, getConnection)
{
	php_event_http_req_t     *http_req;
	struct evhttp_connection *conn;
	php_event_http_conn_t    *evcon;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	conn = evhttp_request_get_connection(http_req->ptr);
	if (conn == NULL) {
		RETURN_NULL();
	}

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_http_conn_ce);
	evcon = Z_EVENT_HTTP_CONN_OBJ_P(return_value);

	evcon->conn = conn;
	evcon->internal = TRUE;
	ZVAL_COPY(&evcon->self, return_value);

#if 0
	ZVAL_UNDEF(&evcon->base);
	ZVAL_UNDEF(&evcon->dns_base);
	ZVAL_UNDEF(&evcon->data_closecb);
	ZVAL_UNDEF(&evcon->cb_close.func_name);
#endif

#if 0
	Z_TRY_ADDREF_P(return_value);
#endif
}
/* }}} */

/* {{{ proto void EventHttpRequest::closeConnection(void);
 */
PHP_METHOD(EventHttpRequest, closeConnection)
{
	php_event_http_req_t     *http_req;
	struct evhttp_connection *conn;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	conn = evhttp_request_get_connection(http_req->ptr);
	if (conn == NULL) {
		return;
	}
	evhttp_connection_free(conn);
}
/* }}} */

/* {{{ proto void EventHttpRequest::sendError(int error[, string reason = NULL]);
 * Send an HTML error message to the client.
 */
PHP_METHOD(EventHttpRequest, sendError)
{
	php_event_http_req_t *http_req;
	zend_long             error;
	char                 *reason     = NULL;
	size_t                reason_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|s!",
				&error, &reason, &reason_len) == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	evhttp_send_error(http_req->ptr, error, reason);
}
/* }}} */

/* {{{ proto void EventHttpRequest::sendReply(int code, string reason[, EventBuffer buf=&null;]);
 * Send an HTML reply to client.
 *
 * The body of the reply consists of data in <parameter>buf</parameter>. */
PHP_METHOD(EventHttpRequest, sendReply)
{
	php_event_http_req_t *http_req;
	zend_long             code;
	char                 *reason;
	size_t                reason_len;
	zval                 *zbuf       = NULL;
	php_event_buffer_t   *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ls|O!",
				&code, &reason, &reason_len,
				&zbuf, php_event_buffer_ce) == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	if (zbuf) {
		b = Z_EVENT_BUFFER_OBJ_P(zbuf);
		PHP_EVENT_ASSERT(b->buf);
	}

	evhttp_send_reply(http_req->ptr, code, reason,
			(zbuf ? b->buf : NULL));
}
/* }}} */

/* {{{ proto void EventHttpRequest::sendReplyChunk(EventBuffer buf);
 * Send another data chunk as part of an ongoing chunked reply.
 *
 * After calling this method <parameter>buf</parameter> will be	empty. */
PHP_METHOD(EventHttpRequest, sendReplyChunk)
{
	php_event_http_req_t *http_req;
	zval                 *zbuf;
	php_event_buffer_t   *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O",
				&zbuf, php_event_buffer_ce) == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	if (zbuf) {
		b = Z_EVENT_BUFFER_OBJ_P(zbuf);
		PHP_EVENT_ASSERT(b->buf);
		evhttp_send_reply_chunk(http_req->ptr, b->buf);
	}
}
/* }}} */

/* {{{ proto void EventHttpRequest::sendReplyEnd(void);
 * Complete a chunked reply, freeing the request as appropriate.
 */
PHP_METHOD(EventHttpRequest, sendReplyEnd)
{
	php_event_http_req_t *http_req;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	evhttp_send_reply_end(http_req->ptr);
}
/* }}} */

/* {{{ proto void EventHttpRequest::sendReplyStart(int code, string reason);
 * Initiate a reply that uses <literal>Transfer-Encoding</literal>
 * <literal>chunked</literal>.
 *
 * This allows the caller to stream the reply back to the client and is useful
 * when either not all of the reply data is immediately available or when
 * sending very large replies.
 *
 * The caller needs to supply data chunks with
 * <method>EventHttpRequest::sendReplyChunk</method> and complete the reply by
 * calling <method>EventHttpRequest::sendReplyEnd</method>.
 */
PHP_METHOD(EventHttpRequest, sendReplyStart)
{
	php_event_http_req_t *http_req;
	zend_long             code;
	char                 *reason;
	size_t                reason_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ls",
				&code, &reason, &reason_len) == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);


	evhttp_send_reply_start(http_req->ptr, code, reason);
}
/* }}} */

/* {{{ proto void EventHttpRequest::cancel(void);
 * Cancels a pending HTTP request.
 *
 * Cancels an ongoing HTTP request. The callback associated with this request
 * is not executed and the request object is freed. If the request is currently
 * being processed, e.g. it is ongoing, the corresponding EventHttpConnection
 * object is going to get reset.
 *
 * A request cannot be canceled if its callback has executed already. A request
 * may be canceled reentrantly from its chunked callback.
 */
PHP_METHOD(EventHttpRequest, cancel)
{
	php_event_http_req_t *http_req;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	evhttp_cancel_request(http_req->ptr);
}
/* }}} */

/* {{{ proto bool EventHttpRequest::addHeader(string key, string value, int type);
 * Adds an HTTP header to the headers of the request.
 * <parameter>type</parameter> is one of <literal>EventHttpRequest::*_HEADER</literal>
 * constants.
 */
PHP_METHOD(EventHttpRequest, addHeader)
{
	php_event_http_req_t *http_req;
	char                 *key;
	char                 *value;
	size_t                key_len;
	size_t                value_len;
	struct evkeyvalq     *headers;
	zend_long             type;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ssl",
				&key, &key_len, &value, &value_len, &type) == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());
	_check_http_req_ptr(http_req);

	headers = _get_http_req_headers(http_req, type);
	PHP_EVENT_ASSERT(headers);


	if (evhttp_add_header(headers, key, value)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto void EventHttpRequest::clearHeaders(string key, string value);
 * Removes all output headers from the header list of the request.
 */
PHP_METHOD(EventHttpRequest, clearHeaders)
{
	php_event_http_req_t *http_req;
	struct evkeyvalq     *out_headers;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());
	_check_http_req_ptr(http_req);

	out_headers = evhttp_request_get_output_headers(http_req->ptr);
	PHP_EVENT_ASSERT(out_headers);

	evhttp_clear_headers(out_headers);
}
/* }}} */

/* {{{ proto bool EventHttpRequest::removeHeader(string key, int type);
 * Removes an HTTP header from the headers of the request.
 * <parameter>type</parameter> is one of <literal>EventHttpRequest::*_HEADER</literal>
 * constants.
 */
PHP_METHOD(EventHttpRequest, removeHeader)
{
	php_event_http_req_t *http_req;
	char                 *key;
	size_t                key_len;
	struct evkeyvalq     *headers;
	zend_long             type;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "sl",
				&key, &key_len, &type) == FAILURE) {
		return;
	}

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());

	_check_http_req_ptr(http_req);

	headers = _get_http_req_headers(http_req, type);
	PHP_EVENT_ASSERT(headers);

	if (evhttp_remove_header(headers, key)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto string EventHttpRequest::findHeader(string key, int type);
 * Finds the value belonging a header.
 * <parameter>type</parameter> is one of <literal>EventHttpRequest::*_HEADER</literal>
 * constants.
 * Returns &null; if header not found.
 */
PHP_METHOD(EventHttpRequest, findHeader)
{
	php_event_http_req_t *http_req;
	char                 *key;
	size_t                key_len;
	struct evkeyvalq     *headers;
	zend_long             type;
	const char           *val;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "sl",
				&key, &key_len, &type) == FAILURE) {
		return;
	}

	_check_http_req_type(type);

	http_req = Z_EVENT_HTTP_REQ_OBJ_P(getThis());
	_check_http_req_ptr(http_req);

	headers = _get_http_req_headers(http_req, type);
	PHP_EVENT_ASSERT(headers);

	val = evhttp_find_header(headers, key);
	if (val == NULL) {
		RETURN_NULL();
	}

	RETVAL_STRING(val);
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
