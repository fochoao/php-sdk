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

extern zend_class_entry *php_event_dns_base_ce;

/* {{{ Private */

#define _ret_if_invalid_bevent_ptr(bev)             \
{                                                   \
    if (!bev->bevent) {                             \
        php_error_docref(NULL, E_WARNING, \
                "Buffer Event is not initialized"); \
        RETURN_FALSE; \
    }                                               \
}

/* {{{ bevent_rw_cb
 * Is called from the bufferevent read and write callbacks */
static zend_always_inline void bevent_rw_cb(struct bufferevent *bevent, php_event_bevent_t *bev, php_event_callback_t *pcb)
{

	zval              argv[2];
	zval              retval;
	php_event_base_t *b;
	zend_string      *func_name;
	zend_fcall_info   fci;
	zval              zcallable;

	PHP_EVENT_ASSERT(bev);
	PHP_EVENT_ASSERT(bevent);
	PHP_EVENT_ASSERT(bevent == bev->bevent);

	/* Protect against accidental destruction of the func name before zend_call_function() finished */
	ZVAL_COPY(&zcallable, &pcb->func_name);

	if (!zend_is_callable(&zcallable, 0, &func_name)) {
		zend_string_release(func_name);
		return;
	}
	zend_string_release(func_name);

#ifdef HAVE_EVENT_PTHREADS_LIB
	if (bevent) {
		bufferevent_lock(bevent);
	}
#endif
	if (Z_ISUNDEF(bev->self)) {
		ZVAL_NULL(&argv[0]);
	} else {
		ZVAL_COPY(&argv[0], &bev->self);
	}

	if (Z_ISUNDEF(bev->data)) {
		ZVAL_NULL(&argv[1]);
	} else {
		ZVAL_COPY(&argv[1], &bev->data);
	}

	zend_fcall_info_init(&zcallable, 0, &fci, &pcb->fci_cache, NULL, NULL);
	fci.retval = &retval;
	fci.params = argv;
	fci.param_count = 2;

	if (zend_call_function(&fci, &pcb->fci_cache) == SUCCESS) {
		if (!Z_ISUNDEF(retval)) {
			zval_ptr_dtor(&retval);
		}
	} else {
		if (EG(exception)) {
			PHP_EVENT_ASSERT(!Z_ISUNDEF(bev->base));
			b = Z_EVENT_BASE_OBJ_P(&bev->base);
			event_base_loopbreak(b->base);

			if (!Z_ISUNDEF(argv[0])) {
				zval_ptr_dtor(&argv[0]);
			}
		} else {
			php_error_docref(NULL, E_WARNING, "Failed to invoke bufferevent callback");
		}
	}

	zval_ptr_dtor(&zcallable);

	if (!Z_ISUNDEF(argv[0])) {
		zval_ptr_dtor(&argv[0]);
	}

#ifdef HAVE_EVENT_PTHREADS_LIB
	if (bevent) {
		bufferevent_unlock(bevent);
	}
#endif

	if (!Z_ISUNDEF(argv[1])) {
		zval_ptr_dtor(&argv[1]);
	}
}
/* }}} */

static void bevent_read_cb(struct bufferevent *bevent, void *ptr)/*{{{*/
{
	php_event_bevent_t *bev = (php_event_bevent_t *)ptr;
	bevent_rw_cb(bevent, bev, &bev->cb_read);
}/*}}}*/

static void bevent_write_cb(struct bufferevent *bevent, void *ptr)/*{{{*/
{
	php_event_bevent_t *bev = (php_event_bevent_t *)ptr;
	bevent_rw_cb(bevent, bev, &bev->cb_write);
}/*}}}*/

