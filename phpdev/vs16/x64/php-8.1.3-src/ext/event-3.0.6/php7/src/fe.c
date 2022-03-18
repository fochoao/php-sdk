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

#include "fe.h"
#include "priv.h"

#ifdef ZEND_TYPE_ENCODE_CLASS_CONST
# define PHP_EVENT_ARG_OBJ_INFO(pass_by_ref, name, classname, allow_null) \
	{ #name, ZEND_TYPE_ENCODE_CLASS_CONST(PHP_EVENT_NS_NAME(classname), allow_null), pass_by_ref, 0 },
#else
# define PHP_EVENT_ARG_OBJ_INFO(pass_by_ref, name, classname, allow_null) \
	{ #name, PHP_EVENT_NS_NAME(classname), IS_OBJECT, pass_by_ref, allow_null, 0 },
#endif

/* {{{ ARGINFO */
ZEND_BEGIN_ARG_INFO(arginfo_event__void, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_base__construct, 0, 0, 0)
	ZEND_ARG_INFO(0, cfg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_base_priority_init, 0, 0, 1)
	ZEND_ARG_INFO(0, n_priorities)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_base_loop, 0, 0, 0)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_base_loopexit, 0, 0, 0)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event__construct, 0, 0, 4)
	PHP_EVENT_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, what)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_set, 0, 0, 2)
	PHP_EVENT_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, what)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_add, 0, 0, 0)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_config_avoid_method, 0, 0, 1)
	ZEND_ARG_INFO(0, method)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_config_require_features, 0, 0, 1)
	ZEND_ARG_INFO(0, feature)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_config_set_flags, 0, 0, 1)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_priority_set, 0, 0, 1)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_pending, 0, 0, 1)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evtimer_new, 0, 0, 2)
	PHP_EVENT_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evtimer_set, 0, 0, 2)
	PHP_EVENT_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();


ZEND_BEGIN_ARG_INFO_EX(arginfo_evsignal_new, 0, 0, 3)
	PHP_EVENT_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_INFO(0, signum)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();


#if LIBEVENT_VERSION_NUMBER >= 0x02010000
ZEND_BEGIN_ARG_INFO_EX(arginfo_event_config_set_max_dispatch_interval, 0, 0, 3)
	ZEND_ARG_INFO(0, max_interval)
	ZEND_ARG_INFO(0, max_callbacks)
	ZEND_ARG_INFO(0, min_priority)
ZEND_END_ARG_INFO();
#endif


ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent__events, 0, 0, 1)
	ZEND_ARG_INFO(0, events)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent__construct, 0, 0, 1)
	PHP_EVENT_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_INFO(0, socket)
	ZEND_ARG_INFO(0, options)
	ZEND_ARG_INFO(0, readcb)
	ZEND_ARG_INFO(0, writecb)
	ZEND_ARG_INFO(0, eventcb)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_connect, 0, 0, 1)
	ZEND_ARG_INFO(0, addr)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_connecthost, 0, 0, 3)
	ZEND_ARG_INFO(0, dns_base)
	ZEND_ARG_INFO(0, hostname)
	ZEND_ARG_INFO(0, port)
	ZEND_ARG_INFO(0, family)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_set_callbacks, 0, 0, 3)
	ZEND_ARG_INFO(0, readcb)
	ZEND_ARG_INFO(0, writecb)
	ZEND_ARG_INFO(0, eventcb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_setwatermark, 0, 0, 3)
	ZEND_ARG_INFO(0, events)
	ZEND_ARG_INFO(0, lowmark)
	ZEND_ARG_INFO(0, highmark)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_write, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_write_buffer, 0, 0, 1)
	ZEND_ARG_INFO(0, buf)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_read, 0, 0, 1)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_pair_new, 0, 0, 1)
	PHP_EVENT_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_priority_set, 0, 0, 1)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_set_timeouts, 0, 0, 2)
	ZEND_ARG_INFO(0, timeout_read)
	ZEND_ARG_INFO(0, timeout_write)
