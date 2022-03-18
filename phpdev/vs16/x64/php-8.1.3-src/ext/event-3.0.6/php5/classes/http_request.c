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

/* {{{ _check_http_req_ptr */
#define _check_http_req_ptr(http_req)               \
{                                                   \
    if (!http_req->ptr) {                           \
        php_error_docref(NULL TSRMLS_CC, E_WARNING, \
                "Invalid HTTP request object");     \
        RETURN_FALSE;                               \
    }                                               \
}
/* }}} */

/* {{{ _check_http_req_type */
#define _check_http_req_type(type)                                            \
{                                                                             \
    if (type & ~(PHP_EVENT_REQ_HEADER_INPUT | PHP_EVENT_REQ_HEADER_OUTPUT)) { \
        php_error_docref(NULL TSRMLS_CC, E_WARNING,                           \
                "Invalid HTTP request type passed: %ld", type);               \
        RETURN_FALSE;                                                         \
    }                                                                         \
}
/* }}} */

/* {{{ _get_http_req_headers */
static zend_always_inline struct evkeyvalq *_get_http_req_headers(const php_event_http_req_t *http_req, const long type)
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
	php_event_http_req_t *http_req = (php_event_http_req_t *) arg;
	zend_fcall_info       *pfci;
	zend_fcall_info_cache *pfcc;
	zval  *arg_data;
	zval  *arg_req;
	zval **args[2];
	zval  *retval_ptr = NULL;
	PHP_EVENT_TSRM_DECL

	PHP_EVENT_ASSERT(http_req && http_req->ptr);
	PHP_EVENT_ASSERT(http_req->fci && http_req->fcc);
	PHP_EVENT_ASSERT(http_req->self);

	pfci = http_req->fci;
	pfcc = http_req->fcc;
	PHP_EVENT_ASSERT(pfci && pfcc);

	PHP_EVENT_TSRMLS_FETCH_FROM_CTX(http_req->thread_ctx);

	/* Call userspace function according to
	 * proto void callback(EventHttpRequest req, mixed data); */

	arg_data = http_req->data;

	arg_req = http_req->self;
	/* req == NULL means timeout */
	if (req == NULL || !arg_req) {
		ALLOC_INIT_ZVAL(arg_req);
	} else {
		Z_ADDREF_P(arg_req);
	}
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

	/* Tell Libevent that we will free the request ourselves(evhttp_request_free in the free-storage handler)*/
	/*evhttp_request_own(http_req->ptr);*/

	if (zend_call_function(pfci, pfcc TSRMLS_CC) == SUCCESS && retval_ptr) {
		zval_ptr_dtor(&retval_ptr);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"An error occurred while invoking the http request callback");
	}

	zval_ptr_dtor(&arg_req);
	zval_ptr_dtor(&arg_data);
}
/* }}} */

/* }}} */


/* {{{ proto EventHttpRequest::__construct(callable callback[, mixed data = NULL]); */
PHP_METHOD(EventHttpRequest, __construct)
{
	zval                  *zself    = getThis();
	php_event_http_req_t  *http_req;
	zend_fcall_info        fci      = empty_fcall_info;
	zend_fcall_info_cache  fcc      = empty_fcall_info_cache;
	zval                  *zarg     = NULL;
	struct evhttp_request *req;


	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f|z",
				&fci, &fcc, &zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, zself);

	req = evhttp_request_new(_req_handler, (void *) http_req);
	PHP_EVENT_ASSERT(req);

	/* Tell Libevent that we will free the request ourselves(evhttp_request_free in the free-storage handler)
	 * XXX Not sure if it's really needed here though. */
	/*evhttp_request_own(req);*/
	http_req->ptr = req;

	if (zarg) {
		Z_ADDREF_P(zarg);
	}
	http_req->data = zarg;

	http_req->self = zself;
	Z_ADDREF_P(zself);

	PHP_EVENT_COPY_FCALL_INFO(http_req->fci, http_req->fcc, &fci, &fcc);

	TSRMLS_SET_CTX(http_req->thread_ctx);
}
/* }}} */

/* {{{ proto int EventHttpRequest::__wakeup()
   Prevents use of a EventHttpRequest instance that has been unserialized */
PHP_METHOD(EventHttpRequest, __wakeup)
{
	zend_throw_exception_ex(php_event_get_exception(), 0 TSRMLS_CC, "EventHttpRequest instances are not serializable");
}
/* }}} */

/* {{{ proto int EventHttpRequest::__sleep()
   Prevents serialization of a EventHttpRequest instance */
