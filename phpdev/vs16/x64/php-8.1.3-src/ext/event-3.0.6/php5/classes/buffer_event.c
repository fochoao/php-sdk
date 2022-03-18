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

extern const zend_function_entry php_event_dns_base_ce_functions[];
extern zend_class_entry *php_event_dns_base_ce;

/* {{{ Private */

#define _ret_if_invalid_bevent_ptr(bev)             \
{                                                   \
    if (!bev->bevent) {                             \
        php_error_docref(NULL TSRMLS_CC, E_WARNING, \
                "Buffer Event is not initialized"); \
        RETURN_FALSE; \
    }                                               \
}

/* {{{ bevent_rw_cb
 * Is called from the bufferevent read and write callbacks */
static zend_always_inline void bevent_rw_cb(struct bufferevent *bevent, php_event_bevent_t *bev, zend_fcall_info *pfci, zend_fcall_info_cache *pfcc)
{

	zval  *arg_data;
	zval  *arg_self;
	zval **args[2];
	zval  *retval_ptr = NULL;
	php_event_base_t *b;
	PHP_EVENT_TSRM_DECL

	PHP_EVENT_ASSERT(bev);
	PHP_EVENT_ASSERT(bevent);
	PHP_EVENT_ASSERT(bevent == bev->bevent);
	PHP_EVENT_ASSERT(pfci && pfcc);
	PHP_EVENT_ASSERT(bev->self);
	PHP_EVENT_ASSERT(bev->base);

	arg_data = bev->data;

	PHP_EVENT_TSRMLS_FETCH_FROM_CTX(bev->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
#ifdef HAVE_EVENT_PTHREADS_LIB
		if (bevent) {
			bufferevent_lock(bevent);
		}
#endif
		/* Setup callback args */

		arg_self = bev->self;
		if (arg_self) {
			Z_ADDREF_P(arg_self);
		} else {
			ALLOC_INIT_ZVAL(arg_self);
		}
		args[0] = &arg_self;

		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[1] = &arg_data;

		/* Prepare callback */
		pfci->params		 = args;
		pfci->retval_ptr_ptr = &retval_ptr;
		pfci->param_count	 = 2;
		pfci->no_separation  = 1;

		if (zend_call_function(pfci, pfcc TSRMLS_CC) == SUCCESS && retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		} else {
			if (EG(exception)) {
				PHP_EVENT_ASSERT(bev->base);
				PHP_EVENT_FETCH_BASE(b, bev->base);
				event_base_loopbreak(b->base);

				zval_ptr_dtor(&arg_data);
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"An error occurred while invoking the callback");
			}
        }

		zval_ptr_dtor(&arg_data);

#ifdef HAVE_EVENT_PTHREADS_LIB
		if (bevent) {
			bufferevent_unlock(bevent);
		}
#endif
        zval_ptr_dtor(&arg_self);
	}
}
/* }}} */

/* {{{ bevent_read_cb */
static void bevent_read_cb(struct bufferevent *bevent, void *ptr)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) ptr;

	bevent_rw_cb(bevent, bev, bev->fci_read, bev->fcc_read);
}
/* }}} */

/* {{{ bevent_write_cb */
static void bevent_write_cb(struct bufferevent *bevent, void *ptr)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) ptr;

	bevent_rw_cb(bevent, bev, bev->fci_write, bev->fcc_write);
}
/* }}} */