/* {{{ bevent_event_cb */
static void bevent_event_cb(struct bufferevent *bevent, short events, void *ptr)
{
	php_event_bevent_t *bev       = (php_event_bevent_t *)ptr;
	zend_fcall_info     fci;
	zval                argv[3];
	zval                retval;
	php_event_base_t   *b;
	zend_string        *func_name;
	zval                zcallable;

	PHP_EVENT_ASSERT(bevent);
	PHP_EVENT_ASSERT(bev->bevent == bevent);

	/* Protect against accidental destruction of the func name before zend_call_function() finished */
	ZVAL_COPY(&zcallable, &bev->cb_event.func_name);

	if (!zend_is_callable(&zcallable, 0, &func_name)) {
		zend_string_release(func_name);
		return;
	}
	zend_string_release(func_name);

#ifdef HAVE_EVENT_PTHREADS_LIB
	if (bevent) {
		bufferevent_lock(bevent);
	}
#endif

	if (Z_ISUNDEF(bev->self)) {
		ZVAL_NULL(&argv[0]);
	} else {
		ZVAL_COPY(&argv[0], &bev->self);
	}

	ZVAL_LONG(&argv[1], events);

	if (Z_ISUNDEF(bev->data)) {
		ZVAL_NULL(&argv[2]);
	} else {
		ZVAL_COPY(&argv[2], &bev->data);
	}

	zend_fcall_info_init(&zcallable, 0, &fci, &bev->cb_event.fci_cache, NULL, NULL);
	fci.retval = &retval;
	fci.params = argv;
	fci.param_count = 3;

	if (zend_call_function(&fci, &bev->cb_event.fci_cache) == SUCCESS) {
		if (!Z_ISUNDEF(retval)) {
			zval_ptr_dtor(&retval);
		}
	} else {
		if (EG(exception)) {
			PHP_EVENT_ASSERT(!Z_ISUNDEF(bev->base));
			b = Z_EVENT_BASE_OBJ_P(&bev->base);
			event_base_loopbreak(b->base);

			if (!Z_ISUNDEF(argv[0])) {
				zval_ptr_dtor(&argv[0]);
			}
			if (!Z_ISUNDEF(argv[1])) {
				zval_ptr_dtor(&argv[1]);
			}
		} else {
			php_error_docref(NULL, E_WARNING, "Failed to invoke bufferevent event callback");
		}
	}

	zval_ptr_dtor(&zcallable);

	if (!Z_ISUNDEF(argv[0])) {
		zval_ptr_dtor(&argv[0]);
	}

#ifdef HAVE_EVENT_PTHREADS_LIB
	if (bevent) {
		bufferevent_unlock(bevent);
	}
#endif

	if (!Z_ISUNDEF(argv[1])) {
		zval_ptr_dtor(&argv[1]);
	}
	if (!Z_ISUNDEF(argv[2])) {
		zval_ptr_dtor(&argv[2]);
	}
}
/* }}} */

#ifdef HAVE_EVENT_OPENSSL_LIB
/* {{{ is_valid_ssl_state */
static zend_always_inline zend_bool is_valid_ssl_state(zend_long state)
{
	return (zend_bool) (state == BUFFEREVENT_SSL_OPEN
			|| state == BUFFEREVENT_SSL_CONNECTING
			|| state == BUFFEREVENT_SSL_ACCEPTING);
}
/* }}} */
#endif /* HAVE_EVENT_OPENSSL_LIB */

/* Private }}} */


/* {{{ proto EventBufferEvent EventBufferEvent::__construct(EventBase base[, mixed socket = NULL[, int options = 0[, callable readcb[, callable writecb[, callable eventcb[, mixed arg = NULL]]]]]]);
 *
 * Create a socket-based buffer event.
 * options is one of EventBufferEvent::OPT_* constants, or 0.
 * Passing NULL to socket parameter means that the socket stream should be created later,
 * e.g. by means of bufferevent_socket_connect().
 *
 * socket parameter may be created as a stream(not necessarily by means of sockets extension)
 *
 * Returns buffer event resource optionally associated with socket resource. */
