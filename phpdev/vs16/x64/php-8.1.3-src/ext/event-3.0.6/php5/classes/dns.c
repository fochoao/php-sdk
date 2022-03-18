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

/* {{{ proto EventDnsBase EventDnsBase::__construct(EventBase base, bool initialize);
 *
 * Returns object representing event dns base.
 *
 * If the initialize argument is true, it tries to configure the DNS base
 * sensibly given your operating system’s default. Otherwise, it leaves the
 * event dns base empty, with no nameservers or options configured. In the latter
 * case you should configure dns base yourself, e.g. with
 * EventDnsBase::parseResolvConf() */
PHP_METHOD(EventDnsBase, __construct)
{
	php_event_base_t     *base;
	zval                 *zbase;
	php_event_dns_base_t *dnsb;
	zend_bool             initialize;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ob",
				&zbase, php_event_base_ce, &initialize) == FAILURE) {
		return;
	}

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	PHP_EVENT_FETCH_BASE(base, zbase);

	PHP_EVENT_FETCH_DNS_BASE(dnsb, getThis());

	dnsb->dns_base = evdns_base_new(base->base, initialize);
}
/* }}} */

/* {{{ proto bool EventDnsBase::parseResolvConf(int flags, string filename);
 * Scans the resolv.conf formatted file stored in filename, and read in all the
 * options from it that are listed in flags */
PHP_METHOD(EventDnsBase, parseResolvConf)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = getThis();
	long                  flags;
	char                 *filename;
	int                   filename_len;
	int                   ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls",
				&flags, &filename, &filename_len) == FAILURE) {
		return;
	}

	if (flags & ~(DNS_OPTION_NAMESERVERS | DNS_OPTION_SEARCH | DNS_OPTION_MISC
				| DNS_OPTIONS_ALL)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid flags");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	ret = evdns_base_resolv_conf_parse(dnsb->dns_base, flags, filename);

	if (ret) {
		char err[40];

		switch (ret) {
			case 1:
				strcpy(err, "Failed to open file");
				break;
			case 2:
				strcpy(err, "Failed to stat file");
				break;
			case 3:
				strcpy(err, "File too large");
				break;
			case 4:
				strcpy(err, "Out of memory");
				break;
			case 5:
				strcpy(err, "Short read from file");
				break;
			case 6:
				strcpy(err, "No nameservers listed in the file");
				break;
		}

		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", err);
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventDnsBase::addNameserverIp(string ip);
 * Adds a nameserver to an existing evdns_base. It takes the nameserver in a
 * text string, either as an IPv4 address, an IPv6 address, an IPv4 address
 * with a port (IPv4:Port), or an IPv6 address with a port ([IPv6]:Port).
 */
PHP_METHOD(EventDnsBase, addNameserverIp)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = getThis();
	char                 *ip;
	int                   ip_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&ip, &ip_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	if (evdns_base_nameserver_ip_add(dnsb->dns_base, ip)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventDnsBase::loadHosts(string hosts);
 *  Loads a hosts file (in the same format as /etc/hosts) from hosts file
 */
PHP_METHOD(EventDnsBase, loadHosts)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = getThis();
	char                 *hosts;
	int                   hosts_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&hosts, &hosts_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	if (evdns_base_load_hosts(dnsb->dns_base, hosts)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto void EventDnsBase::clearSearch(void);
 * Removes all current search suffixes (as configured by the search option)
 * from the evdns_base; the evdns_base_search_add() function adds a suffix
 */
PHP_METHOD(EventDnsBase, clearSearch)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = getThis();

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	evdns_base_search_clear(dnsb->dns_base);
}
/* }}} */

/* {{{ proto void EventDnsBase::addSearch(string domain);
 */
PHP_METHOD(EventDnsBase, addSearch)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = getThis();
	char                 *domain;
	int                   domain_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&domain, &domain_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	evdns_base_search_add(dnsb->dns_base, domain);
}
/* }}} */

/* {{{ proto void EventDnsBase::setSearchNdots(int ndots);
 */
PHP_METHOD(EventDnsBase, setSearchNdots)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = getThis();
	long                  ndots;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&ndots) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	evdns_base_search_ndots_set(dnsb->dns_base, ndots);
}
/* }}} */

/* {{{ proto bool EventDnsBase::setOption(string option, string value);
 */
PHP_METHOD(EventDnsBase, setOption)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = getThis();
	char                 *option;
	int                   option_len;
	char                 *value;
	int                   value_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
				&option, &option_len, &value, &value_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	if (evdns_base_set_option(dnsb->dns_base, option, value)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int EventDnsBase::countNameservers(void);
 */
PHP_METHOD(EventDnsBase, countNameservers)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = getThis();

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	RETURN_LONG(evdns_base_count_nameservers(dnsb->dns_base));
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
