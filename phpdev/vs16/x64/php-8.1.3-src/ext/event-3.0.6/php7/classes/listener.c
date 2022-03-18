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

#define _ret_if_invalid_listener_ptr(l) do {         \
    PHP_EVENT_ASSERT(l && l->listener);              \
    if (!l->listener) {                              \
        php_error_docref(NULL, E_WARNING,            \
                "EventListener is not initialized"); \
        RETURN_FALSE;                                \
    }                                                \
} while (0)

/* {{{ sockaddr_parse
 * Parse in_addr and fill out_arr with IP and port.
 * out_arr must be a pre-allocated empty zend array */
static int sockaddr_parse(const struct sockaddr *in_addr, zval *out_zarr)
{
	char buf[256];
	int  ret      = FAILURE;

	PHP_EVENT_ASSERT(Z_TYPE_P(out_zarr) == IS_ARRAY);

	switch (in_addr->sa_family) {
		case AF_INET:
			if (evutil_inet_ntop(in_addr->sa_family, &((struct sockaddr_in *) in_addr)->sin_addr,
						(void *) &buf, sizeof(buf))) {
				add_next_index_string(out_zarr, (char *)&buf);
				add_next_index_long(out_zarr,
						ntohs(((struct sockaddr_in *) in_addr)->sin_port));

				ret = SUCCESS;
			}
			break;
#if HAVE_IPV6
		case AF_INET6:
			if (evutil_inet_ntop(in_addr->sa_family, &((struct sockaddr_in6 *) in_addr)->sin6_addr,
						(void *) &buf, sizeof(buf))) {
				add_next_index_string(out_zarr, (char *)&buf);
				add_next_index_long(out_zarr,
						ntohs(((struct sockaddr_in6 *) in_addr)->sin6_port));

				ret = SUCCESS;
			}
			break;
#endif
#ifdef AF_UNIX
		case AF_UNIX:
			{
				struct sockaddr_un *ua = (struct sockaddr_un *) in_addr;

				if (ua->sun_path[0] == '\0') {
					/* abstract name */
 					zval tmp;
					int len;

					len = strlen(ua->sun_path + 1) + 1;
					ZVAL_STRINGL(&tmp, ua->sun_path, len);
					add_next_index_zval(out_zarr, &tmp);
				} else {
					add_next_index_string(out_zarr, ua->sun_path);
				}
				add_next_index_long(out_zarr, 0);
			}
			break;
#endif
	}

	return ret;
}
/* }}} */