ZEND_END_ARG_INFO();

#ifdef HAVE_EVENT_OPENSSL_LIB
ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_ssl_filter, 0, 0, 4)
	ZEND_ARG_INFO(0, unused)
	PHP_EVENT_ARG_OBJ_INFO(0, underlying, EventBufferEvent, 0)
	PHP_EVENT_ARG_OBJ_INFO(0, ctx, EventSslContext, 0)
	ZEND_ARG_INFO(0, state)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_create_ssl_filter, 0, 0, 3)
	PHP_EVENT_ARG_OBJ_INFO(0, underlying, EventBufferEvent, 0)
	PHP_EVENT_ARG_OBJ_INFO(0, ctx, EventSslContext, 0)
	ZEND_ARG_INFO(0, state)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_ssl_socket, 0, 0, 4)
	PHP_EVENT_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_INFO(0, socket)
	PHP_EVENT_ARG_OBJ_INFO(0, ctx, EventSslContext, 0)
	ZEND_ARG_INFO(0, state)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO();
#endif


ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_freeze, 0, 0, 1)
	ZEND_ARG_INFO(0, at_front)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_add, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_add_buffer, 0, 0, 1)
	ZEND_ARG_INFO(0, buf)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_remove_buffer, 0, 0, 2)
	ZEND_ARG_INFO(0, buf)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_copyout, 0, 0, 2)
	ZEND_ARG_INFO(1, data)
	ZEND_ARG_INFO(0, max_bytes)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_len, 0, 0, 1)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_remove, 0, 0, 1)
	ZEND_ARG_INFO(0, max_bytes)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_read_line, 0, 0, 1)
	ZEND_ARG_INFO(0, eol_style)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_search, 0, 0, 1)
	ZEND_ARG_INFO(0, what)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, end)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_search_eol, 0, 0, 0)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, eol_style)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_pullup, 0, 0, 1)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_write, 0, 0, 1)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, howmuch)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_substr, 0, 0, 1)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO();


ZEND_BEGIN_ARG_INFO_EX(arginfo_event_socket_1, 0, 0, 0)
	ZEND_ARG_INFO(0, socket)
ZEND_END_ARG_INFO();


ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection__construct, 0, 0, 5)
	PHP_EVENT_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_INFO(0, dns_base)
	ZEND_ARG_INFO(0, address)
	ZEND_ARG_INFO(0, port)
	PHP_EVENT_ARG_OBJ_INFO(0, ctx, EventSslContext, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection_get_peer, 0, 0, 2)
	ZEND_ARG_INFO(1, address)
	ZEND_ARG_INFO(1, port)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection_set_local_address, 0, 0, 1)
	ZEND_ARG_INFO(0, address)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection_set_local_port, 0, 0, 1)
	ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection_set_timeout, 0, 0, 1)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection_set_max_size, 0, 0, 1)
	ZEND_ARG_INFO(0, max_size)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection_set_retries, 0, 0, 1)
	ZEND_ARG_INFO(0, retries)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http__construct, 0, 0, 1)
	PHP_EVENT_ARG_OBJ_INFO(0, base, EventBase, 0)
	PHP_EVENT_ARG_OBJ_INFO(0, ctx, EventSslContext, 0)
