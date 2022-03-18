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

/* {{{ _get_pos */
static int _get_pos(struct evbuffer_ptr *out_ptr, const zend_long pos, struct evbuffer *buf)
{
	if (pos < 0) {
		return FAILURE;
	}

	if (evbuffer_ptr_set(buf, out_ptr, pos, EVBUFFER_PTR_SET) == -1) {
		php_error_docref(NULL, E_WARNING,
				"Failed to set position to %ld", pos);
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */


/* {{{ proto EventBuffer EventBuffer::__construct(void); */
PHP_EVENT_METHOD(EventBuffer, __construct)
{
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(getThis());

	b->buf = evbuffer_new();
}
/* }}} */

/* {{{ proto bool EventBuffer::freeze(bool at_front);
 * Prevent calls that modify an event buffer from succeeding. */
PHP_EVENT_METHOD(EventBuffer, freeze)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;
	zend_bool           at_front;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b",
				&at_front) == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	if (evbuffer_freeze(b->buf, at_front)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBuffer::unfreeze(bool at_front);
 * Re-enable calls that modify an event buffer. */
PHP_EVENT_METHOD(EventBuffer, unfreeze)
{
	zval               *zbuf     = getThis();
	php_event_buffer_t *b;
	zend_bool           at_front;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b",
				&at_front) == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

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
PHP_EVENT_METHOD(EventBuffer, lock)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	evbuffer_lock(b->buf);
}
/* }}} */

/* {{{ proto void EventBuffer::unlock(void);
 * Release the lock on an evbuffer.
 * Has no effect if locking was not enabled with evbuffer_enable_locking.
 */
PHP_EVENT_METHOD(EventBuffer, unlock)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	evbuffer_unlock(b->buf);
}
/* }}} */

/* {{{ proto void EventBuffer::enableLocking(void);
 *
 * Enable locking on an evbuffer so that it can safely be used by multiple threads at the same time.
 * When locking is enabled, the lock will be held when callbacks are invoked.
 * This could result in deadlock if you aren't careful. Plan accordingly!
 */
PHP_EVENT_METHOD(EventBuffer, enableLocking)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	evbuffer_enable_locking(b->buf, NULL);
}
/* }}} */

/* {{{ proto bool EventBuffer::add(string data);
 *
 * Append data to the end of an event buffer.
 */
PHP_EVENT_METHOD(EventBuffer, add)
{
	php_event_buffer_t *b;
	char               *data;
	size_t              data_length;
	zval               *zbuf        = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &data, &data_length) == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	if (evbuffer_add(b->buf, (void *)data, data_length)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto string EventBuffer::read(int max_bytes);
 *
 * Read data from an evbuffer and drain the bytes read.  If more bytes are
 * requested than are available in the evbuffer, we only extract as many bytes
 * as were available.
 */
PHP_EVENT_METHOD(EventBuffer, read)
{
	php_event_buffer_t *b;
	zval               *zbuf      = getThis();
	zend_long           max_bytes;
	zend_long           ret;
	char               *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&max_bytes) == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	data = emalloc(sizeof(char) * max_bytes + 1);

	ret = evbuffer_remove(b->buf, data, max_bytes);
	if (ret > 0) {
		RETVAL_STRINGL(data, ret);
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
PHP_EVENT_METHOD(EventBuffer, addBuffer)
{
	php_event_buffer_t *b_dst;
	php_event_buffer_t *b_src;
	zval               *zbuf_dst = getThis();
	zval               *zbuf_src;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O",
				&zbuf_src, php_event_buffer_ce) == FAILURE) {
		return;
	}

	b_dst = Z_EVENT_BUFFER_OBJ_P(zbuf_dst);
	b_src = Z_EVENT_BUFFER_OBJ_P(zbuf_src);

	if (evbuffer_add_buffer(b_dst->buf, b_src->buf)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int EventBuffer::appendFrom(EventBuffer buf, int len);
 * Moves exactly len bytes from buf to the end of current instance of EventBuffer
 */
PHP_EVENT_METHOD(EventBuffer, appendFrom)
{
	php_event_buffer_t *b_dst;
	php_event_buffer_t *b_src;
	zval               *zbuf_dst = getThis();
	zval               *zbuf_src;
	zend_long               len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Ol",
				&zbuf_src, php_event_buffer_ce, &len) == FAILURE) {
		return;
	}

	b_dst = Z_EVENT_BUFFER_OBJ_P(zbuf_dst);
	b_src = Z_EVENT_BUFFER_OBJ_P(zbuf_src);

	RETVAL_LONG(evbuffer_remove_buffer(b_src->buf, b_dst->buf, (size_t) len));
}
/* }}} */

