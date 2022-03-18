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

/* {{{ _get_pos */
static int _get_pos(struct evbuffer_ptr *out_ptr, const long pos, struct evbuffer *buf TSRMLS_DC)
{
	if (pos < 0) {
		return FAILURE;
	}

	if (evbuffer_ptr_set(buf, out_ptr, pos, EVBUFFER_PTR_SET) == -1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Failed to set position to %ld", pos);
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */


/* {{{ proto EventBuffer EventBuffer::__construct(void); */
PHP_METHOD(EventBuffer, __construct)
{
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, getThis());

	b->buf = evbuffer_new();
}
/* }}} */

/* {{{ proto bool EventBuffer::freeze(bool at_front);
 * Prevent calls that modify an event buffer from succeeding. */
PHP_METHOD(EventBuffer, freeze)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;
	zend_bool           at_front;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b",
				&at_front) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (evbuffer_freeze(b->buf, at_front)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBuffer::unfreeze(bool at_front);
 * Re-enable calls that modify an event buffer. */
PHP_METHOD(EventBuffer, unfreeze)
{
	zval               *zbuf     = getThis();
	php_event_buffer_t *b;
	zend_bool           at_front;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b",
				&at_front) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (evbuffer_unfreeze(b->buf, at_front)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto void EventBuffer::lock(void);
 * Acquire the lock on an evbuffer.
 * Has no effect if locking was not enabled with evbuffer_enable_locking.
 */
PHP_METHOD(EventBuffer, lock)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	evbuffer_lock(b->buf);
}
/* }}} */

/* {{{ proto void EventBuffer::unlock(void);
 * Release the lock on an evbuffer.
 * Has no effect if locking was not enabled with evbuffer_enable_locking.
 */
PHP_METHOD(EventBuffer, unlock)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	evbuffer_unlock(b->buf);
}
/* }}} */

/* {{{ proto void EventBuffer::enableLocking(void);
 *
 * Enable locking on an evbuffer so that it can safely be used by multiple threads at the same time.
 * When locking is enabled, the lock will be held when callbacks are invoked.
 * This could result in deadlock if you aren't careful. Plan accordingly!
 */
PHP_METHOD(EventBuffer, enableLocking)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	evbuffer_enable_locking(b->buf, NULL);
}
/* }}} */

/* {{{ proto bool EventBuffer::add(string data);
 *
 * Append data to the end of an event buffer.
 */
PHP_METHOD(EventBuffer, add)
{
	php_event_buffer_t  *b;
	zval                *zbuf    = getThis();
	zval               **ppzdata;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z",
				&ppzdata) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	convert_to_string_ex(ppzdata);

	if (evbuffer_add(b->buf, (void *) Z_STRVAL_PP(ppzdata), Z_STRLEN_PP(ppzdata))) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto string EventBuffer::read(long max_bytes);
 *
 * Read data from an evbuffer and drain the bytes read.  If more bytes are
 * requested than are available in the evbuffer, we only extract as many bytes
 * as were available.
 */
PHP_METHOD(EventBuffer, read)
{
	php_event_buffer_t *b;
	zval               *zbuf      = getThis();
	long                max_bytes;
	long                ret;
	char               *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&max_bytes) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	data = emalloc(sizeof(char) * max_bytes + 1);

	ret = evbuffer_remove(b->buf, data, max_bytes);
	if (ret > 0) {
		RETVAL_STRINGL(data, ret, 1);
	} else {
		RETVAL_NULL();
	}

	efree(data);
}
/* }}} */

/* {{{ proto bool EventBuffer::addBuffer(EventBuffer buf);
 * Move all data from the buffer provided in buf parameter to the current instance of EventBuffer.
 * This is a destructive add. The data from one buffer moves into the other buffer. However, no unnecessary memory copies occur.
 */