/* {{{ bevent_event_cb */
static void bevent_event_cb(struct bufferevent *bevent, short events, void *ptr)
{
	php_event_bevent_t    *bev  = (php_event_bevent_t *) ptr;
	zend_fcall_info       *pfci = bev->fci_event;
	zend_fcall_info_cache *pfcc = bev->fcc_event;
	zval  *arg_data;
	zval  *arg_events;
	zval  *arg_self;
	zval **args[3];
	zval  *retval_ptr = NULL;
	php_event_base_t *b;
	PHP_EVENT_TSRM_DECL

	PHP_EVENT_ASSERT(pfci && pfcc);
	PHP_EVENT_ASSERT(bevent);
	PHP_EVENT_ASSERT(bev->bevent == bevent);
	PHP_EVENT_ASSERT(bev->self);
	PHP_EVENT_ASSERT(bev->base);

	arg_data = bev->data;

	PHP_EVENT_TSRMLS_FETCH_FROM_CTX(bev->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
#ifdef HAVE_EVENT_PTHREADS_LIB
		if (bevent) {
			bufferevent_lock(bevent);
		}
#endif

		/* Setup callback args */

		arg_self = bev->self;
		if (arg_self) {
			Z_ADDREF_P(arg_self);
		} else {
			ALLOC_INIT_ZVAL(arg_self);
		}
		args[0] = &bev->self;

		MAKE_STD_ZVAL(arg_events);
		ZVAL_LONG(arg_events, events);
		args[1] = &arg_events;

		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[2] = &arg_data;

		/* Prepare callback */
		pfci->params		 = args;
		pfci->retval_ptr_ptr = &retval_ptr;
		pfci->param_count	 = 3;
		pfci->no_separation  = 1;

		if (zend_call_function(pfci, pfcc TSRMLS_CC) == SUCCESS && retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		} else {
			if (EG(exception)) {
				PHP_EVENT_ASSERT(bev->base);
				PHP_EVENT_FETCH_BASE(b, bev->base);
				event_base_loopbreak(b->base);

				zval_ptr_dtor(&arg_events);
				zval_ptr_dtor(&arg_data);
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"An error occurred while invoking the callback");
			}
		}

		zval_ptr_dtor(&arg_events);
		zval_ptr_dtor(&arg_data);

		PHP_EVENT_ASSERT(bevent);
#ifdef HAVE_EVENT_PTHREADS_LIB
		if (bevent) {
			bufferevent_unlock(bevent);
		}
#endif
		zval_ptr_dtor(&arg_self);
	}
}
/* }}} */

#ifdef HAVE_EVENT_OPENSSL_LIB
/* {{{ is_valid_ssl_state */
static zend_always_inline zend_bool is_valid_ssl_state(long state)
{
	return (zend_bool) (state == BUFFEREVENT_SSL_OPEN
			|| state == BUFFEREVENT_SSL_CONNECTING
			|| state == BUFFEREVENT_SSL_ACCEPTING);
}
/* }}} */

