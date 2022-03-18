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
#ifndef PHP_EVENT_COMMON_H
#define PHP_EVENT_COMMON_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <php_network.h>
#include <php_streams.h>

#if !defined(PHP_WIN32) && defined(AF_UNIX)
# include <sys/un.h>
#endif

#include <signal.h>

#ifdef PHP_EVENT_SOCKETS
# include "ext/sockets/php_sockets.h"
# define PHP_EVENT_SOCKETS_SUPPORT
#endif

/* zend_fcall_info.symbol_table removed from PHP 7.1.x */
#if PHP_VERSION_ID < 70100
# define HAVE_PHP_ZEND_FCALL_INFO_SYMBOL_TABLE 1
# define HAVE_PHP_ZEND_FCALL_INFO_FUNCTION_TABLE 1
#endif

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>

#ifdef HAVE_EVENT_PTHREADS_LIB
# include <event2/thread.h>
#endif

#ifdef HAVE_EVENT_EXTRA_LIB
# include <event2/keyvalq_struct.h>
# include <event2/listener.h>
# include <event2/dns.h>
# include <event2/http.h>
# include <event2/rpc.h>
# include <event2/tag.h>
#endif

#ifdef HAVE_EVENT_OPENSSL_LIB
# include <event2/bufferevent_ssl.h>
#endif

#if !defined(_MINIX)
# include <pthread.h>
#endif

#ifdef ZTS
# include <TSRM.h>
#endif

#if !defined(LIBEVENT_VERSION_NUMBER) || LIBEVENT_VERSION_NUMBER < 0x02000200
# error "This version of Libevent is not supported; get 2.0.2-alpha or later."
#endif

#ifdef HAVE_EVENT_OPENSSL_LIB
# include <openssl/evp.h>
# include <openssl/x509.h>
# include <openssl/x509v3.h>
# include <openssl/crypto.h>
# include <openssl/pem.h>
# include <openssl/err.h>
# include <openssl/conf.h>
# include <openssl/rand.h>
# include <openssl/ssl.h>
# include <openssl/pkcs12.h>
# if !defined(OPENSSL_NO_SSL2) && OPENSSL_VERSION_NUMBER < 0x10100000L
#  define HAVE_SSL2 1
# endif
# ifndef OPENSSL_NO_SSL3
#  define HAVE_SSL3 1
# endif
#endif /* HAVE_EVENT_OPENSSL_LIB */

#include "../php_event.h"
#include "structs.h"

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
