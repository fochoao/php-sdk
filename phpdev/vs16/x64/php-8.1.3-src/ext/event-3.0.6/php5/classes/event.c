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

/* {{{ Private */

/* {{{ zval_to_signum */
static zend_always_inline evutil_socket_t zval_to_signum(zval **ppzfd)
{
	evutil_socket_t fd;

	convert_to_long_ex(ppzfd);

	fd = Z_LVAL_PP(ppzfd);

	if (fd < 0 || fd >= NSIG) {
		return -1;
	}

	return fd;
}
/* }}} */


/* {{{ timer_cb */
static void timer_cb(evutil_socket_t fd, short what, void *arg)
{
	php_event_t *e = (php_event_t *) arg;
	zend_fcall_info     *pfci;
	zval                *arg_data;
	zval               **args[1];
	zval                *retval_ptr = NULL;
	PHP_EVENT_TSRM_DECL

	PHP_EVENT_ASSERT(e);
	PHP_EVENT_ASSERT(what & EV_TIMEOUT);
	PHP_EVENT_ASSERT(e->fci && e->fcc);

	pfci     = e->fci;
	arg_data = e->data;

	PHP_EVENT_TSRMLS_FETCH_FROM_CTX(e->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback arg */
		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[0] = &arg_data;

		/* Prepare callback */
		pfci->params		 = args;
		pfci->retval_ptr_ptr = &retval_ptr;
		pfci->param_count	 = 1;
		pfci->no_separation  = 1;

        if (zend_call_function(pfci, e->fcc TSRMLS_CC) == SUCCESS
                && retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "An error occurred while invoking the callback");
        }

        zval_ptr_dtor(&arg_data);
	}
}
/* }}} */

/* {{{ event_cb */
static void event_cb(evutil_socket_t fd, short what, void *arg)
{
	php_event_t *e = (php_event_t *) arg;
	zend_fcall_info     *pfci;
	zval                *arg_data;
	zval                *arg_fd;
	zval                *arg_what;
	zval               **args[3];
	zval                *retval_ptr = NULL;
	PHP_EVENT_TSRM_DECL

	PHP_EVENT_ASSERT(e);
	PHP_EVENT_ASSERT(e->fci && e->fcc);

	pfci       = e->fci;
	arg_data   = e->data;

	PHP_EVENT_TSRMLS_FETCH_FROM_CTX(e->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback arguments */
		MAKE_STD_ZVAL(arg_fd);
		if ((what & EV_SIGNAL) || e->stream_id == -1) {
			ZVAL_LONG(arg_fd, fd);
		} else if (e->stream_id >= 0) {
			ZVAL_RESOURCE(arg_fd, e->stream_id);
			zend_list_addref(e->stream_id);
		}
		args[0] = &arg_fd;

		MAKE_STD_ZVAL(arg_what);
		args[1] = &arg_what;
		ZVAL_LONG(arg_what, what);

		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[2] = &arg_data;

 		/* Prepare callback */
        pfci->params         = args;
        pfci->retval_ptr_ptr = &retval_ptr;
        pfci->param_count    = 3;
        pfci->no_separation  = 1;

        if (zend_call_function(pfci, e->fcc TSRMLS_CC) == SUCCESS
                && retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "An error occurred while invoking the callback");
        }

        zval_ptr_dtor(&arg_fd);
        zval_ptr_dtor(&arg_what);
        zval_ptr_dtor(&arg_data);
	}
}
/* }}} */

/* {{{ signal_cb */
static void signal_cb(evutil_socket_t signum, short what, void *arg)
{
	php_event_t *e = (php_event_t *) arg;
	zend_fcall_info     *pfci;
	zval                *arg_data;
	zval                *arg_signum;
	zval               **args[2];
	zval                *retval_ptr = NULL;
	PHP_EVENT_TSRM_DECL

	PHP_EVENT_ASSERT(e);
	PHP_EVENT_ASSERT(what & EV_SIGNAL);
	PHP_EVENT_ASSERT(e->fci && e->fcc);

	pfci     = e->fci;
	arg_data = e->data;

	PHP_EVENT_TSRMLS_FETCH_FROM_CTX(e->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback args */
		MAKE_STD_ZVAL(arg_signum);
		ZVAL_LONG(arg_signum, signum);
		args[0] = &arg_signum;

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

        if (zend_call_function(pfci, e->fcc TSRMLS_CC) == SUCCESS
                && retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "An error occurred while invoking the callback");
        }

        zval_ptr_dtor(&arg_data);
        zval_ptr_dtor(&arg_signum);
	}
}
/* }}} */