static void _create_ssl_filter(INTERNAL_FUNCTION_PARAMETERS, zend_bool deprecated)/*{{{*/
{
	zval                    *zunused;
	php_event_base_t        *base;
	zval                    *zunderlying;
	php_event_bevent_t      *bev_underlying;
	zval                    *zctx;
	php_event_ssl_context_t *ectx;
	long                     state;
	long                     options        = 0;
	php_event_bevent_t      *bev;
	struct bufferevent      *bevent;
	SSL                     *ssl;

	if (!deprecated) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOl|l",
					&zunderlying, php_event_bevent_ce,
					&zctx, php_event_ssl_context_ce,
					&state, &options) == FAILURE) {
			return;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zOOl|l",
					&zunused,
					&zunderlying, php_event_bevent_ce,
					&zctx, php_event_ssl_context_ce,
					&state, &options) == FAILURE) {
			return;
		}
	}

	if (!is_valid_ssl_state(state)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid state specified");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BEVENT(bev_underlying, zunderlying);
	_ret_if_invalid_bevent_ptr(bev_underlying);

	/* Must also be the base for the underlying bufferevent. See Libevent reference. */
	PHP_EVENT_FETCH_BASE(base, bev_underlying->base);

	PHP_EVENT_FETCH_SSL_CONTEXT(ectx, zctx);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_bevent_ce);
	PHP_EVENT_FETCH_BEVENT(bev, return_value);

	if (UNEXPECTED(ectx->ctx == NULL)) {
		RETURN_FALSE;
	}
	ssl = SSL_new(ectx->ctx);
	if (!ssl) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Event: Failed creating SSL handle");
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
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Failed to allocate bufferevent filter");
		RETURN_FALSE;
	}
	bev->bevent = bevent;

	bev->self = return_value;
	Z_ADDREF_P(return_value);

	bev->base = bev_underlying->base;
	Z_ADDREF_P(bev->base);
}/*}}}*/
#endif

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
PHP_METHOD(EventBufferEvent, __construct)
{
	zval                   *zself     = getThis();
	zval                   *zbase;
	php_event_base_t       *base;
	zval                  **ppzfd     = NULL;
	evutil_socket_t         fd;
	long                    options   = 0;
	php_event_bevent_t     *bev;
	struct bufferevent     *bevent;
	zend_fcall_info         fci_read  = empty_fcall_info;
	zend_fcall_info_cache   fcc_read  = empty_fcall_info_cache;
	zend_fcall_info         fci_write = empty_fcall_info;
	zend_fcall_info_cache   fcc_write = empty_fcall_info_cache;
	zend_fcall_info         fci_event = empty_fcall_info;
	zend_fcall_info_cache   fcc_event = empty_fcall_info_cache;
	zval                   *zarg      = NULL;
	bufferevent_data_cb     read_cb;
	bufferevent_data_cb     write_cb;
	bufferevent_event_cb    event_cb;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|Z!lf!f!f!z!",
				&zbase, php_event_base_ce, &ppzfd, &options,
				&fci_read, &fcc_read,
				&fci_write, &fcc_write,
				&fci_event, &fcc_event,
				&zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	if (ppzfd) {
		/* php_event_zval_to_fd reports error
	 	 * in case if it is not a valid socket resource */
		/*fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);*/
		fd = php_event_zval_to_fd(ppzfd TSRMLS_CC);

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

	PHP_EVENT_FETCH_BASE(base, zbase);

	PHP_EVENT_FETCH_BEVENT(bev, zself);

#ifdef HAVE_EVENT_PTHREADS_LIB
	options |= BEV_OPT_THREADSAFE;
#endif
	bevent = bufferevent_socket_new(base->base, fd, options);
	if (bevent == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed to allocate bufferevent for socket");
		return;
	}
	bev->_internal = 0;
	bev->bevent = bevent;

	bev->self = zself;
	Z_ADDREF_P(zself);

	bev->base = zbase;
	Z_ADDREF_P(zbase);

	bev->input = bev->output = NULL;

	if (ZEND_FCI_INITIALIZED(fci_read)) {
		read_cb = bevent_read_cb;
		PHP_EVENT_FREE_FCALL_INFO(bev->fci_read, bev->fcc_read);
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_read, bev->fcc_read, &fci_read, &fcc_read);
	} else {
		if (bev->fci_read) {
			PHP_EVENT_FREE_FCALL_INFO(bev->fci_read, bev->fcc_read);
		}
		read_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_write)) {
		write_cb = bevent_write_cb;
		PHP_EVENT_FREE_FCALL_INFO(bev->fci_write, bev->fcc_write);
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_write, bev->fcc_write, &fci_write, &fcc_write);
	} else {
		if (bev->fci_write) {
			PHP_EVENT_FREE_FCALL_INFO(bev->fci_write, bev->fcc_write);
		}
		write_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_event)) {
		event_cb = bevent_event_cb;
		PHP_EVENT_FREE_FCALL_INFO(bev->fci_event, bev->fcc_event);
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_event, bev->fcc_event, &fci_event, &fcc_event);
	} else {
		if (bev->fci_event) {
			PHP_EVENT_FREE_FCALL_INFO(bev->fci_event, bev->fcc_event);
		}
		event_cb = NULL;
	}

	if (zarg) {
		Z_ADDREF_P(zarg);
		bev->data = zarg;
	}

	TSRMLS_SET_CTX(bev->thread_ctx);

	if (read_cb || write_cb || event_cb || zarg) {
		bufferevent_setcb(bev->bevent, read_cb, write_cb, event_cb, (void *) bev);
	}
}
/* }}} */

/* {{{ proto void EventBufferEvent::free(void); */
PHP_METHOD(EventBufferEvent, free)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	if (bev->bevent) {
		if (!bev->_internal) {
			bufferevent_free(bev->bevent);
		}
		bev->bevent = 0;

		/* Do it once */
		if (bev->self) {
			zval_ptr_dtor(&bev->self);
			bev->self = NULL;
		}
		if (bev->base) {
			zval_ptr_dtor(&bev->base);
			bev->base = NULL;
		}
	}
}
/* }}} */

/* {{{ proto bool EventBufferEvent::close(void); */
PHP_METHOD(EventBufferEvent, close)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	evutil_socket_t     fd;

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

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