PHP_EVENT_METHOD(EventBufferEvent, __construct)
{
	zval                 *zself     = getThis();
	zval                 *zbase;
	php_event_base_t     *base;
	zval                 *pzfd      = NULL;
	evutil_socket_t       fd;
	zend_long             options   = 0;
	php_event_bevent_t   *bev;
	struct bufferevent   *bevent;
	zend_fcall_info       fci_read = {0}, fci_write = {0}, fci_event = {0};
	zend_fcall_info_cache fcc_read, fcc_write, fcc_event;
	zval                 *zarg      = NULL;
	bufferevent_data_cb   read_cb;
	bufferevent_data_cb   write_cb;
	bufferevent_event_cb  event_cb;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O|z!lf!f!f!z!",
				&zbase, php_event_base_ce,
				&pzfd, &options,
				&fci_read,  &fcc_read,
				&fci_write, &fcc_write,
				&fci_event, &fcc_event,
				&zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	if (pzfd) {
		/* php_event_zval_to_fd reports error
	 	 * in case if it is not a valid socket resource */
		/*fd = (evutil_socket_t) php_event_zval_to_fd(pzfd);*/
		fd = php_event_zval_to_fd(pzfd);

		if (fd < 0) {
			return;
		}
		/* Make sure that the socket is in non-blocking mode(libevent's tip) */
		evutil_make_socket_nonblocking(fd);
	} else {
 		/* User decided to assign fd later,
 		 * e.g. by means of bufferevent_socket_connect()
 		 * which allocates new socket stream in this case. */
		fd = -1;
		/* User has no access to the file descriptor created
		 * internally(bufferevent_getfd is not exposed to userspace at the
		 * moment). Therefore, we have to make it close-on-free. */
		options |= BEV_OPT_CLOSE_ON_FREE;
	}

	base = Z_EVENT_BASE_OBJ_P(zbase);

	bev = Z_EVENT_BEVENT_OBJ_P(zself);

#ifdef HAVE_EVENT_PTHREADS_LIB
	options |= BEV_OPT_THREADSAFE;
#endif
	bevent = bufferevent_socket_new(base->base, fd, options);
	if (bevent == NULL) {
		php_error_docref(NULL, E_ERROR,
				"Failed to allocate bufferevent for socket");
		return;
	}
	bev->_internal = 0;
	bev->bevent = bevent;

	ZVAL_COPY_VALUE(&bev->self, zself);
	ZVAL_COPY(&bev->base, zbase);

	ZVAL_UNDEF(&bev->input);
	ZVAL_UNDEF(&bev->output);

	if (ZEND_FCI_INITIALIZED(fci_read)) {
		read_cb = bevent_read_cb;
		php_event_replace_callback(&bev->cb_read, &fci_read.function_name);
	} else {
		php_event_init_callback(&bev->cb_read);
		read_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_write)) {
		write_cb = bevent_write_cb;
		php_event_replace_callback(&bev->cb_write, &fci_write.function_name);
	} else {
		php_event_init_callback(&bev->cb_write);
		write_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_event)) {
		event_cb = bevent_event_cb;
		php_event_replace_callback(&bev->cb_event, &fci_event.function_name);
	} else {
		php_event_init_callback(&bev->cb_event);
		event_cb = NULL;
	}

	if (zarg) {
		ZVAL_COPY(&bev->data, zarg);
	} else {
		ZVAL_UNDEF(&bev->data);
	}

	if (read_cb || write_cb || event_cb || zarg) {
		bufferevent_setcb(bev->bevent, read_cb, write_cb, event_cb, (void *)bev);
	}
}
/* }}} */

/* {{{ proto void EventBufferEvent::free(void); */
PHP_EVENT_METHOD(EventBufferEvent, free)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);

	if (bev->bevent) {
#if 0
		bufferevent_lock(bev->bevent);
		bufferevent_disable(bev->bevent, EV_WRITE|EV_READ);
		bufferevent_setcb(bev->bevent, NULL, NULL, NULL, NULL);
		bufferevent_unlock(bev->bevent);
#endif

		if (!bev->_internal) {
			bufferevent_free(bev->bevent);
		}
		bev->bevent = 0;

#if 0
		/* Do it once */
		if (!Z_ISUNDEF(bev->self)) {
			zval_ptr_dtor(&bev->self);
			ZVAL_UNDEF(&bev->self);
		}
#else
		if (bev->_internal && !Z_ISUNDEF(bev->self)) {
			zval_ptr_dtor(&bev->self);
			ZVAL_UNDEF(&bev->self);
		}
#endif
		if (!Z_ISUNDEF(bev->base)) {
			Z_TRY_DELREF(bev->base);
			ZVAL_UNDEF(&bev->base);
		}
	}
}
/* }}} */

/* {{{ proto bool EventBufferEvent::close(void); */
PHP_EVENT_METHOD(EventBufferEvent, close)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	evutil_socket_t     fd;

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);

	if (bev->bevent) {
		fd = bufferevent_getfd(bev->bevent);
		if (fd != -1) {
			if (evutil_closesocket(fd) != -1) {
				RETURN_TRUE;
			}
		}
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ proto ?array EventBufferEvent::createPair(EventBase base[, int options = 0]);
 *
 * options is one of EVENT_BEV_OPT_* constants, or 0.
 *
 * Returns array of two EventBufferEvent objects connected to each other.
 * All the usual options are supported, except for EVENT_BEV_OPT_CLOSE_ON_FREE,
 * which has no effect, and EVENT_BEV_OPT_DEFER_CALLBACKS, which is always on.
 */
PHP_EVENT_METHOD(EventBufferEvent, createPair)
{
	zval               *zbase;
	php_event_base_t   *base;
	zend_long               options        = 0;
	zval               zbev[2];
	php_event_bevent_t *b[2];
	struct bufferevent *bevent_pair[2];
	int                 i;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O|l",
				&zbase, php_event_base_ce, &options) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	base = Z_EVENT_BASE_OBJ_P(zbase);

	if (bufferevent_pair_new(base->base, options, bevent_pair)) {
		RETURN_FALSE;
	}

	array_init(return_value);

	for (i = 0; i < 2; i++) {
		PHP_EVENT_INIT_CLASS_OBJECT(&zbev[i], php_event_bevent_ce);
		b[i] = Z_EVENT_BEVENT_OBJ_P(&zbev[i]);
		b[i]->bevent = bevent_pair[i];

		ZVAL_COPY(&b[i]->self, &zbev[i]);
		ZVAL_COPY(&b[i]->base, zbase);

		add_next_index_zval(return_value, &zbev[i]);
	}
}
/* }}} */

