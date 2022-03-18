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

/* {{{ Private */

/* {{{ zval_to_signum */
static zend_always_inline evutil_socket_t zval_to_signum(zval *pzfd)
{
	evutil_socket_t fd;

	convert_to_long_ex(pzfd);

	fd = Z_LVAL_P(pzfd);

	if (fd < 0 || fd >= NSIG) {
		return -1;
	}

	return fd;
}
/* }}} */


/* {{{ timer_cb */
static void timer_cb(evutil_socket_t fd, short what, void *arg)
{
	php_event_t     *e         = (php_event_t *)arg;
	zend_fcall_info  fci;
	zval             argv[1];
	zval             retval;
	zend_string     *func_name;
	zval             zcallable;

	PHP_EVENT_ASSERT(e);
	PHP_EVENT_ASSERT(what & EV_TIMEOUT);

	/* Protect against accidental destruction of the func name before zend_call_function() finished */
	ZVAL_COPY(&zcallable, &e->cb.func_name);

	if (!zend_is_callable(&zcallable, 0, &func_name)) {
		zend_string_release(func_name);
		return;
	}
	zend_string_release(func_name);

	if (Z_ISUNDEF(e->data)) {
		ZVAL_NULL(&argv[0]);
	} else {
		ZVAL_COPY(&argv[0], &e->data);
	}

	zend_fcall_info_init(&zcallable, 0, &fci, &e->cb.fci_cache, NULL, NULL);
	fci.retval = &retval;
	fci.params = argv;
	fci.param_count = 1;

	if (zend_call_function(&fci, &e->cb.fci_cache) == SUCCESS) {
		if (!Z_ISUNDEF(retval)) {
			zval_ptr_dtor(&retval);
		}
	} else {
		php_error_docref(NULL, E_WARNING, "Failed to invoke timer callback");
	}

	zval_ptr_dtor(&zcallable);

	zval_ptr_dtor(&argv[0]);
}
/* }}} */

/* {{{ event_cb */
static void event_cb(evutil_socket_t fd, short what, void *arg)
{
	php_event_t     *e         = (php_event_t *) arg;
	zend_fcall_info  fci;
	zval             argv[3];
	zval             retval;
	zend_string     *func_name;
	zval             zcallable;

	PHP_EVENT_ASSERT(e);

	/* Protect against accidental destruction of the func name before zend_call_function() finished */
	ZVAL_COPY(&zcallable, &e->cb.func_name);

	if (!zend_is_callable(&zcallable, 0, &func_name)) {
		zend_string_release(func_name);
		return;
	}
	zend_string_release(func_name);

	if ((what & EV_SIGNAL) || e->stream_res == NULL) {
		ZVAL_LONG(&argv[0], fd);
	} else if (e->stream_res) {
		ZVAL_RES(&argv[0], e->stream_res);
		Z_TRY_ADDREF(argv[0]);
	} else {
		ZVAL_NULL(&argv[0]);
	}

	ZVAL_LONG(&argv[1], what);

	if (!Z_ISUNDEF(e->data)) {
		ZVAL_COPY(&argv[2], &e->data);
	} else {
		ZVAL_NULL(&argv[2]);
	}

	zend_fcall_info_init(&zcallable, 0, &fci, &e->cb.fci_cache, NULL, NULL);
	fci.retval = &retval;
	fci.params = argv;
	fci.param_count = 3;

	if (zend_call_function(&fci, &e->cb.fci_cache) == SUCCESS) {
		if (!Z_ISUNDEF(retval)) {
			zval_ptr_dtor(&retval);
		}
	} else {
		php_error_docref(NULL, E_WARNING, "Failed to invoke event callback");
	}

	zval_ptr_dtor(&zcallable);

	zval_ptr_dtor(&argv[2]);
	zval_ptr_dtor(&argv[1]);
	zval_ptr_dtor(&argv[0]);
}
/* }}} */