/* {{{ proto array EventBufferEvent::createPair(EventBase base[, int options = 0]);
 *
 * options is one of EVENT_BEV_OPT_* constants, or 0.
 *
 * Returns array of two EventBufferEvent objects connected to each other.
 * All the usual options are supported, except for EVENT_BEV_OPT_CLOSE_ON_FREE,
 * which has no effect, and EVENT_BEV_OPT_DEFER_CALLBACKS, which is always on.
 */
PHP_METHOD(EventBufferEvent, createPair)
{
	zval               *zbase;
	php_event_base_t   *base;
	long                options        = 0;
	zval               *zbev[2];
	php_event_bevent_t *b[2];
	struct bufferevent *bevent_pair[2];
	int                 i;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|l",
				&zbase, php_event_base_ce, &options) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (bufferevent_pair_new(base->base, options, bevent_pair)) {
		RETURN_FALSE;
	}

	array_init(return_value);

	for (i = 0; i < 2; i++) {
		MAKE_STD_ZVAL(zbev[i]);
		PHP_EVENT_INIT_CLASS_OBJECT(zbev[i], php_event_bevent_ce);
		PHP_EVENT_FETCH_BEVENT(b[i], zbev[i]);

		b[i]->bevent    = bevent_pair[i];

		add_next_index_zval(return_value, zbev[i]);
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
PHP_METHOD(EventBufferEvent, connect)
{
	php_event_bevent_t      *bev;
	zval                    *zbevent  = getThis();
	char                    *addr;
	int                      addr_len;
	struct sockaddr_storage  ss;
	int                      ss_len   = sizeof(ss);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&addr, &addr_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	memset(&ss, 0, sizeof(ss));

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

			php_error_docref(NULL TSRMLS_CC, E_WARNING,
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

/* {{{ proto bool EventBufferEvent::connectHost(EventDnsBase dns_base, string hostname, int port[, int family = EventUtil::AF_UNSPEC]);
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
PHP_METHOD(EventBufferEvent, connectHost)
{
#if LIBEVENT_VERSION_NUMBER < 0x02000300
	PHP_EVENT_LIBEVENT_VERSION_REQUIRED(bufferevent_socket_connect_hostname, 2.0.3-alpha);
	RETVAL_FALSE;
#else
	php_event_bevent_t *bev;
	zval               *zbevent      = getThis();
	char               *hostname;
	int                 hostname_len;
	long                port;
	long                family       = AF_UNSPEC;
#ifdef HAVE_EVENT_EXTRA_LIB
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O!sl|l",
				&zdns_base, php_event_dns_base_ce, &hostname, &hostname_len,
				&port, &family) == FAILURE) {
		return;
	}
#else
	zval *zunused;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zsl|l",
				&zunused, &hostname, &hostname_len,
				&port, &family) == FAILURE) {
		return;
	}
#endif

	if (family & ~(AF_INET | AF_INET6 | AF_UNSPEC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid address family specified");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	/* bufferevent_socket_connect() allocates a socket stream internally, if we
	 * didn't provide the file descriptor to the bufferevent before, e.g. with
	 * bufferevent_socket_new() */

#ifdef HAVE_EVENT_EXTRA_LIB
	if (zdns_base) {
		PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);
	}

	if (bufferevent_socket_connect_hostname(bev->bevent,
				(zdns_base ? dnsb->dns_base : NULL),
				family, hostname, port)) {
# ifdef PHP_EVENT_DEBUG
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s",
				evutil_gai_strerror(bufferevent_socket_get_dns_error(bev->bevent)));
# endif
		RETURN_FALSE;
	}
#else /* don't HAVE_EVENT_EXTRA_LIB */
	if (bufferevent_socket_connect_hostname(bev->bevent,
				NULL,
				family, hostname, port)) {
# ifdef PHP_EVENT_DEBUG
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s",
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
PHP_METHOD(EventBufferEvent, getDnsErrorString)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	int                 err;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	err = bufferevent_socket_get_dns_error(bev->bevent);

	if (err == 0) {
		RETURN_EMPTY_STRING();
	}
	RETVAL_STRING(evutil_gai_strerror(err), 1);
}
/* }}} */

/* {{{ proto void EventBufferEvent::setCallbacks(callable readcb, callable writecb, callable eventcb[, mixed arg = NULL]);
 * Changes one or more of the callbacks of a bufferevent.
 * A callback may be disabled by passing NULL instead of the callable.
 * arg is an argument passed to the callbacks.
 */
PHP_METHOD(EventBufferEvent, setCallbacks)
{
	zval                  *zbevent   = getThis();
	php_event_bevent_t    *bev;
	zend_fcall_info        fci_read  = empty_fcall_info;
	zend_fcall_info_cache  fcc_read  = empty_fcall_info_cache;
	zend_fcall_info        fci_write = empty_fcall_info;
	zend_fcall_info_cache  fcc_write = empty_fcall_info_cache;
	zend_fcall_info        fci_event = empty_fcall_info;
	zend_fcall_info_cache  fcc_event = empty_fcall_info_cache;
	zval                  *zarg      = NULL;
	bufferevent_data_cb    read_cb;
	bufferevent_data_cb    write_cb;
	bufferevent_event_cb   event_cb;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f!f!f!|z!",
				&fci_read, &fcc_read,
				&fci_write, &fcc_write,
				&fci_event, &fcc_event,
				&zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	if (ZEND_FCI_INITIALIZED(fci_read)) {
		read_cb = bevent_read_cb;
		PHP_EVENT_FREE_FCALL_INFO(bev->fci_read, bev->fcc_read);
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_read, bev->fcc_read, &fci_read, &fcc_read);
	} else {
		if (bev->fci_read) {
			PHP_EVENT_FREE_FCALL_INFO(bev->fci_read, bev->fcc_read);
		}
		read_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_write)) {
		write_cb = bevent_write_cb;
		PHP_EVENT_FREE_FCALL_INFO(bev->fci_write, bev->fcc_write);
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_write, bev->fcc_write, &fci_write, &fcc_write);
	} else {
		if (bev->fci_write) {
			PHP_EVENT_FREE_FCALL_INFO(bev->fci_write, bev->fcc_write);
		}
		write_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_event)) {
		event_cb = bevent_event_cb;
		PHP_EVENT_FREE_FCALL_INFO(bev->fci_event, bev->fcc_event);
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_event, bev->fcc_event, &fci_event, &fcc_event);
	} else {
		if (bev->fci_event) {
			PHP_EVENT_FREE_FCALL_INFO(bev->fci_event, bev->fcc_event);
		}
		event_cb = NULL;
	}

	if (zarg) {
		Z_ADDREF_P(zarg);
		bev->data = zarg;
	}

	TSRMLS_SET_CTX(bev->thread_ctx);

	bufferevent_setcb(bev->bevent, read_cb, write_cb, event_cb, (void *) bev);
}
/* }}} */