/* {{{ proto bool EventBufferEvent::connect(string addr);
 *
 * Connect buffer event's socket to given address(optionally with port).
 *
 * addr parameter expected to be whether an IP address with optional port number,
 * or a path to UNIX domain socket.
 * Recognized formats are:
 *
 *    [IPv6Address]:port
 *    [IPv6Address]
 *    IPv6Address
 *    IPv4Address:port
 *    IPv4Address
 *    unix:path-to-socket-file
 */
PHP_EVENT_METHOD(EventBufferEvent, connect)
{
	php_event_bevent_t      *bev;
	zval                    *zbevent  = getThis();
	char                    *addr;
	size_t                   addr_len;
	struct sockaddr_storage  ss;
	int                      ss_len   = sizeof(ss);

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s",
				&addr, &addr_len) == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	ZEND_SECURE_ZERO(&ss, sizeof(ss));

#ifdef AF_UNIX
	if (strncasecmp(addr, PHP_EVENT_SUN_PREFIX,
				sizeof(PHP_EVENT_SUN_PREFIX) - 1) == 0) {
		/* UNIX domain socket path */

		struct sockaddr_un *s_un;

		s_un             = (struct sockaddr_un *) &ss;
		s_un->sun_family = AF_UNIX;
		ss_len          = sizeof(struct sockaddr_un);

		strcpy(s_un->sun_path, addr + sizeof(PHP_EVENT_SUN_PREFIX) - 1);

	} else
#endif
		if (evutil_parse_sockaddr_port(addr, (struct sockaddr *) &ss, &ss_len)) {
			/* Numeric addresses only. Don't try to resolve hostname. */

			php_error_docref(NULL, E_WARNING,
					"Failed parsing address: the address is not well-formed, "
					"or the port is out of range");
			RETURN_FALSE;
		}

	/* bufferevent_socket_connect() allocates a socket stream internally, if we
	 * didn't provide the file descriptor to the bufferevent before, e.g. with
	 * bufferevent_socket_new() */
	if (bufferevent_socket_connect(bev->bevent, (struct sockaddr *) &ss, ss_len)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBufferEvent::connectHost(?EventDnsBase dns_base, string hostname, int port[, int family = EventUtil::AF_UNSPEC]);
 *
 * Resolves the DNS name hostname, looking for addresses of type
 * family(EVENT_AF_* constants). If the name resolution fails, it invokes the
 * event callback with an error event. If it succeeds, it launches a connection
 * attempt just as bufferevent_socket_connect would.
 *
 * dns_base is optional. May be NULL, or a resource created with
 * event_dns_base_new()(requires --with-event-extra configure option).
 * For asyncronous hostname resolving pass a valid event dns base resource.
 * Otherwise the hostname resolving will block.
 *
 * Recognized hostname formats are:
 * www.example.com (hostname)
 * 1.2.3.4 (ipv4address)
 * ::1 (ipv6address)
 * [::1] ([ipv6address])
 */
PHP_EVENT_METHOD(EventBufferEvent, connectHost)
{
#if LIBEVENT_VERSION_NUMBER < 0x02000300
	PHP_EVENT_LIBEVENT_VERSION_REQUIRED(bufferevent_socket_connect_hostname, 2.0.3-alpha);
	RETVAL_FALSE;
#else
	php_event_bevent_t *bev;
	zval               *zbevent      = getThis();
	char               *hostname;
	size_t              hostname_len;
	zend_long           port;
	zend_long           family       = AF_UNSPEC;
#ifdef HAVE_EVENT_EXTRA_LIB
	php_event_dns_base_t *dnsb;
#endif

#ifdef HAVE_EVENT_EXTRA_LIB
	zval *zdns_base    = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O!sl|l",
				&zdns_base, php_event_dns_base_ce, &hostname, &hostname_len,
				&port, &family) == FAILURE) {
		return;
	}
#else
	zval *zunused;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "zsl|l",
				&zunused, &hostname, &hostname_len,
				&port, &family) == FAILURE) {
		return;
	}
