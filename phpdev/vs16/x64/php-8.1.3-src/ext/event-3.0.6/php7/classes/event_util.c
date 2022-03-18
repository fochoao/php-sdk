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
#ifndef PHP_WIN32
# include <fcntl.h>
#endif

PHP_METHOD(EventUtil, __construct)
{
	zend_throw_exception(NULL, "An object of this type cannot be created "
			"with the new operator", 0 );
}

/* {{{ proto int EventUtil::getLastSocketErrno([mixed socket = null]);
 *
 * Returns the most recent socket error number(errno). */
PHP_METHOD(EventUtil, getLastSocketErrno)
{
	zval *pzfd = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|z!", &pzfd) == FAILURE) {
		return;
	}

	if (pzfd) {
		evutil_socket_t fd = (evutil_socket_t)php_event_zval_to_fd(pzfd);

		if (fd < 0) {
			RETURN_FALSE;
		}

		RETVAL_LONG(evutil_socket_geterror(fd));
	} else {
		RETVAL_LONG(EVUTIL_SOCKET_ERROR());
	}
}
/* }}} */

/* {{{ proto string EventUtil::getLastSocketError([resource socket = null]);
 *
 * Returns the most recent socket error */
PHP_METHOD(EventUtil, getLastSocketError)
{
	zval *pzfd = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|z!", &pzfd) == FAILURE) {
		return;
	}

	if (pzfd) {
		evutil_socket_t fd = (evutil_socket_t)php_event_zval_to_fd(pzfd);

		if (fd < 0) {
			RETURN_FALSE;
		}

		RETVAL_STRING(evutil_socket_error_to_string(evutil_socket_geterror(fd)));
	} else {
		RETVAL_STRING(evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
	}
}
/* }}} */

#ifdef HAVE_EVENT_OPENSSL_LIB/* {{{ */
/* {{{ proto bool EventUtil::sslRandPoll(void);
 *
 * Generates entropy by means of OpenSSL's RAND_poll()
 */
PHP_METHOD(EventUtil, sslRandPoll)
{
	RETVAL_BOOL((zend_bool) RAND_poll());
}
/* }}} */
#endif/* }}} */

/* {{{ proto bool EventUtil::getSocketName(mixed socket, string &address[, int &port]);
 * Retreives the current address to which the <parameter>socket</parameter> is bound.
 *
 * <parameter>socket</parameter> may be a stream or socket resource, or a numeric file descriptor
 * associated with a socket.
 *
 * Returns &true; on success. Otherwise &false;.*/
PHP_METHOD(EventUtil, getSocketName)
{
	zval            *pzfd;
	zval            *zaddress;
	zval            *zport    = NULL;
	evutil_socket_t  fd;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "zz|z",
				&pzfd, &zaddress, &zport) == FAILURE) {
		return;
	}

	fd = (evutil_socket_t)php_event_zval_to_fd(pzfd);
	if (fd < 0) {
		RETURN_FALSE;
	}

	if (_php_event_getsockname(fd, zaddress, zport) == FAILURE) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventUtil::setSocketOption(mixed socket, int level, int optname, int|array optval)
   Sets socket options for the socket */