/* }}} */


/* {{{ proto Event Event::__construct(EventBase base, mixed fd, int what, callable cb[, zval arg = NULL]);
 * Creates new event */
PHP_METHOD(Event, __construct)
{
	zval                   *zself = getThis();
	zval                   *zbase;
	php_event_base_t       *b;
	zval                  **ppzfd;
	evutil_socket_t         fd;
	long                    what;
	zend_fcall_info         fci   = empty_fcall_info;
	zend_fcall_info_cache   fcc   = empty_fcall_info_cache;
	zval                   *arg   = NULL;
	php_event_t            *e;
	struct event           *event;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OZlf|z",
				&zbase, php_event_base_ce, &ppzfd, &what, &fci, &fcc, &arg) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	if (what & ~(EV_TIMEOUT | EV_READ | EV_WRITE | EV_SIGNAL | EV_PERSIST | EV_ET)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid mask");
		ZVAL_NULL(zself);
		return;
	}

	if (what & EV_SIGNAL) {
		fd = zval_to_signum(ppzfd);
		if (fd == -1) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid signal passed");
			ZVAL_NULL(zself);
			return;
		}
	} else if (what & EV_TIMEOUT) {
		fd = -1;
	} else {
		fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);
		if (fd < 0) {
			ZVAL_NULL(zself);
			return;
		}
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	/* TODO: check if a signum bound to different event bases */

	e = (php_event_t *) zend_object_store_get_object(zself TSRMLS_CC);

	event = event_new(b->base, fd, what, event_cb, (void *) e);
	if (!event) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "event_new failed");
		ZVAL_NULL(zself);
		return;
	}

	e->event = event;

	if (arg) {
		Z_ADDREF_P(arg);
	}
	e->data = arg;

	PHP_EVENT_COPY_FCALL_INFO(e->fci, e->fcc, &fci, &fcc);

	TSRMLS_SET_CTX(e->thread_ctx);

	if (what & EV_SIGNAL) {
		e->stream_id = -1; /* stdin fd = 0 */
	} else if (Z_TYPE_PP(ppzfd) == IS_RESOURCE) {
		/* lval of ppzfd is the resource ID */
		e->stream_id = Z_LVAL_PP(ppzfd);
		zend_list_addref(Z_LVAL_PP(ppzfd));
	} else {
		e->stream_id = -1;
	}
}
/* }}} */

/* {{{ proto void Event::free(void); */
PHP_METHOD(Event, free)
{
	zval        *zself = getThis();
	php_event_t *e;

	PHP_EVENT_FETCH_EVENT(e, zself);

	if (e->event) {
		/* No need in
		 * event_del(e->event);
		 * since event_free makes event non-pending internally */
		event_free(e->event);
		e->event = NULL;

		/*zval_ptr_dtor(&zself);*/
	}
}
/* }}} */

/* {{{ proto bool Event::set(EventBase base, mixed fd,[ int what = NULL[, callable cb = NULL[, zval arg = NULL]]]);
 *
 * Re-configures event.
 *
 * Note, this function doesn't invoke obsolete libevent's event_set. It calls event_assign instead.  */