#endif

	if (family & ~(AF_INET | AF_INET6 | AF_UNSPEC)) {
		php_error_docref(NULL, E_WARNING,
				"Invalid address family specified");
		RETURN_FALSE;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	/* bufferevent_socket_connect() allocates a socket stream internally, if we
	 * didn't provide the file descriptor to the bufferevent before, e.g. with
	 * bufferevent_socket_new() */

#ifdef HAVE_EVENT_EXTRA_LIB
	if (zdns_base) {
		dnsb = Z_EVENT_DNS_BASE_OBJ_P(zdns_base);
	}

	if (bufferevent_socket_connect_hostname(bev->bevent,
				(zdns_base ? dnsb->dns_base : NULL),
				family, hostname, port)) {
# ifdef PHP_EVENT_DEBUG
		php_error_docref(NULL, E_WARNING, "%s",
				evutil_gai_strerror(bufferevent_socket_get_dns_error(bev->bevent)));
# endif
		RETURN_FALSE;
	}
#else /* don't HAVE_EVENT_EXTRA_LIB */
	if (bufferevent_socket_connect_hostname(bev->bevent,
				NULL,
				family, hostname, port)) {
# ifdef PHP_EVENT_DEBUG
		php_error_docref(NULL, E_WARNING, "%s",
				evutil_gai_strerror(bufferevent_socket_get_dns_error(bev->bevent)));
# endif
		RETURN_FALSE;
	}
#endif

	RETVAL_TRUE;
#endif
}
/* }}} */

/* {{{ proto string EventBufferEvent::getDnsErrorString(void);
 * Returns string describing the last failed DNS lookup attempt made by
 * bufferevent_socket_connect_hostname(), or an empty string, if no DNS error
 * detected. */
PHP_EVENT_METHOD(EventBufferEvent, getDnsErrorString)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	int                 err;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	err = bufferevent_socket_get_dns_error(bev->bevent);

	if (err == 0) {
		RETURN_EMPTY_STRING();
	}
	RETVAL_STRING(evutil_gai_strerror(err));
}
/* }}} */

/* {{{ proto void EventBufferEvent::setCallbacks(callable readcb, callable writecb, callable eventcb[, mixed arg = NULL]);
 * Changes one or more of the callbacks of a bufferevent.
 * A callback may be disabled by passing NULL instead of the callable.
 * arg is an argument passed to the callbacks.
 */
PHP_EVENT_METHOD(EventBufferEvent, setCallbacks)
{
	zval                 *zbevent   = getThis();
	php_event_bevent_t   *bev;
	zend_fcall_info       fci_read = {0}, fci_write = {0}, fci_event = {0};
	zend_fcall_info_cache fcc_read, fcc_write, fcc_event;
	zval                 *zarg      = NULL;
	bufferevent_data_cb   read_cb;
	bufferevent_data_cb   write_cb;
	bufferevent_event_cb  event_cb;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "f!f!f!|z!",
				&fci_read,  &fcc_read,
				&fci_write, &fcc_write,
				&fci_event, &fcc_event,
				&zarg) == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	if (ZEND_FCI_INITIALIZED(fci_read)) {
		read_cb = bevent_read_cb;
		php_event_replace_callback(&bev->cb_read, &fci_read.function_name);
	} else {
		php_event_free_callback(&bev->cb_read);
		read_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_write)) {
		write_cb = bevent_write_cb;
		php_event_replace_callback(&bev->cb_write, &fci_write.function_name);
	} else {
		php_event_free_callback(&bev->cb_write);
		write_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_event)) {
		event_cb = bevent_event_cb;
		php_event_replace_callback(&bev->cb_event, &fci_event.function_name);
	} else {
		php_event_free_callback(&bev->cb_event);
		event_cb = NULL;
	}

	php_event_replace_zval(&bev->data, zarg);

	bufferevent_setcb(bev->bevent, read_cb, write_cb, event_cb, (void *)bev);
}
/* }}} */

/* {{{ proto bool EventBufferEvent::enable(int events);
 * Enable events EVENT_READ, EVENT_WRITE, or EVENT_READ | EVENT_WRITE on a buffer event. */
PHP_EVENT_METHOD(EventBufferEvent, enable)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	zend_long               events;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&events) == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	if (bufferevent_enable(bev->bevent, events)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBufferEvent::disable(int events);
 * Disable events EVENT_READ, EVENT_WRITE, or EVENT_READ | EVENT_WRITE on a buffer event. */
PHP_EVENT_METHOD(EventBufferEvent,disable)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	zend_long               events;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&events) == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	if (bufferevent_disable(bev->bevent, events)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int EventBufferEvent::getEnabled(void);
 * Returns bitmask of events currently enabled on the buffer event. */
PHP_EVENT_METHOD(EventBufferEvent, getEnabled)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);


	RETVAL_LONG(bufferevent_get_enabled(bev->bevent));
}
/* }}} */