/* {{{ proto bool EventBuffer::expand(int len);
 * Alters the last chunk of memory in the buffer, or adds a new chunk, such that the buffer is now large enough to contain datlen bytes without any further allocations.
 */
PHP_EVENT_METHOD(EventBuffer, expand)
{
	php_event_buffer_t *b;
	zval               *zbuf = getThis();
	zend_long               len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&len) == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

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
PHP_EVENT_METHOD(EventBuffer, prepend)
{
	php_event_buffer_t *b;
	char               *data;
	size_t              data_length;
	zval               *zbuf        = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &data, &data_length) == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	if (evbuffer_prepend(b->buf, (void *)data, data_length)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBuffer::prependBuffer(EventBuffer buf);
 * Behaves as EventBuffer::addBuffer, except that it moves data to the front of the buffer.
 */
PHP_EVENT_METHOD(EventBuffer, prependBuffer)
{
	php_event_buffer_t *b_dst;
	php_event_buffer_t *b_src;
	zval               *zbuf_dst = getThis();
	zval               *zbuf_src;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O",
				&zbuf_src, php_event_buffer_ce) == FAILURE) {
		return;
	}

	b_dst = Z_EVENT_BUFFER_OBJ_P(zbuf_dst);
	b_src = Z_EVENT_BUFFER_OBJ_P(zbuf_src);

	if (evbuffer_prepend_buffer(b_dst->buf, b_src->buf)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBuffer::drain(int len);
 *
 * Behaves as EventBuffer::remove(), except that it does not copy the data: it
 * just removes it from the front of the buffer.
 */
PHP_EVENT_METHOD(EventBuffer, drain)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;
	zend_long               len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&len) == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	if (evbuffer_drain(b->buf, len)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int EventBuffer::copyout(string &data, int max_bytes);
 *
 * Behaves just like EventBuffer::remove(), but does not drain any data from the buffer.
 * I.e. it copies the first max_bytes bytes from the front of the buffer into data.
 * If there are fewer than datlen bytes available, the function copies all the bytes there are.
 *
 * Returns the number of bytes copied, or -1 on failure.
 */
PHP_EVENT_METHOD(EventBuffer, copyout)
{
	php_event_buffer_t *b;
	zend_long           max_bytes;
	zend_long           ret;
	char               *data;
	zval               *zdata;
	zval               *zbuf      = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "zl",
				&zdata, &max_bytes) == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	data = emalloc(sizeof(char) * max_bytes + 1);

	ret = evbuffer_copyout(b->buf, data, max_bytes);

	if (ret > 0) {
		convert_to_string(zdata);
		zval_dtor(zdata);
#if 0
		Z_STRVAL_P(zdata) = estrndup(data, ret);
		Z_STRLEN_P(zdata) = ret;
#else
		ZVAL_STRINGL(zdata, data, ret);
#endif
	}

	efree(data);

	RETVAL_LONG(ret);
}
/* }}} */

/* {{{ proto ?string EventBuffer::readLine(int eol_style);
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
PHP_EVENT_METHOD(EventBuffer, readLine)
{
	zval               *zbuf      = getThis();
	php_event_buffer_t *b;
	zend_long               eol_style;
	char               *res;
	size_t              len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&eol_style) == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	res = evbuffer_readln(b->buf, &len, eol_style);

	if (!res) {
		RETURN_NULL();
	}

	RETVAL_STRINGL(res, len);
	free(res);
}
/* }}} */