/* {{{ signal_cb */
static void signal_cb(evutil_socket_t signum, short what, void *arg)
{
	php_event_t     *e       = (php_event_t *)arg;
	zend_fcall_info  fci;
	zval             argv[2];
	zval             retval;
	zend_string     *func_name;
	zval             zcallable;

	PHP_EVENT_ASSERT(e);
	PHP_EVENT_ASSERT(what & EV_SIGNAL);

	/* Protect against accidental destruction of the func name before zend_call_function() finished */
	ZVAL_COPY(&zcallable, &e->cb.func_name);

	if (!zend_is_callable(&zcallable, 0, &func_name)) {
		zend_string_release(func_name);
		return;
	}
	zend_string_release(func_name);

	ZVAL_LONG(&argv[0], signum);

	if (Z_ISUNDEF(e->data)) {
		ZVAL_NULL(&argv[1]);
	} else {
		ZVAL_COPY(&argv[1], &e->data);
	}

	zend_fcall_info_init(&zcallable, 0, &fci, &e->cb.fci_cache, NULL, NULL);
	fci.retval = &retval;
	fci.params = argv;
	fci.param_count = 2;

	if (zend_call_function(&fci, &e->cb.fci_cache) == SUCCESS) {
		if (!Z_ISUNDEF(retval)) {
			zval_ptr_dtor(&retval);
		}
	} else {
		php_error_docref(NULL, E_WARNING, "Failed to invoke signal callback");
	}

	zval_ptr_dtor(&zcallable);

	zval_ptr_dtor(&argv[0]);
	zval_ptr_dtor(&argv[1]);
}
/* }}} */

/* }}} */


/* {{{ proto Event Event::__construct(EventBase base, mixed fd, int what, callable cb[, mixed arg = NULL]);
 * Creates new event */
PHP_EVENT_METHOD(Event, __construct)
{
	struct event     *event;
	zval             *zbase;
	zval             *pzfd;
	zval             *zcb;
	php_event_base_t *b;
	php_event_t      *e;
	evutil_socket_t   fd;
	zend_long         what;
	zval             *zself            = getThis();
	zval             *zarg             = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Ozlz|z!",
				&zbase, php_event_base_ce, &pzfd, &what, &zcb, &zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	if (what & ~(EV_TIMEOUT | EV_READ | EV_WRITE | EV_SIGNAL | EV_PERSIST | EV_ET)) {
		php_error_docref(NULL, E_WARNING, "Invalid mask");
		return;
	}

	if (what & EV_SIGNAL) {
		fd = zval_to_signum(pzfd);
		if (fd == -1) {
			php_error_docref(NULL, E_WARNING, "Invalid signal passed");
			return;
		}
	} else if (what & EV_TIMEOUT) {
		fd = -1;
	} else {
		fd = (evutil_socket_t) php_event_zval_to_fd(pzfd);
		if (fd < 0) {
			return;
		}
	}

	b = Z_EVENT_BASE_OBJ_P(zbase);
	PHP_EVENT_ASSERT(b);

	/* TODO: check if a signum is bound to different event bases */
	e = Z_EVENT_EVENT_OBJ_P(zself);

	event = event_new(b->base, fd, what, event_cb, (void *)e);
	if (UNEXPECTED(!event)) {
		php_error_docref(NULL, E_ERROR, "event_new failed");
		return;
	}

	e->event = event;

	if (zarg) {
		ZVAL_COPY(&e->data, zarg);
	} else {
		ZVAL_UNDEF(&e->data);
	}

	php_event_copy_callback(&e->cb, zcb);

	if (what & EV_SIGNAL) {
		e->stream_res = NULL;
	} else if (Z_TYPE_P(pzfd) == IS_RESOURCE) {
		e->stream_res = fd == -1 ? NULL : Z_RES_P(pzfd);
	} else {
		/* We might open a stream for fd, but the only place where we need
		 * stream_res is the event callback, which should only bypass the
		 * fd argument to the user. Since fd is a numeric fd, we can simply
		 * pass ZVAL_LONG(fd) in the event callback. */
		e->stream_res = NULL;
	}
}
/* }}} */

/* {{{ proto void Event::free(void); */
PHP_EVENT_METHOD(Event, free)
{
	php_event_t *e;

	e = Z_EVENT_EVENT_OBJ_P(getThis());
	PHP_EVENT_ASSERT(e);

	if (e->event) {
		/* No need in
		 * event_del(e->event);
		 * since event_free makes event non-pending internally */
		event_free(e->event);
		e->event = NULL;
	}
}
/* }}} */

/* {{{ proto bool Event::set(EventBase base, mixed fd,[ int what = NULL[, callable cb = NULL[, zval arg = NULL]]]);
 *
 * Re-configures event.
 * Note, this function doesn't invoke obsolete libevent's event_set. It calls event_assign instead.  */