/* {{{ proto EventBuffer EventBufferEvent::getInput(void);
 *
 * Returns an input EventBuffer object associated with the buffer event */
PHP_EVENT_METHOD(EventBufferEvent, getInput)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_buffer_ce);
	b = Z_EVENT_BUFFER_OBJ_P(return_value);
	/* Don't do this. It's normal to have refcount = 1 here.
	 * If we got bugs, we most likely free'd an internal buffer somewhere
	 * Z_TRY_ADDREF_P(return_value);*/

	b->buf      = bufferevent_get_input(bev->bevent);
	b->internal = 1;
}
/* }}} */

/* {{{ proto EventBuffer EventBufferEvent::getOutput(void);
 *
 * Returns an output EventBuffer object associated with the buffer event */
PHP_EVENT_METHOD(EventBufferEvent, getOutput)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_buffer_ce);
	b = Z_EVENT_BUFFER_OBJ_P(return_value);
	/* Don't do this. It's normal to have refcount = 1 here.
	 * If we got bugs, we most likely free'd an internal buffer somewhere
	 * Z_TRY_ADDREF_P(return_value);*/

	b->buf      = bufferevent_get_output(bev->bevent);
	b->internal = 1;
}
/* }}} */

/* {{{ proto void EventBufferEvent::setWatermark(int events, int lowmark, int highmark);
 * Adjusts the read watermarks, the write watermarks, or both, of a single bufferevent. */
PHP_EVENT_METHOD(EventBufferEvent, setWatermark)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	zend_long               events;
	zend_long               lowmark;
	zend_long               highmark;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lll",
				&events, &lowmark, &highmark) == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	bufferevent_setwatermark(bev->bevent, events, (size_t) lowmark, (size_t) highmark);
}
/* }}} */

/* {{{ proto bool EventBufferEvent::write(string data);
 * Adds `data' to a buffer event's output buffer. */
PHP_EVENT_METHOD(EventBufferEvent, write)
{
	zval               *zbevent  = getThis();
	php_event_bevent_t *bev;
	char               *data;
	size_t              data_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &data, &data_len) == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	if (bufferevent_write(bev->bevent, data, data_len)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBufferEvent::writeBuffer(EventBuffer buf);
 * Adds contents of the entire buffer to a buffer event's output buffer. */
PHP_EVENT_METHOD(EventBufferEvent, writeBuffer)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O",
				&zbuf, php_event_buffer_ce) == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	if (bufferevent_write_buffer(bev->bevent, b->buf)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto ?string EventBufferEvent::read(int size);
 * Removes up to size bytes from the input buffer, storing them into the memory at data.
 */
PHP_EVENT_METHOD(EventBufferEvent, read)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	zend_long           size;
	char               *data;
	zend_long           ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&size) == FAILURE) {
		return;
	}

	if (size < 0) {
		/*php_error_docref(NULL, E_WARNING, "Size must be positive");*/
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	data = safe_emalloc(size, sizeof(char), 1);

	ret = bufferevent_read(bev->bevent, data, size);

	if (ret > 0) {
		RETVAL_STRINGL(data, ret);
	} else {
		RETVAL_NULL();
	}

	efree(data);
}
/* }}} */

/* {{{ proto bool EventBufferEvent::readBuffer(EventBuffer buf);
 * Drains the entire contents of the input buffer and places them into buf */