/* {{{ proto bool EventBufferEvent::enable(int events);
 * Enable events EVENT_READ, EVENT_WRITE, or EVENT_READ | EVENT_WRITE on a buffer event. */
PHP_METHOD(EventBufferEvent, enable)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	long                events;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&events) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	if (bufferevent_enable(bev->bevent, events)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBufferEvent::disable(int events);
 * Disable events EVENT_READ, EVENT_WRITE, or EVENT_READ | EVENT_WRITE on a buffer event. */
PHP_METHOD(EventBufferEvent,disable)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	long                events;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&events) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	if (bufferevent_disable(bev->bevent, events)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int EventBufferEvent::getEnabled(void);
 * Returns bitmask of events currently enabled on the buffer event. */
PHP_METHOD(EventBufferEvent, getEnabled)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);


	RETVAL_LONG(bufferevent_get_enabled(bev->bevent));
}
/* }}} */

/* {{{ proto EventBuffer EventBufferEvent::getInput(void);
 *
 * Returns an input EventBuffer object associated with the buffer event */
PHP_METHOD(EventBufferEvent, getInput)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_buffer_ce);
	PHP_EVENT_FETCH_BUFFER(b, return_value);
	/* Don't do this. It's normal to have refcount = 1 here.
	 * If we got bugs, we most likely free'd an internal buffer somewhere
	 * Z_ADDREF_P(return_value);*/

	b->buf      = bufferevent_get_input(bev->bevent);
	b->internal = 1;
}
/* }}} */

/* {{{ proto EventBuffer EventBufferEvent::getOutput(void);
 *
 * Returns an output EventBuffer object associated with the buffer event */