/* {{{ _php_event_listener_cb */
static void _php_event_listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx) {
	php_event_listener_t *l       = (php_event_listener_t *)ctx;
	zend_fcall_info       fci;
	zval                  argv[4];
	zval                  retval;
	zend_string     *func_name;
	zval                  zcallable;

	PHP_EVENT_ASSERT(l);

	/* Protect against accidental destruction of the func name before zend_call_function() finished */
	ZVAL_COPY(&zcallable, &l->cb.func_name);

	if (!zend_is_callable(&zcallable,  IS_CALLABLE_STRICT, &func_name)) {
		zend_string_release(func_name);
		return;
	}
	zend_string_release(func_name);

	/* Call user function having proto:
	 * void cb (EventListener $listener, resource $fd, array $address, mixed $data);
	 * $address = array ("IP-address", port)
	 */

	ZVAL_COPY(&argv[0], &l->self);

	/* Always create new resource, since every new connection creates new fd.
	 * We are in the accept-connection callback now. */
	/*
	 * We might convert it to stream. But likely nobody wants it for any
	 * purpose than passing back to event, e.g. to
	 * EventBufferEvent::__construct
	 *
	 * php_stream *stream;
	 * stream = php_stream_fopen_from_fd(fd, "r", NULL);
	 * php_stream_to_zval(stream, arg_fd);
	 *
	 * Thus, we're just passing numeric fd here.
	 */
	if (fd) {
		ZVAL_LONG(&argv[1], fd);
	} else {
		ZVAL_NULL(&argv[1]);
	}

	/* A client connected via UNIX domain can't be bound to the socket.
	 * I.e. the socket is most likely abstract(unnamed), and has no sense here. */
#ifdef AF_UNIX
	if (address->sa_family == AF_UNIX) {
		ZVAL_NULL(&argv[2]);
	} else {
		array_init(&argv[2]);
		sockaddr_parse(address, &argv[2]);
	}
#else
	array_init(&argv[2]);
	sockaddr_parse(address, &argv[2]);
#endif

	if (Z_ISUNDEF(l->data)) {
		ZVAL_NULL(&argv[3]);
	} else {
		ZVAL_COPY(&argv[3], &l->data);
	}

	fci.size = sizeof(fci);
#ifdef HAVE_PHP_ZEND_FCALL_INFO_FUNCTION_TABLE
	fci.function_table = EG(function_table);
#endif
	ZVAL_COPY_VALUE(&fci.function_name, &zcallable);
	fci.object = NULL;
	fci.retval = &retval;
	fci.params = argv;
	fci.param_count = 4;
	fci.no_separation  = 1;
#ifdef HAVE_PHP_ZEND_FCALL_INFO_SYMBOL_TABLE
	fci.symbol_table = NULL;
#endif

	if (zend_call_function(&fci, &l->cb.fci_cache) == SUCCESS) {
		if (!Z_ISUNDEF(retval)) {
			zval_ptr_dtor(&retval);
		}
	} else {
		php_error_docref(NULL, E_WARNING, "Failed to invoke listener callback");
	}

	zval_ptr_dtor(&zcallable);

	zval_ptr_dtor(&argv[0]);
	zval_ptr_dtor(&argv[1]);
	zval_ptr_dtor(&argv[2]);
	zval_ptr_dtor(&argv[3]);
}
/* }}} */