PHP_METHOD(EventBuffer, addBuffer)
{
	php_event_buffer_t *b_dst;
	php_event_buffer_t *b_src;
	zval               *zbuf_dst = getThis();
	zval               *zbuf_src;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&zbuf_src, php_event_buffer_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b_dst, zbuf_dst);
	PHP_EVENT_FETCH_BUFFER(b_src, zbuf_src);

	if (evbuffer_add_buffer(b_dst->buf, b_src->buf)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int EventBuffer::appendFrom(EventBuffer buf, int len);
 * Moves exactly len bytes from buf to the end of current instance of EventBuffer
 */
PHP_METHOD(EventBuffer, appendFrom)
{
	php_event_buffer_t *b_dst;
	php_event_buffer_t *b_src;
	zval               *zbuf_dst = getThis();
	zval               *zbuf_src;
	long                len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ol",
				&zbuf_src, php_event_buffer_ce, &len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b_dst, zbuf_dst);
	PHP_EVENT_FETCH_BUFFER(b_src, zbuf_src);

	RETVAL_LONG(evbuffer_remove_buffer(b_src->buf, b_dst->buf, (size_t) len));
}
/* }}} */

/* {{{ proto bool EventBuffer::expand(int len);
 * Alters the last chunk of memory in the buffer, or adds a new chunk, such that the buffer is now large enough to contain datlen bytes without any further allocations.
 */
