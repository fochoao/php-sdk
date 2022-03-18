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

/* {{{ proto EventConfig::__construct(void);
 * On success returns an object representing an event configuration
 * which can be passed to EventBase::__construct. */
PHP_EVENT_METHOD(EventConfig, __construct)
{
	php_event_config_t *cfg;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	cfg = Z_EVENT_CONFIG_OBJ_P(getThis());

	cfg->ptr = event_config_new();
}
/* }}} */

/*{{{ proto int EventConfig::__sleep */
PHP_EVENT_METHOD(EventConfig, __sleep)
{
	zend_throw_exception_ex(php_event_get_exception(), 0, "EventConfig instances are not serializable");
}
/*}}}*/

/*{{{ proto int EventConfig::__wakeup */
PHP_EVENT_METHOD(EventConfig, __wakeup)
{
	zend_throw_exception_ex(php_event_get_exception(), 0, "EventConfig instances are not serializable");
}
/*}}}*/

/* {{{ proto bool EventConfig::avoidMethod(string method);
 * Tells libevent to avoid specific event method.
 * See http://www.wangafu.net/~nickm/libevent-book/Ref2_eventbase.html#_creating_an_event_base
 * Returns &true; on success, otherwise &false;.*/
PHP_EVENT_METHOD(EventConfig, avoidMethod)
{
	zval               *zcfg       = getThis();
	char               *method;
	size_t              method_len;
	php_event_config_t *cfg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s",
				&method, &method_len) == FAILURE) {
		return;
	}

	cfg = Z_EVENT_CONFIG_OBJ_P(zcfg);

	if (event_config_avoid_method(cfg->ptr, method)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventConfig::requireFeatures(int feature);
 * Enters a required event method feature that the application demands. */
PHP_EVENT_METHOD(EventConfig, requireFeatures)
{
	zval               *zcfg    = getThis();
	zend_long               feature;
	php_event_config_t *cfg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&feature) == FAILURE) {
		return;
	}

	cfg = Z_EVENT_CONFIG_OBJ_P(zcfg);

	if (event_config_require_features(cfg->ptr, feature)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02010000
/* {{{ proto void EventConfig::setMaxDispatchInterval(int max_interval, int max_callbacks, int min_priority);
 * Prevents priority inversion by limiting how many low-priority event
 * callbacks can be invoked before checking for more high-priority events.
 * Available since libevent 2.1.0-alpha. */
PHP_EVENT_METHOD(EventConfig, setMaxDispatchInterval)
{
	zval                  *zcfg          = getThis();
	php_event_timestamp_t  max_interval;
	zend_long                  max_callbacks;
	zend_long                  min_priority;
	php_event_config_t    *cfg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "dll",
				&max_interval, &max_callbacks, &min_priority) == FAILURE) {
		return;
	}

	cfg = Z_EVENT_CONFIG_OBJ_P(zcfg);

	if (max_interval > 0) {
		struct timeval tv;
		PHP_EVENT_TIMEVAL_SET(tv, max_interval);

		event_config_set_max_dispatch_interval(cfg->ptr, &tv, max_callbacks, min_priority);
	} else {
		event_config_set_max_dispatch_interval(cfg->ptr, NULL, max_callbacks, min_priority);
	}
}
/* }}} */
#endif

#if LIBEVENT_VERSION_NUMBER >= 0x02000201 /* 2.0.2-alpha */
/*{{{ proto bool EventConfig::setFlags(int flags)
 * Sets one or more flags to configure what parts of the eventual EventBase
 * will be initialized, and how they'll work. */
PHP_EVENT_METHOD(EventConfig, setFlags)
{
	zend_long           flags;
	php_event_config_t *cfg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &flags) == FAILURE) {
		return;
	}

	cfg = Z_EVENT_CONFIG_OBJ_P(getThis());

	if (!event_config_set_flag(cfg->ptr, flags)) {
		RETURN_TRUE;
	}

	RETVAL_FALSE;
}
/*}}}*/
#endif /* 2.0.2-alpha */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