PHP_METHOD(EventBufferEvent, getOutput)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_buffer_ce);
	PHP_EVENT_FETCH_BUFFER(b, return_value);
	/* Don't do this. It's normal to have refcount = 1 here.
	 * If we got bugs, we most likely free'd an internal buffer somewhere
	 * Z_ADDREF_P(return_value);*/

	b->buf      = bufferevent_get_output(bev->bevent);
	b->internal = 1;
}
/* }}} */

/* {{{ proto void EventBufferEvent::setWatermark(int events, int lowmark, int highmark);
 * Adjusts the read watermarks, the write watermarks, or both, of a single bufferevent. */
PHP_METHOD(EventBufferEvent, setWatermark)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	long                events;
	long                lowmark;
	long                highmark;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll",
				&events, &lowmark, &highmark) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	bufferevent_setwatermark(bev->bevent, events, (size_t) lowmark, (size_t) highmark);
}
/* }}} */

/* {{{ proto bool EventBufferEvent::write(string data);
 * Adds `data' to a buffer event's output buffer. */
PHP_METHOD(EventBufferEvent, write)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	zval               *zdata;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
				&zdata) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	convert_to_string(zdata);

	if (bufferevent_write(bev->bevent, Z_STRVAL_P(zdata), Z_STRLEN_P(zdata))) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBufferEvent::writeBuffer(EventBuffer buf);
 * Adds contents of the entire buffer to a buffer event's output buffer. */
PHP_METHOD(EventBufferEvent, writeBuffer)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&zbuf, php_event_buffer_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (bufferevent_write_buffer(bev->bevent, b->buf)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto string EventBufferEvent::read(int size);
 * Removes up to size bytes from the input buffer, storing them into the memory at data.
 */
PHP_METHOD(EventBufferEvent, read)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	long                size;
	char               *data;
	long                ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&size) == FAILURE) {
		return;
	}

	if (size < 0) {
		/*php_error_docref(NULL TSRMLS_CC, E_WARNING, "Size must be positive");*/
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	data = safe_emalloc(size, sizeof(char), 1);

	ret = bufferevent_read(bev->bevent, data, size);

	if (ret > 0) {
		RETVAL_STRINGL(data, ret, 1);
	} else {
		RETVAL_NULL();
	}

	efree(data);
}
/* }}} */

/* {{{ proto bool EventBufferEvent::readBuffer(EventBuffer buf);
 * Drains the entire contents of the input buffer and places them into buf */
