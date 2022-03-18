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
#include "zend_exceptions.h"

/* {{{ proto EventBase EventBase::__construct([EventConfig cfg = null]); */
PHP_METHOD(EventBase, __construct)
{
	php_event_base_t   *b;
	php_event_config_t *cfg;
	zval               *zcfg = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|O!",
				&zcfg, php_event_config_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, getThis());

	if (zcfg == NULL) {
		b->base = event_base_new();
	} else {
		PHP_EVENT_FETCH_CONFIG(cfg, zcfg);

		b->base = event_base_new_with_config(cfg->ptr);
		if (!b->base) {
			zend_throw_exception_ex(php_event_get_exception(), 0 TSRMLS_CC,
					"EventBase cannot be constructed with the provided configuration. "
					"Make sure that the specified features are supported on the current platform.");
		}
	}
}
/* }}} */

/* {{{ proto int EventBase::__wakeup()
   Prevents use of a EventBase instance that has been unserialized */
PHP_METHOD(EventBase, __wakeup)
{
	zend_throw_exception_ex(php_event_get_exception(), 0 TSRMLS_CC, "EventBase instances are not serializable");
}
/* }}} */

/* {{{ proto int EventBase::__sleep()
   Prevents serialization of a EventBase instance */
PHP_METHOD(EventBase, __sleep)
{
	zend_throw_exception_ex(php_event_get_exception(), 0 TSRMLS_CC, "EventBase instances are not serializable");
}
/* }}} */

/* {{{ proto void EventBase::free(void); */
PHP_METHOD(EventBase, free)
{
	zval *zbase = getThis();
	php_event_base_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (b->base) {
		event_base_free(b->base);
		b->base=NULL;
	}
}
/* }}} */

/* {{{ proto string EventBase::getMethod(void);
 * Returns event method in use. */
PHP_METHOD(EventBase, getMethod)
{
	zval             *zbase = getThis();
	php_event_base_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	RETURN_STRING((char *)event_base_get_method(b->base), 1);
}
/* }}} */

/* {{{ proto int EventBase::getFeatures(void);
 * Returns bitmask of features supported. See EVENT_FEATURE_* constants. */
PHP_METHOD(EventBase, getFeatures)
{
	zval             *zbase = getThis();
	php_event_base_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	RETVAL_LONG(event_base_get_features(b->base));
}
/* }}} */

/* {{{ proto bool EventBase::priorityInit(int n_priorities);
 * Sets number of priorities per event base. Returns &true; on success, otherwise &false; */
PHP_METHOD(EventBase, priorityInit)
{
	zval             *zbase = getThis();
	long              n_priorities;
	php_event_base_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&n_priorities) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_priority_init(b->base, n_priorities)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBase::loop([int flags]);
 * Wait for events to become active, and run their callbacks. */
PHP_METHOD(EventBase, loop)
{
	zval             *zbase = getThis();
	long              flags = -1;
	php_event_base_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l",
				&flags) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	/* Call event_base_dispatch when flags omitted. */
	if (flags == -1) {
		if (event_base_dispatch(b->base) == -1) {
			RETURN_FALSE;
		}
	} else if (event_base_loop(b->base, flags) == -1) {
		RETURN_FALSE;
	}

	if (EG(exception)) {
		zend_throw_exception_object(EG(exception) TSRMLS_CC);
	}
	/* Since 2.1.2-alpha we can call event_base_loopcontinue(b->base);*/

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBase::dispatch(void);
 * Wait for events to become active, and run their callbacks.
 * The same as EventBase::loop() with no flags set*/
PHP_METHOD(EventBase, dispatch)
{
	zval             *zbase = getThis();
	php_event_base_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_dispatch(b->base) == -1) {
		RETURN_FALSE;
	}

	if (EG(exception)) {
		zend_throw_exception_object(EG(exception) TSRMLS_CC);
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBase::exit([double timeout = 0.0]);
 * Tells event_base to stop optionally after given number of seconds. */
PHP_METHOD(EventBase, exit)
{
	zval             *zbase = getThis();
	php_event_base_t *b;
	double            timeout = -1;
	int               res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|d",
				&timeout) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (timeout == -1) {
		res = event_base_loopexit(b->base, NULL);
	} else {
		struct timeval tv;
		PHP_EVENT_TIMEVAL_SET(tv, timeout);

		res = event_base_loopexit(b->base, &tv);
	}

	if (res) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBase::stop(void);
 * Tells event_base to stop. */
PHP_METHOD(EventBase, stop)
{
	zval             *zbase = getThis();
	php_event_base_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_loopbreak(b->base)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBase::set(Event event);
 * Associate event base with an event. */
PHP_METHOD(EventBase, set)
{
	zval             *zbase = getThis();
	php_event_base_t *b;
	zval             *zevent;
	php_event_t      *e;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&zevent, php_event_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (php_event_is_pending(e->event)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't modify pending event");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_set(b->base, e->event)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBase::gotStop(void);
 *
 * Checks if the event loop was told to abort immediately by EventBase::stop() */
PHP_METHOD(EventBase, gotStop)
{
	zval             *zbase = getThis();
	php_event_base_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_got_break(b->base)) {
		RETURN_TRUE;
	}
	RETVAL_FALSE;
}
/* }}} */

/* {{{ proto bool EventBase::gotExit(void);
 * Checks if the event loop was told to exit by EventBase::exit */
PHP_METHOD(EventBase, gotExit)
{
	zval             *zbase = getThis();
	php_event_base_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_got_exit(b->base)) {
		RETURN_TRUE;
	}
	RETVAL_FALSE;
}
/* }}} */

/* {{{ proto double EventBase::getTimeOfDayCached(void);
 *
 * On success returns the current time(as returned by gettimeofday()), looking
 * at the cached value in 'base' if possible, and calling gettimeofday() or
 * clock_gettime() as appropriate if there is no cached time. On failure
 * returns NULL. */
PHP_METHOD(EventBase, getTimeOfDayCached)
{
	zval             *zbase = getThis();
	php_event_base_t *b;
	struct timeval    tv;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_gettimeofday_cached(b->base, &tv)) {
		RETURN_NULL();
	}

	RETVAL_DOUBLE(PHP_EVENT_TIMEVAL_TO_DOUBLE(tv));
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02010100
/* {{{ proto bool EventBase::updateCacheTime(void);
 * Updates cache time. Available since libevent 2.1.1-alpha */
PHP_METHOD(EventBase, updateCacheTime)
{
	zval             *zbase = getThis();
	php_event_base_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_update_cache_time(b->base)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */
#endif

/* {{{ proto bool EventBase::reInit(void);
 * Re-initialize event base. Should be called after a fork.
 * XXX pthread_atfork() in MINIT */
PHP_METHOD(EventBase, reInit)
{
	zval             *zbase = getThis();
	php_event_base_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_reinit(b->base)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02010200
/* {{{ proto bool EventBase::resume(void);
 * Tells event_base to resume previously stopped event.
 * Available since libevent version 2.1.2-alpha. */
PHP_METHOD(EventBase, resume)
{
	zval             *zbase = getThis();
	php_event_base_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_loopcontinue(b->base)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