/* {{{ listener_error_cb */
static void listener_error_cb(struct evconnlistener *listener, void *ctx) {
	php_event_listener_t *l         = (php_event_listener_t *)ctx;
	zend_fcall_info       fci;
	zval                  argv[2];
	zval                  retval;
	zend_string          *func_name;
	zval                  zcallable;

	PHP_EVENT_ASSERT(l);

	/* Protect against accidental destruction of the func name before zend_call_function() finished */
	ZVAL_COPY(&zcallable, &l->cb_err.func_name);

	if (!zend_is_callable(&zcallable, IS_CALLABLE_STRICT, &func_name)) {
		zend_string_release(func_name);
		return;
	}
	zend_string_release(func_name);

	/* Call user function having proto:
	 * void cb (EventListener $listener, mixed $data); */

	ZVAL_COPY(&argv[0], &l->self);

	if (Z_ISUNDEF(l->data)) {
		ZVAL_NULL(&argv[1]);
	} else {
		ZVAL_COPY(&argv[1], &l->data);
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

	if (zend_call_function(&fci, &l->cb.fci_cache) == SUCCESS) {
		if (!Z_ISUNDEF(retval)) {
			zval_ptr_dtor(&retval);
		}
	} else {
		php_error_docref(NULL, E_WARNING, "Failed to invoke listener error callback");
	}

	zval_ptr_dtor(&zcallable);

	zval_ptr_dtor(&argv[0]);
	zval_ptr_dtor(&argv[1]);
}
/* }}} */

/* Private }}} */

/* {{{ proto EventListener EventListener::__construct(EventBase base, callable cb, mixed data, int flags, int backlog, mixed target);
 *
 * Creates new connection listener associated with an event base.
 *
 * target parameter may be string, socket resource, or a stream associated with a socket.
 * In case if target is a string, the string will be parsed as network address.
 * A path to UNIX domain socket should be prefixed with 'unix:', e.g.
 * unix:/tmp/my.sock.
 *
 * Returns object representing the event connection listener.
 */
PHP_METHOD(EventListener, __construct)
{
	struct evconnlistener *listener;
	zval                  *zself    = getThis();
	zval                  *zbase;
	zval                  *zcb;
	zval                  *zdata    = NULL;
	zval                  *pztarget;
	zend_long              flags;
	zend_long              backlog;
	php_event_base_t      *base;
	php_event_listener_t  *l;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Ozz!llz",
				&zbase, php_event_base_ce,
				&zcb, &zdata, &flags, &backlog, &pztarget) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	base = Z_EVENT_BASE_OBJ_P(zbase);
	PHP_EVENT_ASSERT(base);

	if (Z_TYPE_P(pztarget) == IS_STRING) {
		struct sockaddr_storage ss;
		socklen_t ss_len = sizeof(ss);
		ZEND_SECURE_ZERO(&ss, sizeof(ss));

#ifdef AF_UNIX
		if (strncasecmp(Z_STRVAL_P(pztarget), PHP_EVENT_SUN_PREFIX,
					sizeof(PHP_EVENT_SUN_PREFIX) - 1) == 0) {
			struct sockaddr_un *s_un;

			s_un             = (struct sockaddr_un *)&ss;
			s_un->sun_family = AF_UNIX;

			strcpy(s_un->sun_path, Z_STRVAL_P(pztarget) + sizeof(PHP_EVENT_SUN_PREFIX) - 1);
			ss_len = sizeof(struct sockaddr_un);
		} else
#endif
			if (php_network_parse_network_address_with_port(Z_STRVAL_P(pztarget),
						Z_STRLEN_P(pztarget), (struct sockaddr *)&ss, &ss_len) != SUCCESS) {
				zend_throw_exception_ex(zend_ce_exception, 0,
						"Failed to parse network address %s", Z_STRVAL_P(pztarget));
				return;
			}

		l = Z_EVENT_LISTENER_OBJ_P(zself);
		PHP_EVENT_ASSERT(l);

		listener = evconnlistener_new_bind(base->base, _php_event_listener_cb,
				(void *)l, flags, backlog, (struct sockaddr *)&ss, ss_len);
	} else { /* pztarget is not string */
		evutil_socket_t fd = -1;

		fd = php_event_zval_to_fd(pztarget);
		if (fd < 0) {
			return;
		}

		if (flags & ~LEV_OPT_LEAVE_SOCKETS_BLOCKING) {
			/* Make sure that the socket is in non-blocking mode(libevent's tip) */
			evutil_make_socket_nonblocking(fd);
		}

		l = Z_EVENT_LISTENER_OBJ_P(zself);

		listener = evconnlistener_new(base->base, _php_event_listener_cb,
				(void *) l, flags, backlog, fd);
	}

	if (!listener) {
		zend_throw_exception_ex(zend_ce_exception, 0, "Failed to allocate listener");
		return;
	}

	l->listener = listener;
	php_event_copy_zval(&l->data, zdata);
	php_event_copy_callback(&l->cb, zcb);
	ZVAL_COPY_VALUE(&l->self, zself);
}
/* }}} */

/*{{{ proto int EventListener::__sleep */
PHP_METHOD(EventListener, __sleep)
{
	zend_throw_exception_ex(php_event_get_exception(), 0, "EventListener instances are not serializable");
}
/*}}}*/

/*{{{ proto int EventListener::__wakeup */
PHP_METHOD(EventListener, __wakeup)
{
	zend_throw_exception_ex(php_event_get_exception(), 0, "EventListener instances are not serializable");
}
/*}}}*/

/*{{{ proto void EventListener::free(void); */
PHP_METHOD(EventListener, free)
{
	zval *self = getThis();
	php_event_listener_t *l;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	l = Z_EVENT_LISTENER_OBJ_P(self);

	if (l != NULL && l->listener != NULL) {
		evconnlistener_free(l->listener);
		l->listener = NULL;
	}
}/*}}}*/

/* {{{ proto bool EventListener::enable(void);
 * Enable an event connect listener resource */
