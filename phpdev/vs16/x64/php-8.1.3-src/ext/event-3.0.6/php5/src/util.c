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

#include "common.h"
#include "util.h"
#ifndef PHP_WIN32
# include <fcntl.h>
#endif

/* {{{ php_event_zval_to_fd
 * Get numeric file descriptor from PHP stream or Socket resource */
php_socket_t php_event_zval_to_fd(zval **ppfd TSRMLS_DC)
{
	php_socket_t  file_desc = -1;
	php_stream   *stream;
	FILE *fp = NULL;
#ifdef PHP_EVENT_SOCKETS_SUPPORT
	php_socket   *php_sock;
#endif

	if (Z_TYPE_PP(ppfd) == IS_RESOURCE) {
		/* PHP stream or PHP socket resource  */
		if (ZEND_FETCH_RESOURCE_NO_RETURN(stream, php_stream *, ppfd, -1, NULL, php_file_le_stream())
				|| ZEND_FETCH_RESOURCE_NO_RETURN(stream, php_stream *, ppfd, -1, NULL, php_file_le_pstream()))
		{
			php_stream_from_zval_no_verify(stream, ppfd);

			if (stream == NULL) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed obtaining fd");
				return -1;
			}

			/* PHP stream */
			if (php_stream_can_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL) == SUCCESS) {
				if (php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT,
							(void*) &file_desc, 1) != SUCCESS || file_desc < 0) {
					return -1;
				}
			} else if (php_stream_can_cast(stream, PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL) == SUCCESS) {
				if (php_stream_cast(stream, PHP_STREAM_AS_FD,
							(void*) &file_desc, 1) != SUCCESS || file_desc < 0) {
					return -1;
				}
			} else if (php_stream_can_cast(stream, PHP_STREAM_AS_STDIO | PHP_STREAM_CAST_INTERNAL) == SUCCESS) {
				if (php_stream_cast(stream, PHP_STREAM_AS_STDIO | PHP_STREAM_CAST_INTERNAL,
							(void**) &fp, 1) != SUCCESS) {
					return -1;
				}
				file_desc = fileno(fp);
			} else { /* STDIN, STDOUT, STDERR etc. */
				file_desc = Z_LVAL_P(*ppfd);
			}
		} else {
			/* PHP socket resource */
#ifdef PHP_EVENT_SOCKETS_SUPPORT
			if (ZEND_FETCH_RESOURCE_NO_RETURN(php_sock, php_socket *,ppfd, -1, NULL, php_sockets_le_socket())) {
				if (php_sock->error) {
					if (!php_sock->blocking && php_sock->error == EINPROGRESS) {
#ifdef PHP_EVENT_DEBUG
						php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Operation in progress");
#endif
					} else
						return -1;
				}

				return php_sock->bsd_socket;
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"either valid PHP stream or valid PHP socket resource expected");
			}
#else
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"valid PHP stream resource expected");
#endif
			return -1;
		}
	} else if (Z_TYPE_PP(ppfd) == IS_LONG) {
		/* Numeric fd */
		file_desc = Z_LVAL_PP(ppfd);
		if (file_desc < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid file descriptor passed");
			return -1;
		}
	} else {
		/* Invalid fd */
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid file descriptor passed");
		return -1;
	}

	/* Validate file descriptor */
#ifndef PHP_WIN32
	if (file_desc >= 0 && fcntl(file_desc, F_GETFD) == -1) {
#else
	if (file_desc == INVALID_SOCKET) {
#endif
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "fcntl: invalid file descriptor passed");
		return -1;
	}

	return file_desc;
}
/* }}} */

/* {{{ _php_event_getsockname */
int _php_event_getsockname(evutil_socket_t fd, zval **ppzaddress, zval **ppzport TSRMLS_DC)
{
	php_sockaddr_storage  sa_storage;
	struct sockaddr      *sa         = (struct sockaddr *) &sa_storage;
	socklen_t             sa_len     = sizeof(php_sockaddr_storage);
	long                  port       = -1;

	if (getsockname(fd, sa, &sa_len)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Unable to retreive socket name, errno: %d", errno);
		return FAILURE;
	}

	switch (sa->sa_family) {
		case AF_INET:
			{
				struct sockaddr_in *sin = (struct sockaddr_in *) sa;
				char addr[INET_ADDRSTRLEN + 1];

				if (evutil_inet_ntop(sa->sa_family, &sin->sin_addr,
							(void *) &addr, sizeof(addr))) {
					if (*ppzaddress) {
						zval_dtor(*ppzaddress);
					}
					ZVAL_STRING(*ppzaddress, addr, 1);

					if (*ppzport != NULL) {
						port = ntohs(sin->sin_port);
					}
				}
			}
			break;
#if HAVE_IPV6
		case AF_INET6:
			{
				struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
				char addr6[INET6_ADDRSTRLEN + 1];

				if (evutil_inet_ntop(sa->sa_family, &sin6->sin6_addr,
							(void *) &addr6, sizeof(addr6))) {
					if (*ppzaddress) {
						zval_dtor(*ppzaddress);
					}
					ZVAL_STRING(*ppzaddress, addr6, 1);

					if (*ppzport != NULL) {
						port = ntohs(sin6->sin6_port);
					}
				}
			}
			break;
#endif
#ifdef AF_UNIX
		case AF_UNIX:
			{
				struct sockaddr_un *ua = (struct sockaddr_un *) sa;

				if (*ppzaddress) {
					zval_dtor(*ppzaddress);
				}
				ZVAL_STRING(*ppzaddress, ua->sun_path, 1);
			}
			break;
#endif
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Unsupported address family: %d", sa->sa_family);
			return FAILURE;
	}

	if (port != -1) {
		if (*ppzport) {
			zval_dtor(*ppzport);
		}
		ZVAL_LONG(*ppzport, port);
	}

	return SUCCESS;
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