ZEND_END_ARG_INFO();


ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_accept, 0, 0, 1)
	ZEND_ARG_INFO(0, socket)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_bind, 0, 0, 2)
	ZEND_ARG_INFO(0, address)
	ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_set_callback, 0, 0, 2)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_set_gen_callback, 0, 0, 1)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_set_allowed_methods, 0, 0, 1)
	ZEND_ARG_INFO(0, methods)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_set_value, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_add_alias, 0, 0, 1)
	ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_req__construct, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_req_send_error, 0, 0, 1)
	ZEND_ARG_INFO(0, error)
	ZEND_ARG_INFO(0, reason)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_req_send_reply, 0, 0, 2)
	ZEND_ARG_INFO(0, code)
	ZEND_ARG_INFO(0, reason)
	ZEND_ARG_INFO(0, buf)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_req_send_reply_chunk, 0, 0, 1)
	ZEND_ARG_INFO(0, buf)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_req_send_reply_start, 0, 0, 2)
	ZEND_ARG_INFO(0, code)
	ZEND_ARG_INFO(0, reason)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_con_make_request, 0, 0, 3)
	ZEND_ARG_INFO(0, req)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_con_set_closecb, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_req_add_header, 0, 0, 3)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_req_remove_header, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO();


ZEND_BEGIN_ARG_INFO_EX(arginfo_event_ssl_context__construct, 0, 0, 2)
	ZEND_ARG_INFO(0, method)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO();

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
ZEND_BEGIN_ARG_INFO_EX(arginfo_event_ssl_context_set_min_proto_version, 0, 0, 1)
	ZEND_ARG_INFO(0, proto)
ZEND_END_ARG_INFO();
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_util_get_socket_name, 0, 0, 2)
	ZEND_ARG_INFO(0, socket)
	ZEND_ARG_INFO(1, address)
	ZEND_ARG_INFO(1, port)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_util_get_socket_fd, 0, 0, 1)
	ZEND_ARG_INFO(0, socket)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_util_set_socket_option, 0, 0, 4)
	ZEND_ARG_INFO(0, socket)
	ZEND_ARG_INFO(0, level)
	ZEND_ARG_INFO(0, optname)
	ZEND_ARG_INFO(0, optval)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_util_create_socket, 0, 0, 1)
	ZEND_ARG_INFO(0, fd)
ZEND_END_ARG_INFO();


/* ARGINFO END }}} */


#if HAVE_EVENT_EXTRA_LIB
/* {{{ ARGINFO for extra API */

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns__construct, 0, 0, 2)
	PHP_EVENT_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_INFO(0, initialize)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns_resolv_conf_parse, 0, 0, 2)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns_base_nameserver_ip_add, 0, 0, 1)
	ZEND_ARG_INFO(0, ip)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns_base_load_hosts, 0, 0, 1)
	ZEND_ARG_INFO(0, hosts)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns_base_search_add, 0, 0, 1)
	ZEND_ARG_INFO(0, domain)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns_base_search_ndots_set, 0, 0, 1)
	ZEND_ARG_INFO(0, ndots)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns_base_set_option, 0, 0, 2)
	ZEND_ARG_INFO(0, option)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO();


ZEND_BEGIN_ARG_INFO_EX(arginfo_evconnlistener__construct, 0, 0, 6)
	PHP_EVENT_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, backlog)
	ZEND_ARG_INFO(0, target)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evconnlistener_set_cb, 0, 0, 1)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evconnlistener_set_error_cb, 0, 0, 1)
	ZEND_ARG_INFO(0, cb)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evconnlistener_get_fd, 0, 0, 1)
	ZEND_ARG_INFO(1, address)
	ZEND_ARG_INFO(1, port)
ZEND_END_ARG_INFO();


/* ARGINFO for extra API END }}} */
#endif