PHP_METHOD(EventUtil, setSocketOption)
{
	zval            *zfd;
	zval            *zoptval;
	zend_long        level;
	zend_long        optname;
	evutil_socket_t  fd;
	void            *opt_ptr;
	HashTable       *opt_ht;
	int              ov;
	socklen_t        optlen;
	int              retval;
	struct linger    lv;
	struct timeval   tv;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "zllz",
				&zfd, &level, &optname, &zoptval) == FAILURE) {
		return;
	}

	fd = php_event_zval_to_fd(zfd);
	if (fd == -1) {
		RETURN_FALSE;
	}

	switch (optname) {
		case SO_LINGER:
			{
				zval          *l_onoff;
				zval          *l_linger;

				convert_to_array_ex(zoptval);
				opt_ht = HASH_OF(zoptval);

				if ((l_onoff = zend_hash_str_find(opt_ht, "l_onoff", sizeof("l_onoff") - 1)) == NULL) {
					php_error_docref(NULL, E_WARNING, "no key \"l_onoff\" passed in optval");
					RETURN_FALSE;
				}

				if ((l_linger = zend_hash_str_find(opt_ht, "l_linger", sizeof("l_linger") - 1)) == NULL) {
					php_error_docref(NULL, E_WARNING, "no key \"l_linger\" passed in optval");
					RETURN_FALSE;
				}

				convert_to_long_ex(l_onoff);
				convert_to_long_ex(l_linger);

				lv.l_onoff  = (unsigned short)Z_LVAL_P(l_onoff);
				lv.l_linger = (unsigned short)Z_LVAL_P(l_linger);

				optlen = sizeof(lv);
				opt_ptr = &lv;
				break;
			}

		case SO_RCVTIMEO:
		case SO_SNDTIMEO:
			{
				zval           *sec;
				zval           *usec;

				convert_to_array_ex(zoptval);
				opt_ht = HASH_OF(zoptval);

				if ((sec = zend_hash_str_find(opt_ht, "sec", sizeof("sec") - 1)) == NULL) {
					php_error_docref(NULL, E_WARNING, "no key \"sec\" passed in optval");
					RETURN_FALSE;
				}

				if ((usec = zend_hash_str_find(opt_ht, "usec", sizeof("usec") - 1)) == NULL) {
					php_error_docref(NULL, E_WARNING, "no key \"usec\" passed in optval");
					RETURN_FALSE;
				}

				convert_to_long_ex(sec);
				convert_to_long_ex(usec);

				tv.tv_sec  = Z_LVAL_P(sec);
				tv.tv_usec = Z_LVAL_P(usec);

				optlen  = sizeof(tv);
				opt_ptr = &tv;
				break;
			}

		default:
			convert_to_long_ex(zoptval);
			ov = Z_LVAL_P(zoptval);

			optlen = sizeof(ov);
			opt_ptr = &ov;
			break;
	}

	retval = setsockopt(fd, level, optname, opt_ptr, optlen);

	if (retval != 0) {
		if (retval != -2) { /* error, but message already emitted */
			php_error_docref(NULL, E_WARNING, "Unable to set socket option, errno: %d", errno);
		}

		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventUtil::getSocketFd(mixed socket)
 *    Gets numeric file descriptor of a socket. */
PHP_METHOD(EventUtil, getSocketFd) {
	zval *pzfd = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &pzfd) == FAILURE) {
		return;
	}

	RETVAL_LONG(pzfd ? php_event_zval_to_fd(pzfd) : -1);
}
/* }}} */

#ifdef PHP_EVENT_SOCKETS_SUPPORT
/* {{{ proto resource EventUtil::createSocket(int fd)
 *    Creates socket resource from a numeric file descriptor. */
PHP_METHOD(EventUtil, createSocket) {
	zend_long  fd;
	php_socket *php_sock;
	socklen_t  opt_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &fd) == FAILURE) {
		return;
	}

	if (!ZEND_VALID_SOCKET(fd)) {
		php_error_docref(NULL, E_WARNING, "Invalid file descriptor");
		RETURN_FALSE;
	}

	php_sock = php_create_socket();
	php_sock->bsd_socket = fd;

	opt_length = sizeof(php_sock->type);
	if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &php_sock->type, &opt_length) != 0) {
		php_error_docref(NULL, E_WARNING, "Unable to retrieve socket type");
		RETURN_FALSE;
	}

#ifndef PHP_WIN32
	php_sock->blocking = (fcntl(fd, F_GETFL) & O_NONBLOCK) == 0 ? 0 : 1;
#else
	php_sock->blocking = 1;
#endif

	RETURN_RES(zend_register_resource(php_sock, php_sockets_le_socket()));
} /* }}} */
#endif /* PHP_EVENT_SOCKETS_SUPPORT */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