PHP_METHOD(Event, set)
{
	zval                   *zbase;
	php_event_base_t       *b;
	zval                   *zevent  = getThis();
	php_event_t            *e;
	zval                  **ppzfd   = NULL;
	evutil_socket_t         fd = -1;
	long                    what    = -1;
	zend_fcall_info         fci     = empty_fcall_info;
	zend_fcall_info_cache   fcc     = empty_fcall_info_cache;
	zval                   *arg     = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OZ!|lfz!",
				&zbase, php_event_base_ce, &ppzfd,
				&what, &fci, &fcc, &arg) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	if (what != -1) {
		if (what & ~(EV_TIMEOUT | EV_READ | EV_WRITE | EV_SIGNAL | EV_PERSIST | EV_ET)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid events mask");
			RETURN_FALSE;
		}

		if (what & EV_SIGNAL) {
			if (zval_to_signum(ppzfd) == -1) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid signal passed");
				RETURN_FALSE;
			}
		} else {
			fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);
			if (fd < 0) {
				RETURN_FALSE;
			}
		}
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (php_event_is_pending(e->event)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't modify pending event");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	/* TODO: check if a signum bound to different event bases */

	if (ppzfd) {
		if (what != -1 && what & EV_SIGNAL) {
			e->stream_id = -1; /* stdin fd = 0 */
		} else if (Z_TYPE_PP(ppzfd) == IS_RESOURCE) {
			if (e->stream_id != Z_LVAL_PP(ppzfd)) {
				zend_list_delete(e->stream_id);
				/* lval of ppzfd is the resource ID */
				e->stream_id = Z_LVAL_PP(ppzfd);
				zend_list_addref(Z_LVAL_PP(ppzfd));
			}
		} else {
			e->stream_id = -1;
		}
	}

	if (ZEND_FCI_INITIALIZED(fci)) {
		PHP_EVENT_FREE_FCALL_INFO(e->fci, e->fcc);
		PHP_EVENT_COPY_FCALL_INFO(e->fci, e->fcc, &fci, &fcc);
	}

	if (arg) {
		if (e->data) {
			zval_ptr_dtor(&e->data);
		}
		e->data = arg;
		Z_ADDREF_P(arg);
	}

	event_get_assignment(e->event, &b->base,
			(ppzfd ? NULL : &fd),
			(short *) (what == -1 ? &what : NULL),
			NULL /* ignore old callback */ ,
			NULL /* ignore old callback argument */);

	if (event_assign(e->event, b->base, fd, what, event_cb, (void *) e)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto array Event::getSupportedMethods(void);
 * Returns array with of the names of the methods supported in this version of Libevent */
PHP_METHOD(Event, getSupportedMethods)
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
		add_next_index_string(return_value, methods[i], 1);
	}
}
/* }}} */

/* {{{ proto bool Event::add([double timeout]);
 * Make event pending. */
PHP_METHOD(Event, add)
{
	zval        *zevent = getThis();
	php_event_t *e;
	double       timeout = -1;
	int          res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|d",
				&timeout) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (timeout == -1) {
		res = event_add(e->event, NULL);
	} else {
		struct timeval tv;
		PHP_EVENT_TIMEVAL_SET(tv, timeout);

		res = event_add(e->event, &tv);
	}

	if (res) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed adding event");
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool Event::del(void);
 * Remove an event from the set of monitored events. */