PHP_EVENT_METHOD(EventBufferEvent, readBuffer)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O",
				&zbuf, php_event_buffer_ce) == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	b = Z_EVENT_BUFFER_OBJ_P(zbuf);

	if (bufferevent_read_buffer(bev->bevent, b->buf)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBufferEvent::setPriority(int priority);
 * Assign a priority to a bufferevent.
 * Only supported for socket bufferevents. */
PHP_EVENT_METHOD(EventBufferEvent, setPriority)
{
	zval               *zbevent  = getThis();
	php_event_bevent_t *bev;
	zend_long               priority;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&priority) == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	if (bufferevent_priority_set(bev->bevent, priority)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBufferEvent::setTimeouts(double timeout_read, double timeout_write);
 * Set the read and write timeout for a bufferevent. */
PHP_EVENT_METHOD(EventBufferEvent, setTimeouts)
{
	zval               *zbevent       = getThis();
	php_event_bevent_t *bev;
	double              timeout_read;
	double              timeout_write;
	struct timeval      tv_read;
	struct timeval      tv_write;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "dd",
				&timeout_read, &timeout_write) == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	PHP_EVENT_TIMEVAL_SET(tv_read, timeout_read);
	PHP_EVENT_TIMEVAL_SET(tv_write, timeout_write);

	if (bufferevent_set_timeouts(bev->bevent, &tv_read, &tv_write)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

#ifdef HAVE_EVENT_OPENSSL_LIB /* {{{ */

/* {{{ proto EventBufferEvent EventBufferEvent::createSslFilter(EventBufferEvent underlying, EventSslContext ctx, int state[, int options = 0]);
 */
PHP_EVENT_METHOD(EventBufferEvent, createSslFilter)
{
	php_event_base_t        *base;
	zval                    *zunderlying;
	php_event_bevent_t      *bev_underlying;
	zval                    *zctx;
	php_event_ssl_context_t *ectx;
	zend_long                state;
	zend_long                options        = 0;
	php_event_bevent_t      *bev;
	struct bufferevent      *bevent;
	SSL                     *ssl;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "OOl|l",
				&zunderlying, php_event_bevent_ce,
				&zctx, php_event_ssl_context_ce,
				&state, &options) == FAILURE) {
		return;
	}

	if (!is_valid_ssl_state(state)) {
		php_error_docref(NULL, E_WARNING, "Invalid state specified");
		RETURN_FALSE;
	}

	bev_underlying = Z_EVENT_BEVENT_OBJ_P(zunderlying);
	_ret_if_invalid_bevent_ptr(bev_underlying);

	/* Must also be the base for the underlying bufferevent. See Libevent reference. */
	base = Z_EVENT_BASE_OBJ_P(&bev_underlying->base);

	ectx = Z_EVENT_SSL_CONTEXT_OBJ_P(zctx);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_bevent_ce);
	bev = Z_EVENT_BEVENT_OBJ_P(return_value);

	if (UNEXPECTED(ectx->ctx == NULL)) {
		RETURN_FALSE;
	}
	ssl = SSL_new(ectx->ctx);
	if (UNEXPECTED(!ssl)) {
		php_error_docref(NULL, E_WARNING, "Event: Failed creating SSL handle");
		RETURN_FALSE;
	}
	/* Attach ectx to ssl for callbacks */
	SSL_set_ex_data(ssl, php_event_ssl_data_index, ectx);

#ifdef HAVE_EVENT_PTHREADS_LIB
	options |= BEV_OPT_THREADSAFE;
#endif
	bevent = bufferevent_openssl_filter_new(base->base,
			bev_underlying->bevent,
			ssl, state, options);
	if (bevent == NULL) {
		php_error_docref(NULL, E_WARNING, "Failed to allocate bufferevent filter");
		RETURN_FALSE;
	}
	bev->bevent = bevent;

	ZVAL_COPY_VALUE(&bev->self, return_value);
	ZVAL_COPY(&bev->base, &bev_underlying->base);

	ZVAL_UNDEF(&bev->input);
	ZVAL_UNDEF(&bev->output);
	ZVAL_UNDEF(&bev->data);
}
/* }}} */

/* {{{ proto EventBufferEvent EventBufferEvent::sslSocket(EventBase base, mixed socket, EventSslContext ctx, int state[, int options = 0]);
 * */
PHP_EVENT_METHOD(EventBufferEvent, sslSocket)
{
	zval                     *zbase;
	php_event_base_t         *base;
	zval                     *zctx;
	php_event_ssl_context_t  *ectx;
	zval                     *pzfd;
	evutil_socket_t           fd;
	zend_long                     state;
	zend_long                     options = 0;
	php_event_bevent_t       *bev;
	struct bufferevent       *bevent;
	SSL                      *ssl;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "OzOl|l",
				&zbase, php_event_base_ce,
				&pzfd,
				&zctx, php_event_ssl_context_ce,
				&state, &options) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	if (!is_valid_ssl_state(state)) {
		php_error_docref(NULL, E_WARNING,
				"Invalid state specified");
		RETURN_FALSE;
	}

	base = Z_EVENT_BASE_OBJ_P(zbase);
	ectx = Z_EVENT_SSL_CONTEXT_OBJ_P(zctx);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_bevent_ce);
	bev = Z_EVENT_BEVENT_OBJ_P(return_value);

	if (Z_TYPE_P(pzfd) == IS_NULL) {
		/* User decided to set fd later via connect or connectHost etc.*/
		fd = -1;
	} else {
		fd = php_event_zval_to_fd(pzfd);
		if (fd < 0) {
			RETURN_FALSE;
		}
		/* Make sure that the socket is in non-blocking mode(libevent's tip) */
		/*evutil_make_socket_nonblocking(fd);*/
	}

	PHP_EVENT_ASSERT(ectx->ctx);
	ssl = SSL_new(ectx->ctx);
	if (!ssl) {
		php_error_docref(NULL, E_WARNING,
				"Event: Failed creating SSL handle");
		RETURN_FALSE;
	}
	/* Attach ectx to ssl for callbacks */
	SSL_set_ex_data(ssl, php_event_ssl_data_index, ectx);