/* {{{ proto int|false EventBuffer::search(string what[, int start = -1[, int end = -1]]);
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
PHP_EVENT_METHOD(EventBuffer, search)
{
	zval               *zbuf      = getThis();
	zend_long           start_pos = -1;
	zend_long           end_pos   = -1;
	char               *what;
	size_t              what_len;
	php_event_buffer_t *b;

	struct evbuffer_ptr ptr_start, ptr_end, ptr_res;


	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|ll",
				&what, &what_len,
				&start_pos,
				&end_pos) == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	if (start_pos != -1
			&& _get_pos(&ptr_start, start_pos, b->buf) == FAILURE) {
		start_pos = -1;
	}
	if (end_pos != -1 &&
		 	(end_pos > evbuffer_get_length(b->buf)
			 || _get_pos(&ptr_end, end_pos, b->buf) == FAILURE)) {
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

/* {{{ proto int|false EventBuffer::searchEol([int start = -1[, int eol_style = EventBuffer::EOL_ANY]]);
 * Searches for occurance of end of line.
 *
 * Returns unsigned numeric position on success. Otherwise -1.
 */
PHP_EVENT_METHOD(EventBuffer, searchEol)
{
	zval               *zbuf      = getThis();
	zend_long               start_pos = -1;
	zend_long               eol_style = EVBUFFER_EOL_ANY;
	php_event_buffer_t *b;

	struct evbuffer_ptr ptr_start, ptr_res;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|ll",
				&start_pos, &eol_style) == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	if (start_pos != -1
			&& _get_pos(&ptr_start, start_pos, b->buf) == FAILURE) {
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

/* {{{ proto ?string EventBuffer::pullup(int size);
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
PHP_EVENT_METHOD(EventBuffer, pullup)
{
	zval               *zbuf   = getThis();
	php_event_buffer_t *b;
	zend_long           size;
	unsigned char      *mem;
	size_t              length;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&size) == FAILURE) {
		return;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	mem = evbuffer_pullup(b->buf, size);

	if (mem == NULL) {
		RETURN_NULL();
	}

	length = evbuffer_get_length(b->buf);

	/* evbuffer_pullup() doesn't add terminating zero */
	mem[length] = '\0';

	RETVAL_STRINGL((const char *)mem, length);
}
/* }}} */

/* {{{ proto int|false EventBuffer::write(mixed fd[, int howmuch]);
 *
 * Write contents of the buffer to a file descriptor.
 * The buffer will be drained after the bytes have been successfully written.
 *
 * Returns the number of bytes written, or &false; on error.
 */
PHP_EVENT_METHOD(EventBuffer, write)
{
	zval                *zbuf  = getThis();
	php_event_buffer_t  *b;
	zval                *pzfd;
	evutil_socket_t      fd;
	zend_long                res;
	zend_long                howmuch = -1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|l",
				&pzfd, &howmuch) == FAILURE) {
		return;
	}

	fd = php_event_zval_to_fd(pzfd);
	if (fd == -1) {
		RETURN_FALSE;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

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

/* {{{ proto int|false EventBuffer::readFrom(mixed fd[, int howmuch]);
 *
 * Read data from a file descriptor onto the end of the buffer.
 *
 * Returns the number of bytes read, or &false; on error.
 */
PHP_EVENT_METHOD(EventBuffer, readFrom)
{
	zval                *zbuf = getThis();
	php_event_buffer_t  *b;
	zval                *pzfd;
	evutil_socket_t      fd;
	zend_long                res;
	zend_long                howmuch = -1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|l",
				&pzfd, &howmuch) == FAILURE) {
		return;
	}

	fd = php_event_zval_to_fd(pzfd);
	if (fd == -1) {
		RETURN_FALSE;
	}

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

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
PHP_EVENT_METHOD(EventBuffer, substr)
{
	struct evbuffer_ptr    ptr;
	zend_string           *str;
	zval                  *zbuf;
	php_event_buffer_t    *b;
	int                    n_chunks;
	int                    i;
	struct evbuffer_iovec *pv;
	zend_long              n_start;
	zend_long              n_length = -1;
	zend_long              n_read   = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|l",
				&n_start, &n_length) == FAILURE) {
		return;
	}

	zbuf = getThis();

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	if (_get_pos(&ptr, n_start, b->buf) == FAILURE) {
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

	str = zend_string_alloc(n_read, 0);

	for (n_read = 0, i = 0; i < n_chunks; ++i) {
		size_t len = pv[i].iov_len;

		if (n_read + len > n_length) {
			len = n_length - n_read;
		}

		memcpy(ZSTR_VAL(str) + n_read, pv[i].iov_base, len);

		n_read += len;
	}
	efree(pv);

	ZSTR_VAL(str)[n_read] = '\0';
	RETVAL_NEW_STR(str);
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