PHP_METHOD(EventBuffer, expand)
{
	php_event_buffer_t *b;
	zval               *zbuf = getThis();
	long                len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (evbuffer_expand(b->buf, (size_t) len)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBuffer::prepend(string data);
 *
 * Prepend data to the front of the event buffer.
 */
PHP_METHOD(EventBuffer, prepend)
{
	php_event_buffer_t  *b;
	zval                *zbuf    = getThis();
	zval               **ppzdata;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z",
				&ppzdata) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	convert_to_string_ex(ppzdata);

	if (evbuffer_prepend(b->buf, (void *) Z_STRVAL_PP(ppzdata), Z_STRLEN_PP(ppzdata))) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBuffer::prependBuffer(EventBuffer buf);
 * Behaves as EventBuffer::addBuffer, except that it moves data to the front of the buffer.
 */
PHP_METHOD(EventBuffer, prependBuffer)
{
	php_event_buffer_t *b_dst;
	php_event_buffer_t *b_src;
	zval               *zbuf_dst = getThis();
	zval               *zbuf_src;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&zbuf_src, php_event_buffer_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b_dst, zbuf_dst);
	PHP_EVENT_FETCH_BUFFER(b_src, zbuf_src);

	if (evbuffer_prepend_buffer(b_dst->buf, b_src->buf)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBuffer::drain(long len);
 *
 * Behaves as EventBuffer::remove(), except that it does not copy the data: it
 * just removes it from the front of the buffer.
 */
PHP_METHOD(EventBuffer, drain)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;
	long                len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (evbuffer_drain(b->buf, len)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int EventBuffer::copyout(string &data, long max_bytes);
 *
 * Behaves just like EventBuffer::remove(), but does not drain any data from the buffer.
 * I.e. it copies the first max_bytes bytes from the front of the buffer into data.
 * If there are fewer than datlen bytes available, the function copies all the bytes there are.
 *
 * Returns the number of bytes copied, or -1 on failure.
 */
PHP_METHOD(EventBuffer, copyout)
{
	php_event_buffer_t *b;
	zval               *zbuf      = getThis();
	zval               *zdata;
	long                max_bytes;
	long                ret;
	char               *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl",
				&zdata, &max_bytes) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	data = emalloc(sizeof(char) * max_bytes + 1);

	ret = evbuffer_copyout(b->buf, data, max_bytes);

	if (ret > 0) {
		convert_to_string(zdata);
		zval_dtor(zdata);
		Z_STRVAL_P(zdata) = estrndup(data, ret);
		Z_STRLEN_P(zdata) = ret;
	}

	efree(data);

	RETVAL_LONG(ret);
}
/* }}} */

/* {{{ proto mixed EventBuffer::readLine(int eol_style);
 *
 * Extracts a line from the front of the buffer and returns it in a newly
 * allocated NUL-terminated string. If there is not a whole
 * line to read, the function returns NULL. The line terminator is not included
 * in the copied string.
 *
 * eol_style is one of EventBuffer:EOL_* constants.
 *
 * On success returns the line read from the buffer, otherwise NULL.
 */
PHP_METHOD(EventBuffer, readLine)
{
	zval               *zbuf      = getThis();
	php_event_buffer_t *b;
	long                eol_style;
	char               *res;
	size_t              len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&eol_style) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	res = evbuffer_readln(b->buf, &len, eol_style);

	if (!res) {
		RETURN_NULL();
	}

	RETVAL_STRINGL(res, len, 1);
	free(res);
}
/* }}} */

/* {{{ proto mixed EventBuffer::search(int what[, int start = -1[, int end = -1]]);
 *
 * Scans the buffer for an occurrence of the len-character string what. It
 * returns object representing the position of the string, or NULL if the
 * string was not found. If the start argument is provided, it's the position
 * at which the search should begin; otherwise, the search is from the start
 * of the string. If end argument provided, the search is performed between
 * start and end buffer positions.
 *
 * Returns position of the first occurance of the string
 * in the buffer, or &false; if string is not found.
 */
PHP_METHOD(EventBuffer, search)
{
	zval               *zbuf      = getThis();
	long                start_pos = -1;
	long                end_pos   = -1;
	char               *what;
	int                 what_len;
	php_event_buffer_t *b;

	struct evbuffer_ptr ptr_start, ptr_end, ptr_res;


	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ll",
				&what, &what_len,
				&start_pos,
				&end_pos) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (start_pos != -1
			&& _get_pos(&ptr_start, start_pos, b->buf TSRMLS_CC) == FAILURE) {
		start_pos = -1;
	}
	if (end_pos != -1 &&
		 	(end_pos > evbuffer_get_length(b->buf)
			 || _get_pos(&ptr_end, end_pos, b->buf TSRMLS_CC) == FAILURE)) {
		end_pos = -1;
	}

	if (end_pos != -1) {
		ptr_res = evbuffer_search_range(b->buf, what, (size_t) what_len,
				(start_pos != -1 ? &ptr_start : NULL), &ptr_end);
	} else {
		ptr_res = evbuffer_search(b->buf, what, (size_t) what_len,
				(start_pos != -1 ? &ptr_start : NULL));
	}

	if (ptr_res.pos == -1) {
		RETURN_FALSE;
	}
	RETVAL_LONG(ptr_res.pos);
}
/* }}} */

/* {{{ proto int EventBuffer::searchEol([int start = -1[, int eol_style = EventBuffer::EOL_ANY]]);
 * Searches for occurance of end of line.
 *
 * Returns unsigned numeric position on success. Otherwise -1.
 */
PHP_METHOD(EventBuffer, searchEol)
{
	zval               *zbuf      = getThis();
	long                start_pos = -1;
	long                eol_style = EVBUFFER_EOL_ANY;
	php_event_buffer_t *b;

	struct evbuffer_ptr ptr_start, ptr_res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ll",
				&start_pos, &eol_style) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (start_pos != -1
			&& _get_pos(&ptr_start, start_pos, b->buf TSRMLS_CC) == FAILURE) {
		start_pos = -1;
	}

	ptr_res = evbuffer_search_eol(b->buf, (start_pos != -1 ? &ptr_start : NULL),
			NULL, eol_style);

	if (ptr_res.pos == -1) {
		RETURN_FALSE;
	}
	RETVAL_LONG(ptr_res.pos);
}
/* }}} */

/* {{{ proto string EventBuffer::pullup(int size);
 *
 * "Linearizes" the first size bytes of the buffer, copying or moving them as needed to
 * ensure that they are all contiguous and occupying the same chunk of memory. If size is
 * negative, the function linearizes the entire buffer. If size is greater than the number
 * of bytes in the buffer, the function returns NULL. Otherwise, EventBuffer::pullup()
 * returns string.
 *
 * Calling EventBuffer::pullup() with a large size can be quite slow, since it potentially
 * needs to copy the entire buffer's contents.
 */