const zend_function_entry php_event_ce_functions[] = {/* {{{ */
	PHP_ME(Event, __construct,         arginfo_event__construct,   ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(Event, free,                arginfo_event__void,        ZEND_ACC_PUBLIC)
	PHP_ME(Event, set,                 arginfo_event_set,          ZEND_ACC_PUBLIC)
	PHP_ME(Event, getSupportedMethods, arginfo_event__void,        ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)
	PHP_ME(Event, add,                 arginfo_event_add,          ZEND_ACC_PUBLIC)
	PHP_ME(Event, del,                 arginfo_event__void,        ZEND_ACC_PUBLIC)
	PHP_ME(Event, setPriority,         arginfo_event_priority_set, ZEND_ACC_PUBLIC)
	PHP_ME(Event, pending,             arginfo_event_pending,      ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02010200
	PHP_ME(Event, removeTimer, arginfo_event__void, ZEND_ACC_PUBLIC)
#endif

	PHP_ME(Event, timer,        arginfo_evtimer_new,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Event, setTimer,     arginfo_evtimer_set,  ZEND_ACC_PUBLIC)
	PHP_ME(Event, signal,       arginfo_evsignal_new, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

	PHP_MALIAS(Event, addTimer,  add, arginfo_event_add,   ZEND_ACC_PUBLIC)
	PHP_MALIAS(Event, delTimer,  del, arginfo_event__void, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Event, addSignal, add, arginfo_event_add,   ZEND_ACC_PUBLIC)
	PHP_MALIAS(Event, delSignal, del, arginfo_event__void, ZEND_ACC_PUBLIC)

	PHP_FE_END
};
/* }}} */

const zend_function_entry php_event_base_ce_functions[] = {/* {{{ */
	PHP_ME(EventBase, __construct,        arginfo_event_base__construct,    ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EventBase, __sleep,            arginfo_event__void,              ZEND_ACC_PUBLIC  | ZEND_ACC_FINAL)
	PHP_ME(EventBase, __wakeup,           arginfo_event__void,              ZEND_ACC_PUBLIC  | ZEND_ACC_FINAL)
	PHP_ME(EventBase, getMethod,          arginfo_event__void,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, getFeatures,        arginfo_event__void,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, priorityInit,       arginfo_event_base_priority_init, ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, loop,               arginfo_event_base_loop,          ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, dispatch,           arginfo_event__void,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, exit,               arginfo_event_base_loopexit,      ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, stop,               arginfo_event__void,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, gotStop,            arginfo_event__void,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, gotExit,            arginfo_event__void,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, getTimeOfDayCached, arginfo_event__void,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, reInit,             arginfo_event__void,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, free,               arginfo_event__void,              ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02010100
	PHP_ME(EventBase, updateCacheTime, arginfo_event__void, ZEND_ACC_PUBLIC)
#endif
#if LIBEVENT_VERSION_NUMBER >= 0x02010200
	PHP_ME(EventBase, resume,             arginfo_event__void,              ZEND_ACC_PUBLIC)
#endif

	PHP_FE_END
};
/* }}} */

const zend_function_entry php_event_config_ce_functions[] = {/* {{{ */
	PHP_ME(EventConfig, __construct,     arginfo_event__void,                   ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(EventConfig, __sleep,         arginfo_event__void,                   ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(EventConfig, __wakeup,        arginfo_event__void,                   ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(EventConfig, avoidMethod,     arginfo_event_config_avoid_method,     ZEND_ACC_PUBLIC)
	PHP_ME(EventConfig, requireFeatures, arginfo_event_config_require_features, ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02010000
	PHP_ME(EventConfig, setMaxDispatchInterval, arginfo_event_config_set_max_dispatch_interval, ZEND_ACC_PUBLIC)
#endif
#if LIBEVENT_VERSION_NUMBER >= 0x02000201
	PHP_ME(EventConfig, setFlags, arginfo_event_config_set_flags, ZEND_ACC_PUBLIC)
#endif

	PHP_FE_END
};
/* }}} */

const zend_function_entry php_event_bevent_ce_functions[] = {/* {{{ */
	PHP_ME(EventBufferEvent, __construct,       arginfo_bufferevent__construct,    ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EventBufferEvent, free,              arginfo_event__void,               ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, close,             arginfo_event__void,               ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, connect,           arginfo_bufferevent_connect,       ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, connectHost,       arginfo_bufferevent_connecthost,   ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, getDnsErrorString, arginfo_event__void,               ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, setCallbacks,      arginfo_bufferevent_set_callbacks, ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, enable,            arginfo_bufferevent__events,       ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, disable,           arginfo_bufferevent__events,       ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, getEnabled,        arginfo_event__void,               ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, getInput,          arginfo_event__void,               ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, getOutput,         arginfo_event__void,               ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, setWatermark,      arginfo_bufferevent_setwatermark,  ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, write,             arginfo_bufferevent_write,         ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, writeBuffer,       arginfo_bufferevent_write_buffer,  ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, read,              arginfo_bufferevent_read,          ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, readBuffer,        arginfo_bufferevent_write_buffer,  ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, createPair,        arginfo_bufferevent_pair_new,      ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(EventBufferEvent, setPriority,       arginfo_bufferevent_priority_set,  ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, setTimeouts,       arginfo_bufferevent_set_timeouts,  ZEND_ACC_PUBLIC)
#ifdef HAVE_EVENT_OPENSSL_LIB
	PHP_ME(EventBufferEvent, sslFilter,           arginfo_bufferevent_ssl_filter,        ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC  | ZEND_ACC_DEPRECATED)
	PHP_ME(EventBufferEvent, createSslFilter,     arginfo_bufferevent_create_ssl_filter, ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)
	PHP_ME(EventBufferEvent, sslSocket,           arginfo_bufferevent_ssl_socket,        ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)
	PHP_ME(EventBufferEvent, sslError,            arginfo_event__void,                   ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, sslRenegotiate,      arginfo_event__void,                   ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, sslGetCipherInfo,    arginfo_event__void,                   ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, sslGetCipherName,    arginfo_event__void,                   ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, sslGetCipherVersion, arginfo_event__void,                   ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, sslGetProtocol,      arginfo_event__void,                   ZEND_ACC_PUBLIC)
#endif

	PHP_FE_END
};
/* }}} */

const zend_function_entry php_event_buffer_ce_functions[] = {/* {{{ */
	PHP_ME(EventBuffer, __construct,   arginfo_event__void,            ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EventBuffer, freeze,        arginfo_evbuffer_freeze,        ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, unfreeze,      arginfo_evbuffer_freeze,        ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, lock,          arginfo_event__void,            ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, unlock,        arginfo_event__void,            ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, enableLocking, arginfo_event__void,            ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, add,           arginfo_evbuffer_add,           ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, addBuffer,     arginfo_evbuffer_add_buffer,    ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, appendFrom,    arginfo_evbuffer_remove_buffer, ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, read,          arginfo_evbuffer_remove,        ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, expand,        arginfo_evbuffer_len,           ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, prepend,       arginfo_evbuffer_add,           ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, prependBuffer, arginfo_evbuffer_add_buffer,    ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, drain,         arginfo_evbuffer_len,           ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, copyout,       arginfo_evbuffer_copyout,       ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, readLine,      arginfo_evbuffer_read_line,     ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, search,        arginfo_evbuffer_search,        ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, searchEol,     arginfo_evbuffer_search_eol,    ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, pullup,        arginfo_evbuffer_pullup,        ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, write,         arginfo_evbuffer_write,         ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, readFrom,      arginfo_evbuffer_write,         ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, substr,        arginfo_evbuffer_substr,        ZEND_ACC_PUBLIC)

	PHP_FE_END
};
/* }}} */

const zend_function_entry php_event_util_ce_functions[] = {/* {{{ */
	PHP_ME(EventUtil, __construct, arginfo_event__void, ZEND_ACC_PRIVATE)

	PHP_ME(EventUtil, getLastSocketErrno, arginfo_event_socket_1, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(EventUtil, getLastSocketError, arginfo_event_socket_1, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

#ifdef            HAVE_EVENT_OPENSSL_LIB
	PHP_ME(EventUtil, sslRandPoll,           arginfo_event__void,                ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
#endif
	PHP_ME(EventUtil, getSocketName,   arginfo_event_util_get_socket_name,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(EventUtil, getSocketFd,     arginfo_event_util_get_socket_fd,     ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(EventUtil, setSocketOption, arginfo_event_util_set_socket_option, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
#ifdef PHP_EVENT_SOCKETS_SUPPORT
	PHP_ME(EventUtil, createSocket,    arginfo_event_util_create_socket,     ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
#endif

	PHP_FE_END
};
/* }}} */

/* }}} */

#if HAVE_EVENT_EXTRA_LIB
/* {{{ Extra API */

const zend_function_entry php_event_dns_base_ce_functions[] = {
	PHP_ME(EventDnsBase, __construct,      arginfo_evdns__construct,             ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EventDnsBase, parseResolvConf,  arginfo_evdns_resolv_conf_parse,      ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, addNameserverIp,  arginfo_evdns_base_nameserver_ip_add, ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, loadHosts,        arginfo_evdns_base_load_hosts,        ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, clearSearch,      arginfo_event__void,                  ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, addSearch,        arginfo_evdns_base_search_add,        ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, setSearchNdots,   arginfo_evdns_base_search_ndots_set,  ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, setOption,        arginfo_evdns_base_set_option,        ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, countNameservers, arginfo_event__void,                  ZEND_ACC_PUBLIC)

	PHP_FE_END
};

const zend_function_entry php_event_http_conn_ce_functions[] = {
	PHP_ME(EventHttpConnection, __construct,       arginfo_event_evhttp_connection__construct,        ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(EventHttpConnection, __sleep,           arginfo_event__void,                               ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(EventHttpConnection, __wakeup,          arginfo_event__void,                               ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(EventHttpConnection, getBase,           arginfo_event__void,                               ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, getPeer,           arginfo_event_evhttp_connection_get_peer,          ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, setLocalAddress,   arginfo_event_evhttp_connection_set_local_address, ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, setLocalPort,      arginfo_event_evhttp_connection_set_local_port,    ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, setTimeout,        arginfo_event_evhttp_connection_set_timeout,       ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, setMaxHeadersSize, arginfo_event_evhttp_connection_set_max_size,      ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, setMaxBodySize,    arginfo_event_evhttp_connection_set_max_size,      ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, setRetries,        arginfo_event_evhttp_connection_set_retries,       ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, makeRequest,       arginfo_event_http_con_make_request,               ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, setCloseCallback,  arginfo_event_http_con_set_closecb,                ZEND_ACC_PUBLIC)

	PHP_FE_END
};

const zend_function_entry php_event_http_ce_functions[] = {
	PHP_ME(EventHttp, __construct,        arginfo_event_http__construct,          ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(EventHttp, __sleep,            arginfo_event__void,                    ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(EventHttp, __wakeup,           arginfo_event__void,                    ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(EventHttp, accept,             arginfo_event_http_accept,              ZEND_ACC_PUBLIC)
	PHP_ME(EventHttp, bind,               arginfo_event_http_bind,                ZEND_ACC_PUBLIC)
	PHP_ME(EventHttp, setCallback,        arginfo_event_http_set_callback,        ZEND_ACC_PUBLIC)
	PHP_ME(EventHttp, setDefaultCallback, arginfo_event_http_set_gen_callback,    ZEND_ACC_PUBLIC)
	PHP_ME(EventHttp, setAllowedMethods,  arginfo_event_http_set_allowed_methods, ZEND_ACC_PUBLIC)
	PHP_ME(EventHttp, setMaxBodySize,     arginfo_event_http_set_value,           ZEND_ACC_PUBLIC)
	PHP_ME(EventHttp, setMaxHeadersSize,  arginfo_event_http_set_value,           ZEND_ACC_PUBLIC)
	PHP_ME(EventHttp, setTimeout,         arginfo_event_http_set_value,           ZEND_ACC_PUBLIC)
	PHP_ME(EventHttp, addServerAlias,     arginfo_event_http_add_alias,           ZEND_ACC_PUBLIC)
	PHP_ME(EventHttp, removeServerAlias,  arginfo_event_http_add_alias,           ZEND_ACC_PUBLIC)

	PHP_FE_END
};

const zend_function_entry php_event_http_req_ce_functions[] = {
	PHP_ME(EventHttpRequest, __construct, arginfo_event_http_req__construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)

	PHP_ME(EventHttpRequest, __sleep,          arginfo_event__void,                     ZEND_ACC_PUBLIC  | ZEND_ACC_FINAL)
	PHP_ME(EventHttpRequest, __wakeup,         arginfo_event__void,                     ZEND_ACC_PUBLIC  | ZEND_ACC_FINAL)
	PHP_ME(EventHttpRequest, free,             arginfo_event__void,                     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, getCommand,       arginfo_event__void,                     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, getHost,          arginfo_event__void,                     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, getUri,           arginfo_event__void,                     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, getResponseCode,  arginfo_event__void,                     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, getInputHeaders,  arginfo_event__void,                     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, getOutputHeaders, arginfo_event__void,                     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, getInputBuffer,   arginfo_event__void,                     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, getOutputBuffer,  arginfo_event__void,                     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, sendError,        arginfo_event_http_req_send_error,       ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, sendReply,        arginfo_event_http_req_send_reply,       ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, sendReplyChunk,   arginfo_event_http_req_send_reply_chunk, ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, sendReplyEnd,     arginfo_event__void,                     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, sendReplyStart,   arginfo_event_http_req_send_reply_start, ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, cancel,           arginfo_event__void,                     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, addHeader,        arginfo_event_http_req_add_header,       ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, clearHeaders,     arginfo_event__void,                     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, removeHeader,     arginfo_event_http_req_remove_header,    ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, findHeader,       arginfo_event_http_req_remove_header,    ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02001100
	PHP_ME(EventHttpRequest, getBufferEvent,   arginfo_event__void,                     ZEND_ACC_PUBLIC)
#endif
	PHP_ME(EventHttpRequest, getConnection,    arginfo_event__void,                     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpRequest, closeConnection,  arginfo_event__void,                     ZEND_ACC_PUBLIC)

	PHP_FE_END
};

const zend_function_entry php_event_listener_ce_functions[] = {
	PHP_ME(EventListener, __construct,      arginfo_evconnlistener__construct,   ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(EventListener, __sleep,          arginfo_event__void,                 ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(EventListener, __wakeup,         arginfo_event__void,                 ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(EventListener, free,             arginfo_event__void,                 ZEND_ACC_PUBLIC)
	PHP_ME(EventListener, enable,           arginfo_event__void,                 ZEND_ACC_PUBLIC)
	PHP_ME(EventListener, disable,          arginfo_event__void,                 ZEND_ACC_PUBLIC)
	PHP_ME(EventListener, setCallback,      arginfo_evconnlistener_set_cb,       ZEND_ACC_PUBLIC)
	PHP_ME(EventListener, setErrorCallback, arginfo_evconnlistener_set_error_cb, ZEND_ACC_PUBLIC)
	PHP_ME(EventListener, getSocketName,    arginfo_evconnlistener_get_fd,       ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02000300
	PHP_ME(EventListener, getBase, arginfo_event__void, ZEND_ACC_PUBLIC)
#endif

	PHP_FE_END
};

/* Extra API END}}} */
#endif

#ifdef HAVE_EVENT_OPENSSL_LIB
const zend_function_entry php_event_ssl_context_ce_functions[] = {/* {{{ */
	PHP_ME(EventSslContext, __construct, arginfo_event_ssl_context__construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
	PHP_ME(EventSslContext, setMinProtoVersion, arginfo_event_ssl_context_set_min_proto_version, ZEND_ACC_PUBLIC)
	PHP_ME(EventSslContext, setMaxProtoVersion, arginfo_event_ssl_context_set_min_proto_version, ZEND_ACC_PUBLIC)
#endif

	PHP_FE_END
};
/* }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