PHP_METHOD(EventHttpRequest, __sleep)
{
	zend_throw_exception_ex(php_event_get_exception(), 0 TSRMLS_CC, "EventHttpRequest instances are not serializable");
}
/* }}} */

/* {{{ proto void EventHttpRequest::free(void);
 * Frees the object and removes associated events. */
PHP_METHOD(EventHttpRequest, free)
{
	zval                 *zself    = getThis();
	php_event_http_req_t *http_req;

	PHP_EVENT_FETCH_HTTP_REQ(http_req, zself);

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
	if (http_req->self) {
		zval_ptr_dtor(&http_req->self);
		http_req->self = NULL;
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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	_check_http_req_ptr(http_req);

	RETVAL_STRING(evhttp_request_get_host(http_req->ptr), 1);
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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	_check_http_req_ptr(http_req);

	RETVAL_STRING(evhttp_request_get_uri(http_req->ptr), 1);
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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	_check_http_req_ptr(http_req);

	array_init(return_value);

	headers = evhttp_request_get_input_headers(http_req->ptr);
	for (header = headers->tqh_first; header;
			header = header->next.tqe_next) {
		add_assoc_string(return_value, header->key, header->value, 1);
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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	_check_http_req_ptr(http_req);

	array_init(return_value);

	headers = evhttp_request_get_output_headers(http_req->ptr);
	for (header = headers->tqh_first; header;
			header = header->next.tqe_next) {
		add_assoc_string(return_value, header->key, header->value, 1);
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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	_check_http_req_ptr(http_req);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_buffer_ce);
	PHP_EVENT_FETCH_BUFFER(b, return_value);
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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	_check_http_req_ptr(http_req);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_buffer_ce);
	PHP_EVENT_FETCH_BUFFER(b, return_value);
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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	_check_http_req_ptr(http_req);

	conn = evhttp_request_get_connection(http_req->ptr);
	if (conn == NULL) {
		RETURN_NULL();
	}

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_bevent_ce);
	PHP_EVENT_FETCH_BEVENT(bev, return_value);

	bev->bevent = evhttp_connection_get_bufferevent(conn);
	bev->self = return_value;
	Z_ADDREF_P(return_value);
	bev->input = NULL;
	bev->output = NULL;
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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	_check_http_req_ptr(http_req);

	conn = evhttp_request_get_connection(http_req->ptr);
	if (conn == NULL) {
		RETURN_NULL();
	}

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_http_conn_ce);
	PHP_EVENT_FETCH_HTTP_CONN(evcon, return_value);

	evcon->conn = conn;
	evcon->self = return_value;
	Z_ADDREF_P(evcon->self);
	evcon->internal = 1;

	/* Set in ctor:
	   evcon->base = NULL;
	   evcon->dns_base = NULL;
	   evcon->data_closecb = NULL;
	   evcon->fci_closecb = NULL;
	   evcon->fcc_closecb = NULL;
	*/
#if 0
	Z_ADDREF_P(return_value);
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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

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
	long                  error;
	char                 *reason = NULL;
	int                   reason_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|s!",
				&error, &reason, &reason_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

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
	long                  code;
	char                 *reason;
	int                   reason_len;
	zval                 *zbuf = NULL;
	php_event_buffer_t   *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls|O!",
				&code, &reason, &reason_len,
				&zbuf, php_event_buffer_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	_check_http_req_ptr(http_req);

	if (zbuf) {
		PHP_EVENT_FETCH_BUFFER(b, zbuf);
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&zbuf, php_event_buffer_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	_check_http_req_ptr(http_req);

	if (zbuf) {
		PHP_EVENT_FETCH_BUFFER(b, zbuf);
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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

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
	long                  code;
	char                 *reason;
	int                   reason_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls",
				&code, &reason, &reason_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

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
	int                   key_len;
	int                   value_len;
	struct evkeyvalq     *headers;
	long                  type;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssl",
				&key, &key_len, &value, &value_len, &type) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());
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

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());
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
	int                   key_len;
	struct evkeyvalq     *headers;
	long                  type;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl",
				&key, &key_len, &type) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

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
	int                   key_len;
	struct evkeyvalq     *headers;
	long                  type;
	const char *val;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl",
				&key, &key_len, &type) == FAILURE) {
		return;
	}

	_check_http_req_type(type);

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());
	_check_http_req_ptr(http_req);

	headers = _get_http_req_headers(http_req, type);
	PHP_EVENT_ASSERT(headers);

	val = evhttp_find_header(headers, key);
	if (val == NULL) {
		RETURN_NULL();
	}

	RETVAL_STRING(val, 1);
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