PHP_METHOD(EventBufferEvent, readBuffer)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&zbuf, php_event_buffer_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (bufferevent_read_buffer(bev->bevent, b->buf)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBufferEvent::setPriority(int priority);
 * Assign a priority to a bufferevent.
 * Only supported for socket bufferevents. */
PHP_METHOD(EventBufferEvent, setPriority)
{
	zval               *zbevent  = getThis();
	php_event_bevent_t *bev;
	long                priority;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&priority) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	if (bufferevent_priority_set(bev->bevent, priority)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBufferEvent::setTimeouts(double timeout_read, double timeout_write);
 * Set the read and write timeout for a bufferevent. */
PHP_METHOD(EventBufferEvent, setTimeouts)
{
	zval               *zbevent       = getThis();
	php_event_bevent_t *bev;
	double              timeout_read;
	double              timeout_write;
	struct timeval      tv_read;
	struct timeval      tv_write;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dd",
				&timeout_read, &timeout_write) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
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
/* {{{ proto EventBufferEvent EventBufferEvent::sslFilter(mixed unused, EventBufferEvent underlying, EventSslContext ctx, int state[, int options = 0]);
 */
PHP_METHOD(EventBufferEvent, sslFilter)
{
	_create_ssl_filter(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto EventBufferEvent EventBufferEvent::createSslFilter(EventBufferEvent underlying, EventSslContext ctx, int state[, int options = 0]);
 */
PHP_METHOD(EventBufferEvent, createSslFilter)
{
	_create_ssl_filter(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto EventBufferEvent EventBufferEvent::sslSocket(EventBase base, mixed socket, EventSslContext ctx, int state[, int options = 0]);
 * */
PHP_METHOD(EventBufferEvent, sslSocket)
{
	zval                     *zbase;
	php_event_base_t         *base;
	zval                     *zctx;
	php_event_ssl_context_t  *ectx;
	zval                    **ppzfd;
	evutil_socket_t           fd;
	long                      state;
	long                      options = 0;
	php_event_bevent_t       *bev;
	struct bufferevent       *bevent;
	SSL                      *ssl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OZOl|l",
				&zbase, php_event_base_ce,
				&ppzfd,
				&zctx, php_event_ssl_context_ce,
				&state, &options) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	if (!is_valid_ssl_state(state)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid state specified");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);
	PHP_EVENT_FETCH_SSL_CONTEXT(ectx, zctx);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_bevent_ce);
	PHP_EVENT_FETCH_BEVENT(bev, return_value);

	if (Z_TYPE_PP(ppzfd) == IS_NULL) {
		/* User decided to set fd later via connect or connectHost etc.*/
		fd = -1;
	} else {
		fd = php_event_zval_to_fd(ppzfd TSRMLS_CC);
		if (fd < 0) {
			RETURN_FALSE;
		}
		/* Make sure that the socket is in non-blocking mode(libevent's tip) */
		/*evutil_make_socket_nonblocking(fd);*/
	}

	PHP_EVENT_ASSERT(ectx->ctx);
	ssl = SSL_new(ectx->ctx);
	if (!ssl) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
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
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed to allocate bufferevent filter");
		RETURN_FALSE;
	}
	bev->bevent = bevent;

	bev->self = return_value;
	Z_ADDREF_P(return_value);

	bev->base = zbase;
	Z_ADDREF_P(zbase);
}
/* }}} */

/* {{{ proto string EventBufferEvent::sslError(void);
 *
 * Returns most recent OpenSSL error reported on the buffer event. The function
 * returns FALSE, if there is no more error to return. */
PHP_METHOD(EventBufferEvent, sslError)
{
	zval               *zbevent  = getThis();
	php_event_bevent_t *bev;
	char                buf[512];
	unsigned long       e;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	e = bufferevent_get_openssl_error(bev->bevent);
	if (e) {
		RETURN_STRING(ERR_error_string(e, buf), 1);
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
PHP_METHOD(EventBufferEvent, sslRenegotiate)
{
	zval               *zbevent  = getThis();
	php_event_bevent_t *bev;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	bufferevent_ssl_renegotiate(bev->bevent);
}
/* }}} */

/* {{{ proto string EventBufferEvent::sslGetCipherInfo(void);
 *
 * Returns the current Cipher of the connexion as
 * SSL_get_current_cipher/SSL_CIPHER_description do. Otherwise FALSE. */
PHP_METHOD(EventBufferEvent, sslGetCipherInfo)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	struct ssl_st      *ssl;
	char               *desc;
	const SSL_CIPHER   *cipher;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	ssl = bufferevent_openssl_get_ssl(bev->bevent);
	if (ssl) {
		cipher = SSL_get_current_cipher(ssl);
		if (cipher) {
			desc = SSL_CIPHER_description(cipher, NULL, 128);
			RETVAL_STRING(desc, 1);
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
PHP_METHOD(EventBufferEvent, sslGetCipherName)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	struct ssl_st      *ssl;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	ssl = bufferevent_openssl_get_ssl(bev->bevent);
	if (ssl) {
		RETURN_STRING(SSL_get_cipher_name(ssl), 1);
	}
	RETVAL_FALSE;
}
/* }}} */

/* {{{ proto string EventBufferEvent::sslGetCipherVersion(void);
 *
 * Returns the current cipher version of the connection as SSL_get_cipher_version does.
 * returns FALSE, if there is no more error to return. */
PHP_METHOD(EventBufferEvent, sslGetCipherVersion)
{
	zval               *zbevent  = getThis();
	php_event_bevent_t *bev;
	struct ssl_st      *ssl;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	ssl = bufferevent_openssl_get_ssl(bev->bevent);
	if (ssl) {
		RETURN_STRING(SSL_get_cipher_version(ssl), 1);
	}
	RETVAL_FALSE;
}
/* }}} */

/* {{{ proto string EventBufferEvent::sslGetProtocol(void);
 *
 * Returns the current Protocol of the connection as SSL_get_version does,
 * otherwise FALSE. */
PHP_METHOD(EventBufferEvent, sslGetProtocol)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	struct ssl_st      *ssl;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	_ret_if_invalid_bevent_ptr(bev);

	ssl = bufferevent_openssl_get_ssl(bev->bevent);
	if (ssl) {
		RETURN_STRING(SSL_get_version(ssl), 1);
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