PHP_EVENT_METHOD(Event, set)
{
	zval             *zbase;
	php_event_base_t *b;
	php_event_t      *e;
	zval             *pzfd  = NULL;
	evutil_socket_t   fd    = -1;
	zend_long         what  = -1;
	zend_fcall_info   fci = {0};
	zend_fcall_info_cache fci_cache;
	zval             *zarg   = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Oz!|lf!z!",
				&zbase, php_event_base_ce, &pzfd,
				&what, &fci, &fci_cache, &zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	if (what != -1) {
		if (what & ~(EV_TIMEOUT | EV_READ | EV_WRITE | EV_SIGNAL | EV_PERSIST | EV_ET)) {
			php_error_docref(NULL, E_WARNING, "Invalid events mask");
			RETURN_FALSE;
		}

		if (what & EV_SIGNAL) {
			if (zval_to_signum(pzfd) == -1) {
				php_error_docref(NULL, E_WARNING, "Invalid signal passed");
				RETURN_FALSE;
			}
		} else {
			fd = (evutil_socket_t)php_event_zval_to_fd(pzfd);
			if (fd < 0) {
				RETURN_FALSE;
			}
		}
	}

	e = Z_EVENT_EVENT_OBJ_P(getThis());

	if (php_event_is_pending(e->event)) {
		php_error_docref(NULL, E_WARNING, "Can't modify pending event");
		RETURN_FALSE;
	}

	b = Z_EVENT_BASE_OBJ_P(zbase);
	PHP_EVENT_ASSERT(b);

	/* TODO: check if a signum bound to different event bases */

	if (pzfd) {
		if (what != -1 && what & EV_SIGNAL) {
			e->stream_res = NULL; /* stdin fd = 0 */
		} else if (Z_TYPE_P(pzfd) == IS_RESOURCE) {
			e->stream_res = Z_RES_P(pzfd);
		} else {
			e->stream_res = NULL;
		}
	}

	if (ZEND_FCI_INITIALIZED(fci)) {
		php_event_replace_callback(&e->cb, &fci.function_name);
	}

	if (zarg) {
		if (!Z_ISUNDEF(e->data)) {
			zval_ptr_dtor(&e->data);
		}
		ZVAL_COPY(&e->data, zarg);
	}

	event_get_assignment(e->event, &b->base,
			(pzfd ? NULL : &fd),
			(short *) (what == -1 ? &what : NULL),
			NULL /* ignore old callback */ ,
			NULL /* ignore old callback argument */);

	if (event_assign(e->event, b->base, fd, what, event_cb, (void *)e)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto array Event::getSupportedMethods(void);
 * Returns array with of the names of the methods supported in this version of Libevent */
PHP_EVENT_METHOD(Event, getSupportedMethods)
{
	int i;
	const char **methods;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	methods = event_get_supported_methods();

	if (methods == NULL) {
		RETURN_FALSE;
	}

	array_init(return_value);

	for (i = 0; methods[i] != NULL; ++i) {
		add_next_index_string(return_value, methods[i]);
	}
}
/* }}} */

/* {{{ proto bool Event::add([double timeout = -1.0]);
 * Make event pending. */
PHP_EVENT_METHOD(Event, add)
{
	zval        *zevent = getThis();
	php_event_t *e;
	double       timeout = -1;
	int          res;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|d",
				&timeout) == FAILURE) {
		return;
	}

	e = Z_EVENT_EVENT_OBJ_P(zevent);

	if (!e->event) {
		php_error_docref(NULL, E_WARNING, "Failed adding event: Event object is malformed or freed");
		RETURN_FALSE;
	}

	if (timeout == -1) {
		res = event_add(e->event, NULL);
	} else {
		struct timeval tv;
		PHP_EVENT_TIMEVAL_SET(tv, timeout);

		res = event_add(e->event, &tv);
	}

	if (res) {
		php_error_docref(NULL, E_WARNING, "Failed adding event");
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool Event::del(void);
 * Remove an event from the set of monitored events. */
PHP_EVENT_METHOD(Event, del)
{
	zval        *zevent = getThis();
	php_event_t *e;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	e = Z_EVENT_EVENT_OBJ_P(zevent);

	if (e->event == NULL || event_del(e->event)) {
		php_error_docref(NULL, E_WARNING, "Failed deletting event");
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02010200
/* {{{ proto bool Event::removeTimer(void);
 * Remove a pending event’s timeout completely without deleting its IO or signal components.
 * Available since libevent 2.1.2-alpha. */
PHP_EVENT_METHOD(Event, removeTimer)
{
	zval        *zevent = getThis();
	php_event_t *e;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	e = Z_EVENT_EVENT_OBJ_P(zevent);

	if (event_remove_timer(e->event)) {
		php_error_docref(NULL, E_WARNING, "Failed deletting event");
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */
#endif

/* {{{ proto bool Event::setPriority(int priority);
 * Set event priority. */
PHP_EVENT_METHOD(Event, setPriority)
{
	zval        *zevent = getThis();
	php_event_t *e;
	zend_long        priority;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&priority) == FAILURE) {
		return;
	}

	e = Z_EVENT_EVENT_OBJ_P(zevent);

	if (event_priority_set(e->event, priority)) {
		php_error_docref(NULL, E_WARNING, "Unable to set event priority: %ld", priority);
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool Event::pending(int flags);
 *  Detect whether event is pending or scheduled. */
PHP_EVENT_METHOD(Event, pending)
{
	zval        *zevent = getThis();
	php_event_t *e;
	zend_long        flags;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&flags) == FAILURE) {
		return;
	}

	e = Z_EVENT_EVENT_OBJ_P(zevent);

	if (event_pending(e->event, flags, NULL)) {
		RETURN_TRUE;
	}
	RETVAL_FALSE;
}
/* }}} */


/* {{{ proto Event Event::timer(EventBase base, callable cb[, mixed arg = NULL]);
 * Factory method for timer event */
PHP_EVENT_METHOD(Event, timer)
{
	zval             *zbase;
	php_event_base_t *b;
	zval             *zcb;
	zval             *zarg  = NULL;
	php_event_t      *e;
	struct event     *event;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Oz|z!",
				&zbase, php_event_base_ce, &zcb, &zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	b = Z_EVENT_BASE_OBJ_P(zbase);
	PHP_EVENT_ASSERT(zbase);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_ce);
	e = Z_EVENT_EVENT_OBJ_P(return_value);

	event = evtimer_new(b->base, timer_cb, (void *)e);
	if (UNEXPECTED(!event)) {
		RETURN_FALSE;
	}

	e->event = event;
	if (zarg) {
		ZVAL_COPY(&e->data, zarg);
	} else {
		ZVAL_UNDEF(&e->data);
	}
	php_event_copy_callback(&e->cb, zcb);
	e->stream_res = NULL; /* stdin fd = 0 */
}
/* }}} */

/* {{{ proto bool Event::setTimer(EventBase base, callable cb[, mixed arg = NULL]);
 * Re-configures timer event.
 * Note, this function doesn't invoke obsolete libevent's event_set. It calls event_assign instead. */
PHP_EVENT_METHOD(Event, setTimer)
{
	zval             *zbase;
	php_event_base_t *b;
	php_event_t      *e;
    zend_fcall_info   fci;
    zend_fcall_info_cache fci_cache;
	zval             *zarg   = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Of|z!",
				&zbase, php_event_base_ce,
				&fci, &fci_cache, &zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	e = Z_EVENT_EVENT_OBJ_P(getThis());
	PHP_EVENT_ASSERT(e);

	if (evtimer_pending(e->event, NULL)) {
		php_error_docref(NULL, E_WARNING, "Can't modify pending timer");
		RETURN_FALSE;
		return;
	}

	b = Z_EVENT_BASE_OBJ_P(zbase);

	php_event_replace_callback(&e->cb, &fci.function_name);
	if (zarg) {
		ZVAL_COPY(&e->data, zarg);
	} else {
		ZVAL_UNDEF(&e->data);
	}

	e->stream_res = NULL; /* stdin fd = 0 */

	if (evtimer_assign(e->event, b->base, timer_cb, (void *) e)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto Event signal(EventBase base, int signum, callable cb[, mixed arg = NULL]);
 * Factory method for signal event */
PHP_EVENT_METHOD(Event, signal)
{
	struct event     *event;
	zval             *zbase;
	zval             *zcb;
	zval             *zarg   = NULL;
	php_event_t      *e;
	php_event_base_t *b;
	zend_long         signum;


	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Olz|z!",
				&zbase, php_event_base_ce, &signum, &zcb, &zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	if (signum < 0 || signum >= NSIG) {
		php_error_docref(NULL, E_WARNING, "Invalid signal passed");
		RETURN_FALSE;
	}

	b = Z_EVENT_BASE_OBJ_P(zbase);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_ce);
	e = Z_EVENT_EVENT_OBJ_P(return_value);

	event = evsignal_new(b->base, signum, signal_cb, (void *)e);
	if (UNEXPECTED(!event)) {
		RETURN_FALSE;
	}

	e->event = event;
	if (zarg) {
		ZVAL_COPY(&e->data, zarg);
	} else {
		ZVAL_UNDEF(&e->data);
	}
	php_event_copy_callback(&e->cb, zcb);
	e->stream_res = NULL; /* stdin fd = 0 */
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