#ifdef HAVE_EVENT_PTHREADS_LIB
	options |= BEV_OPT_THREADSAFE;
#endif
	bevent = bufferevent_openssl_socket_new(base->base, fd, ssl, state, options);
	if (bevent == NULL) {
		php_error_docref(NULL, E_ERROR,
				"Failed to allocate bufferevent filter");
		RETURN_FALSE;
	}
	bev->bevent = bevent;

	ZVAL_COPY_VALUE(&bev->self, return_value);
	ZVAL_COPY(&bev->base, zbase);
}
/* }}} */

/* {{{ proto string EventBufferEvent::sslError(void);
 *
 * Returns most recent OpenSSL error reported on the buffer event. The function
 * returns FALSE, if there is no more error to return. */
PHP_EVENT_METHOD(EventBufferEvent, sslError)
{
	zval               *zbevent  = getThis();
	php_event_bevent_t *bev;
	char                buf[512];
	zend_ulong          e;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	e = bufferevent_get_openssl_error(bev->bevent);
	if (e) {
		RETURN_STRING(ERR_error_string(e, buf));
	}

	RETVAL_FALSE;
}
/* }}} */

/* {{{ proto void EventBufferEvent::sslRenegotiate(void);
 *
 * Tells a bufferevent to begin SSL renegotiation.
 *
 * Warning. Calling this function tells the SSL to renegotiate, and the
 * bufferevent to invoke appropriate callbacks. This is an advanced topic; you
 * should generally avoid it unless you really know what you’re doing,
 * especially since many SSL versions have had known security issues related to
 * renegotiation.
 **/
PHP_EVENT_METHOD(EventBufferEvent, sslRenegotiate)
{
	zval               *zbevent  = getThis();
	php_event_bevent_t *bev;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	bufferevent_ssl_renegotiate(bev->bevent);
}
/* }}} */

/* {{{ proto string EventBufferEvent::sslGetCipherInfo(void);
 *
 * Returns the current Cipher of the connexion as
 * SSL_get_current_cipher/SSL_CIPHER_description do. Otherwise FALSE. */
PHP_EVENT_METHOD(EventBufferEvent, sslGetCipherInfo)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	struct ssl_st      *ssl;
	char               *desc;
	const SSL_CIPHER   *cipher;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	ssl = bufferevent_openssl_get_ssl(bev->bevent);
	if (ssl) {
		cipher = SSL_get_current_cipher(ssl);
		if (cipher) {
			desc = SSL_CIPHER_description(cipher, NULL, 128);
			RETVAL_STRING(desc);
			OPENSSL_free(desc);
			return;
		}
	}

	RETVAL_FALSE;
}
/* }}} */

/* {{{ proto string EventBufferEvent::sslGetCipherName(void);
 *
 * Returns the current Cipher Name of the connection as SSL_get_cipher_name does.
 * returns FALSE, if there is no more error to return. */
PHP_EVENT_METHOD(EventBufferEvent, sslGetCipherName)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	struct ssl_st      *ssl;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	ssl = bufferevent_openssl_get_ssl(bev->bevent);
	if (ssl) {
		RETURN_STRING(SSL_get_cipher_name(ssl));
	}
	RETVAL_FALSE;
}
/* }}} */

/* {{{ proto string EventBufferEvent::sslGetCipherVersion(void);
 *
 * Returns the current cipher version of the connection as SSL_get_cipher_version does.
 * returns FALSE, if there is no more error to return. */
PHP_EVENT_METHOD(EventBufferEvent, sslGetCipherVersion)
{
	zval               *zbevent  = getThis();
	php_event_bevent_t *bev;
	struct ssl_st      *ssl;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	ssl = bufferevent_openssl_get_ssl(bev->bevent);
	if (ssl) {
		RETURN_STRING(SSL_get_cipher_version(ssl));
	}
	RETVAL_FALSE;
}
/* }}} */

/* {{{ proto string EventBufferEvent::sslGetProtocol(void);
 *
 * Returns the current Protocol of the connection as SSL_get_version does,
 * otherwise FALSE. */
PHP_EVENT_METHOD(EventBufferEvent, sslGetProtocol)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	struct ssl_st      *ssl;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	bev = Z_EVENT_BEVENT_OBJ_P(zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	ssl = bufferevent_openssl_get_ssl(bev->bevent);
	if (ssl) {
		RETURN_STRING(SSL_get_version(ssl));
	}
	RETVAL_FALSE;
}
/* }}} */

#endif /* HAVE_EVENT_OPENSSL_LIB }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