PHP_METHOD(Event, del)
{
	zval        *zevent = getThis();
	php_event_t *e;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (e->event == NULL || event_del(e->event)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed deletting event");
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02010200
/* {{{ proto bool Event::removeTimer(void);
 * Remove a pending event’s timeout completely without deleting its IO or signal components.
 * Available since libevent 2.1.2-alpha. */
PHP_METHOD(Event, removeTimer)
{
	zval        *zevent = getThis();
	php_event_t *e;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (event_remove_timer(e->event)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed deletting event");
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */
#endif

/* {{{ proto bool Event::setPriority(int priority);
 * Set event priority. */
PHP_METHOD(Event, setPriority)
{
	zval        *zevent = getThis();
	php_event_t *e;
	long         priority;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&priority) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (event_priority_set(e->event, priority)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to set event priority: %ld", priority);
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool Event::pending(int flags);
 *  Detect whether event is pending or scheduled. */
PHP_METHOD(Event, pending)
{
	zval        *zevent = getThis();
	php_event_t *e;
	long         flags;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&flags) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (event_pending(e->event, flags, NULL)) {
		RETURN_TRUE;
	}
	RETVAL_FALSE;
}
/* }}} */


/* {{{ proto Event Event::timer(EventBase base, callable cb[, zval arg = NULL]);
 * Factory method for timer event */
PHP_METHOD(Event, timer)
{
	zval                  *zbase;
	php_event_base_t      *b;
	zend_fcall_info        fci   = empty_fcall_info;
	zend_fcall_info_cache  fcc   = empty_fcall_info_cache;
	zval                  *arg   = NULL;
	php_event_t           *e;
	struct event          *event;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Of|z",
				&zbase, php_event_base_ce, &fci, &fcc, &arg) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	PHP_EVENT_FETCH_BASE(b, zbase);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_ce);
	PHP_EVENT_FETCH_EVENT(e, return_value);

	event = evtimer_new(b->base, timer_cb, (void *) e);
	if (!event) {
		RETURN_FALSE;
	}

	e->event = event;

	if (arg) {
		Z_ADDREF_P(arg);
	}
	e->data = arg;

	PHP_EVENT_COPY_FCALL_INFO(e->fci, e->fcc, &fci, &fcc);

	TSRMLS_SET_CTX(e->thread_ctx);

	e->stream_id = -1; /* stdin fd = 0 */
}
/* }}} */

/* {{{ proto bool Event::setTimer(EventBase base, callable cb[, zval arg = NULL]);
 * Re-configures timer event.
 * Note, this function doesn't invoke obsolete libevent's event_set. It calls event_assign instead. */
PHP_METHOD(Event, setTimer)
{
	zval                  *zbase;
	php_event_base_t      *b;
	zval                  *zevent = getThis();
	php_event_t           *e;
	zend_fcall_info        fci    = empty_fcall_info;
	zend_fcall_info_cache  fcc    = empty_fcall_info_cache;
	zval                  *arg    = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Of|z!",
				&zbase, php_event_base_ce,
				&fci, &fcc, &arg) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (evtimer_pending(e->event, NULL)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't modify pending timer");
		RETURN_FALSE;
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (ZEND_FCI_INITIALIZED(fci)) {
		if (e->fci && ZEND_FCI_INITIALIZED(*e->fci)) {
			PHP_EVENT_FREE_FCALL_INFO(e->fci, e->fcc);
		}

		PHP_EVENT_COPY_FCALL_INFO(e->fci, e->fcc, &fci, &fcc);
	}

	if (arg) {
		if (e->data) {
			zval_ptr_dtor(&e->data);
		}
		e->data = arg;
		Z_ADDREF_P(arg);
	}

	e->stream_id = -1; /* stdin fd = 0 */

    if (evtimer_assign(e->event, b->base, timer_cb, (void *) e)) {
    	RETURN_FALSE;
    }
    RETVAL_TRUE;
}
/* }}} */

/* {{{ proto Event signal(EventBase base, int signum, callable cb[, zval arg = NULL]);
 * Factory method for signal event */
PHP_METHOD(Event, signal)
{
	zval                  *zbase;
	php_event_base_t      *b;
	long                   signum;
	zend_fcall_info        fci    = empty_fcall_info;
	zend_fcall_info_cache  fcc    = empty_fcall_info_cache;
	zval                  *arg    = NULL;
	php_event_t           *e;
	struct event          *event;


	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Olf|z",
				&zbase, php_event_base_ce, &signum, &fci, &fcc, &arg) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	if (signum < 0 || signum >= NSIG) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid signal passed");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_ce);
	PHP_EVENT_FETCH_EVENT(e, return_value);

	event = evsignal_new(b->base, signum, signal_cb, (void *) e);
	if (!event) {
		RETURN_FALSE;
	}

	e->event = event;

	if (arg) {
		Z_ADDREF_P(arg);
	}
	e->data = arg;

	PHP_EVENT_COPY_FCALL_INFO(e->fci, e->fcc, &fci, &fcc);

	TSRMLS_SET_CTX(e->thread_ctx);

	e->stream_id = -1; /* stdin fd = 0 */
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