PHP_METHOD(EventBuffer, pullup)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;
	long                size;
	unsigned char      *mem;
	size_t              length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&size) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	mem = evbuffer_pullup(b->buf, size);

	if (mem == NULL) {
		RETURN_NULL();
	}

	length = evbuffer_get_length(b->buf);

	/* evbuffer_pullup() doesn't add terminating zero */
	mem[length] = '\0';

	RETVAL_STRINGL((const char *)mem, length, 1);
}
/* }}} */

/* {{{ proto int EventBuffer::write(mixed fd[, int howmuch]);
 *
 * Write contents of the buffer to a file descriptor.
 * The buffer will be drained after the bytes have been successfully written.
 *
 * Returns the number of bytes written, or &false; on error.
 */
PHP_METHOD(EventBuffer, write)
{
	zval                *zbuf  = getThis();
	php_event_buffer_t  *b;
	zval               **ppzfd;
	evutil_socket_t      fd;
	long                 res;
	long                 howmuch = -1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z|l",
				&ppzfd, &howmuch) == FAILURE) {
		return;
	}

	fd = php_event_zval_to_fd(ppzfd TSRMLS_CC);
	if (fd == -1) {
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (howmuch < 0) {
		res = evbuffer_write(b->buf, fd);
	} else {
		res = evbuffer_write_atmost(b->buf, fd, howmuch);
	}

	if (res == -1) {
		RETURN_FALSE;
	}

	RETVAL_LONG(res);
}
/* }}} */

/* {{{ proto int EventBuffer::readFrom(mixed fd[, int howmuch]);
 *
 * Read data from a file descriptor onto the end of the buffer.
 *
 * Returns the number of bytes read, or &false; on error.
 */
PHP_METHOD(EventBuffer, readFrom)
{
	zval                *zbuf = getThis();
	php_event_buffer_t  *b;
	zval               **ppzfd;
	evutil_socket_t      fd;
	long                 res;
	long                 howmuch = -1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z|l",
				&ppzfd, &howmuch) == FAILURE) {
		return;
	}

	fd = php_event_zval_to_fd(ppzfd TSRMLS_CC);
	if (fd == -1) {
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	res = evbuffer_read(b->buf, fd, howmuch);

	if (res == -1) {
		RETURN_FALSE;
	}

	RETVAL_LONG(res);
}
/* }}} */

/* {{{ proto string EventBuffer::substr(int start[, int length]);
 * Returns portion of the buffer contents specified by
 * <parameter>start</parameter> and <parameter>length</parameter>
 */
PHP_METHOD(EventBuffer, substr)
{
	zval               *zbuf   = getThis();
	php_event_buffer_t *b;
	long                n_start;
	long                n_length = -1;

	struct evbuffer_ptr    ptr;
	struct evbuffer_iovec *pv;
	int                    n_chunks;
	long                   n_read   = 0;
	int                    i;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l",
				&n_start, &n_length) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (_get_pos(&ptr, n_start, b->buf TSRMLS_CC) == FAILURE) {
		RETURN_FALSE;
	}

	/* Determine how many chunks we need */
	n_chunks = evbuffer_peek(b->buf, n_length, &ptr, NULL, 0);
	/* Allocate space for the chunks. */
	pv = emalloc(sizeof(struct evbuffer_iovec) * n_chunks);
	/* Fill up pv */
	n_chunks = evbuffer_peek(b->buf, n_length, &ptr, pv, n_chunks);

	/* Determine the size of the result string */
	for (i = 0; i < n_chunks; ++i) {
		size_t len = pv[i].iov_len;

		if (n_read + len > n_length) {
			len = n_length - n_read;
		}

		n_read += len;
	}

	/* Build result string */
	Z_TYPE_P(return_value)   = IS_STRING;
	Z_STRLEN_P(return_value) = n_read;
	Z_STRVAL_P(return_value) = emalloc(n_read + 1);

	for (n_read = 0, i = 0; i < n_chunks; ++i) {
		size_t len = pv[i].iov_len;

		if (n_read + len > n_length) {
			len = n_length - n_read;
		}

		memcpy(Z_STRVAL_P(return_value) + n_read, pv[i].iov_base, len);

		n_read += len;
	}
	Z_STRVAL_P(return_value)[n_read] = '\0';

	efree(pv);
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