PHP_METHOD(EventListener, enable)
{
	zval                 *zlistener = getThis();
	php_event_listener_t *l;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	l = Z_EVENT_LISTENER_OBJ_P(zlistener);
	_ret_if_invalid_listener_ptr(l);

	if (evconnlistener_enable(l->listener)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventListener::disable(void);
 *
 * Disable an event connect listener resource */
PHP_METHOD(EventListener, disable)
{
	zval                 *zlistener = getThis();
	php_event_listener_t *l;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	l = Z_EVENT_LISTENER_OBJ_P(zlistener);
	_ret_if_invalid_listener_ptr(l);

	if (evconnlistener_disable(l->listener)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto void EventListener::setCallback(callable cb[, mixed arg = NULL]);
 *
 * Adjust event connect listener's callback and optionally the callback argument.
 * Both cb and arg may be NULL.
 */
PHP_METHOD(EventListener, setCallback)
{
	php_event_listener_t *l;
	zval                 *zcb;
	zval                 *zarg      = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|z!", &zcb, &zarg) == FAILURE) {
		return;
	}

	l = Z_EVENT_LISTENER_OBJ_P(getThis());
	_ret_if_invalid_listener_ptr(l);

	php_event_replace_callback(&l->cb, zcb);
	php_event_replace_zval(&l->data, zarg);

	/*
	 * No sense in the following call, since the callback and the pointer
	 * remain the same
	 * evconnlistener_set_cb(l->listener, _php_event_listener_cb, (void *) l);
	 */
}
/* }}} */

/* {{{ proto void EventListener::setErrorCallback(callable cb);
 * Set event listener's error callback
 */
PHP_METHOD(EventListener, setErrorCallback)
{
	php_event_listener_t *l;
	zval                 *zcb;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &zcb) == FAILURE) {
		return;
	}

	l = Z_EVENT_LISTENER_OBJ_P(getThis());
	_ret_if_invalid_listener_ptr(l);

	php_event_replace_callback(&l->cb_err, zcb);

	/*
	 * No much sense in the following call, since the callback and the pointer
	 * remain the same. However, we have to set it once at least
	 */
	 evconnlistener_set_error_cb(l->listener, listener_error_cb);
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02000300
/* {{{ proto EventBase EventListener::getBase(void);
 * Get event base associated with the connection listener
 */
PHP_METHOD(EventListener, getBase)
{
	php_event_listener_t *l;
	zval                 *zlistener = getThis();
	php_event_base_t     *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	l = Z_EVENT_LISTENER_OBJ_P(zlistener);
	_ret_if_invalid_listener_ptr(l);

	/* base = evconnlistener_get_base(l->listener); */

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_base_ce);
	b = Z_EVENT_BASE_OBJ_P(return_value);
	/* Don't do this. It's normal to have refcount = 1 here.
	 * If we got bugs, we most likely free'd an internal buffer somewhere
	 * Z_TRY_ADDREF_P(return_value);*/

	b->base = evconnlistener_get_base(l->listener);
	b->internal = 1;
}
/* }}} */
#endif

/* {{{ proto bool EventListener::getSocketName(string &address[, int &port]);
 * Retreives the current address to which the listener's socket is bound.
 * Returns &true; on success. Otherwise &false;.*/
PHP_METHOD(EventListener, getSocketName)
{
	php_event_listener_t  *l;
	zval                  *zlistener = getThis();
	zval                  *zaddress;
	zval                  *zport     = NULL;
	evutil_socket_t        fd;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|z",
				&zaddress, &zport) == FAILURE) {
		return;
	}

	l = Z_EVENT_LISTENER_OBJ_P(zlistener);
	_ret_if_invalid_listener_ptr(l);

	fd = evconnlistener_get_fd(l->listener);
	if (fd <= 0) {
		RETURN_FALSE;
	}

	if (_php_event_getsockname(fd, zaddress, zport) == FAILURE) {
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
