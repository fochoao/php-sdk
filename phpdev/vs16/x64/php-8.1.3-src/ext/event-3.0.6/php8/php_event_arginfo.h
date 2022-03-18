/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: a6b73da1aafb89d8e74bbc59e9a57eca55a67645 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_EventConfig___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventConfig___sleep, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventConfig___wakeup, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventConfig_avoidMethod, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, method, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventConfig_requireFeatures, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, feature, IS_LONG, 0)
ZEND_END_ARG_INFO()

#if LIBEVENT_VERSION_NUMBER >= 0x02010000
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventConfig_setMaxDispatchInterval, 0, 3, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, max_interval, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, max_callbacks, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, min_priority, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if LIBEVENT_VERSION_NUMBER >= 0x02000201 /* 2.0.2-alpha */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventConfig_setFlags, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_EventBase___construct, 0, 0, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, cfg, EventConfig, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_EventBase___sleep arginfo_class_EventConfig___sleep

#define arginfo_class_EventBase___wakeup arginfo_class_EventConfig___wakeup

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBase_getMethod, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBase_getFeatures, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBase_priorityInit, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, n_priorities, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBase_loop, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBase_dispatch, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBase_exit, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_DOUBLE, 0, "0.0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBase_set, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, event, Event, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_EventBase_stop arginfo_class_EventBase_dispatch

#define arginfo_class_EventBase_gotStop arginfo_class_EventBase_dispatch

#define arginfo_class_EventBase_gotExit arginfo_class_EventBase_dispatch

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBase_getTimeOfDayCached, 0, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_EventBase_reInit arginfo_class_EventBase_dispatch

#define arginfo_class_EventBase_free arginfo_class_EventConfig___wakeup

#if LIBEVENT_VERSION_NUMBER >= 0x02010100
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBase_updateCacheTime, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#endif

#if LIBEVENT_VERSION_NUMBER >= 0x02010200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBase_resume, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Event___construct, 0, 0, 4)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, what, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_Event_free arginfo_class_EventConfig___wakeup

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Event_set, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, what, IS_LONG, 0, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cb, IS_CALLABLE, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_Event_getSupportedMethods arginfo_class_EventConfig___sleep

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Event_add, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_DOUBLE, 0, "-1")
ZEND_END_ARG_INFO()

#define arginfo_class_Event_del arginfo_class_EventBase_dispatch

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Event_setPriority, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, priority, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Event_pending, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

#if LIBEVENT_VERSION_NUMBER >= 0x02010200
#define arginfo_class_Event_removeTimer arginfo_class_EventBase_resume
#endif

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Event_timer, 0, 2, Event, 0)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Event_setTimer, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Event_signal, 0, 3, Event, 0)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, signum, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_Event_addTimer arginfo_class_Event_add

#define arginfo_class_Event_delTimer arginfo_class_EventBase_dispatch

#define arginfo_class_Event_addSignal arginfo_class_Event_add

#define arginfo_class_Event_delSignal arginfo_class_EventBase_dispatch

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_EventDnsBase___construct, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, initialize, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventDnsBase_parseResolvConf, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventDnsBase_addNameserverIp, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, ip, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventDnsBase_loadHosts, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, hosts, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventDnsBase_clearSearch, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventDnsBase_addSearch, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, domain, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventDnsBase_setSearchNdots, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, ndots, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventDnsBase_setOption, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventDnsBase_countNameservers, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_EventHttpConnection___construct, 0, 0, 4)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_OBJ_INFO(0, dns_base, EventDnsBase, 1)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, ctx, EventSslContext, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB && !(LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB))
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_EventHttpConnection___construct, 0, 0, 4)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_OBJ_INFO(0, dns_base, EventDnsBase, 1)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpConnection___sleep, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpConnection___wakeup arginfo_class_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_EventHttpConnection_getBase, 0, 0, EventBase, MAY_BE_FALSE)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpConnection_getPeer, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(1, address, IS_MIXED, 1)
	ZEND_ARG_TYPE_INFO(1, port, IS_MIXED, 1)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpConnection_setLocalAddress, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpConnection_setLocalPort, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpConnection_setTimeout, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpConnection_setMaxHeadersSize, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, max_size, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpConnection_setMaxBodySize arginfo_class_EventHttpConnection_setMaxHeadersSize
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpConnection_setRetries, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, retries, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpConnection_makeRequest, 0, 3, _IS_BOOL, 1)
	ZEND_ARG_OBJ_INFO(0, req, EventHttpRequest, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, uri, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpConnection_setCloseCallback, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_EventHttp___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, ctx, EventSslContext, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB && !(LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB))
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_EventHttp___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttp___sleep arginfo_class_EventHttpConnection___sleep
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttp___wakeup arginfo_class_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttp_accept, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, socket, IS_MIXED, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttp_bind, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttp_setCallback, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttp_setDefaultCallback, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttp_setAllowedMethods, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, methods, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttp_setMaxBodySize, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttp_setMaxHeadersSize arginfo_class_EventHttp_setMaxBodySize
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttp_setTimeout arginfo_class_EventHttp_setMaxBodySize
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttp_addServerAlias, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, alias, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttp_removeServerAlias arginfo_class_EventHttp_addServerAlias
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_EventHttpRequest___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpRequest___sleep arginfo_class_EventHttpConnection___sleep
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpRequest___wakeup arginfo_class_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpRequest_free arginfo_class_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpRequest_getCommand arginfo_class_EventDnsBase_countNameservers
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpRequest_getHost, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpRequest_getUri arginfo_class_EventHttpRequest_getHost
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpRequest_getResponseCode arginfo_class_EventDnsBase_countNameservers
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpRequest_getInputHeaders arginfo_class_EventHttpConnection___sleep
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpRequest_getOutputHeaders arginfo_class_EventHttpConnection___sleep
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_EventHttpRequest_getInputBuffer, 0, 0, EventBuffer, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpRequest_getOutputBuffer arginfo_class_EventHttpRequest_getInputBuffer
#endif

#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02001100
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_EventHttpRequest_getBufferEvent, 0, 0, EventBufferEvent, 1)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_EventHttpRequest_getConnection, 0, 0, EventHttpConnection, 1)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpRequest_closeConnection arginfo_class_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpRequest_sendError, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, error, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, reason, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpRequest_sendReply, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, code, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, reason, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, buf, EventBuffer, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpRequest_sendReplyChunk, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, buf, EventBuffer, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpRequest_sendReplyEnd arginfo_class_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpRequest_sendReplyStart, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, code, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, reason, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpRequest_cancel arginfo_class_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpRequest_addHeader, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventHttpRequest_clearHeaders arginfo_class_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpRequest_removeHeader, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventHttpRequest_findHeader, 0, 2, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_EventListener___construct, 0, 0, 6)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_MIXED, 1)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, backlog, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, target, IS_MIXED, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventListener___sleep arginfo_class_EventHttpConnection___sleep
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventListener___wakeup arginfo_class_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventListener_free arginfo_class_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventListener_enable, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventListener_disable arginfo_class_EventListener_enable
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_EventListener_setCallback arginfo_class_EventHttp_setDefaultCallback
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventListener_setErrorCallback, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02000300
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_EventListener_getBase, 0, 0, EventBase, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventListener_getSocketName, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(1, address, IS_MIXED, 1)
	ZEND_ARG_TYPE_INFO(1, port, IS_MIXED, 1)
ZEND_END_ARG_INFO()
#endif

#define arginfo_class_EventUtil___construct arginfo_class_EventConfig___construct

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_EventUtil_getLastSocketErrno, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, socket, Socket, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_EventUtil_getLastSocketError, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, socket, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventUtil_sslRandPoll, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventUtil_getSocketName, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, socket, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(1, address, IS_MIXED, 1)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(1, port, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventUtil_getSocketFd, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, socket, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventUtil_setSocketOption, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, socket, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, optname, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, optval, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#if defined(PHP_EVENT_SOCKETS_SUPPORT)
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_EventUtil_createSocket, 0, 1, Socket, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#define arginfo_class_EventBuffer___construct arginfo_class_EventConfig___construct

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBuffer_freeze, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, at_front, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_EventBuffer_unfreeze arginfo_class_EventBuffer_freeze

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBuffer_lock, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, at_front, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_EventBuffer_unlock arginfo_class_EventBuffer_lock

#define arginfo_class_EventBuffer_enableLocking arginfo_class_EventConfig___wakeup

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBuffer_add, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBuffer_read, 0, 1, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, max_bytes, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBuffer_addBuffer, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, buf, EventBuffer, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBuffer_appendFrom, 0, 2, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, buf, EventBuffer, 0)
	ZEND_ARG_TYPE_INFO(0, len, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBuffer_expand, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, len, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_EventBuffer_prepend arginfo_class_EventBuffer_add

#define arginfo_class_EventBuffer_prependBuffer arginfo_class_EventBuffer_addBuffer

#define arginfo_class_EventBuffer_drain arginfo_class_EventBuffer_expand

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBuffer_copyout, 0, 2, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(1, data, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, max_bytes, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBuffer_readLine, 0, 1, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, eol_style, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_EventBuffer_search, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, what, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start, IS_LONG, 0, "-1")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, end, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_EventBuffer_searchEol, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start, IS_LONG, 0, "-1")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, eol_style, IS_LONG, 0, "EventBuffer::EOL_ANY")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBuffer_pullup, 0, 1, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_EventBuffer_write, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, howmuch, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

#define arginfo_class_EventBuffer_readFrom arginfo_class_EventBuffer_write

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_EventBuffer_substr, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, start, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_EventBufferEvent___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, socket, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, readcb, IS_CALLABLE, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, writecb, IS_CALLABLE, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, eventcb, IS_CALLABLE, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_EventBufferEvent_free arginfo_class_EventConfig___wakeup

#define arginfo_class_EventBufferEvent_close arginfo_class_EventConfig___wakeup

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBufferEvent_connect, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, addr, IS_STRING, 0)
ZEND_END_ARG_INFO()

#if defined(HAVE_EVENT_EXTRA_LIB)
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBufferEvent_connectHost, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, dns_base, EventDnsBase, 1)
	ZEND_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, family, IS_LONG, 0, "EventUtil::AF_UNSPEC")
ZEND_END_ARG_INFO()
#endif

#if !(defined(HAVE_EVENT_EXTRA_LIB))
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBufferEvent_connectHost, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, unused, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, family, IS_LONG, 0, "EventUtil::AF_UNSPEC")
ZEND_END_ARG_INFO()
#endif

#define arginfo_class_EventBufferEvent_getDnsErrorString arginfo_class_EventBase_getMethod

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBufferEvent_setCallbacks, 0, 3, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, readcb, IS_CALLABLE, 1)
	ZEND_ARG_TYPE_INFO(0, writecb, IS_CALLABLE, 1)
	ZEND_ARG_TYPE_INFO(0, eventcb, IS_CALLABLE, 1)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBufferEvent_enable, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, events, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_EventBufferEvent_disable arginfo_class_EventBufferEvent_enable

#define arginfo_class_EventBufferEvent_getEnabled arginfo_class_EventBase_getFeatures

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_EventBufferEvent_getInput, 0, 0, EventBuffer, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_EventBufferEvent_getOutput arginfo_class_EventBufferEvent_getInput

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBufferEvent_setWatermark, 0, 3, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, events, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, lowmark, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, highmark, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_EventBufferEvent_write arginfo_class_EventBuffer_add

#define arginfo_class_EventBufferEvent_writeBuffer arginfo_class_EventBuffer_addBuffer

#define arginfo_class_EventBufferEvent_read arginfo_class_EventBuffer_pullup

#define arginfo_class_EventBufferEvent_readBuffer arginfo_class_EventBuffer_addBuffer

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_EventBufferEvent_createPair, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

#define arginfo_class_EventBufferEvent_setPriority arginfo_class_Event_setPriority

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBufferEvent_setTimeouts, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, timeout_read, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, timeout_write, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_EventBufferEvent_createSslFilter, 0, 3, EventBufferEvent, 0)
	ZEND_ARG_OBJ_INFO(0, unnderlying, EventBufferEvent, 0)
	ZEND_ARG_OBJ_INFO(0, ctx, EventSslContext, 0)
	ZEND_ARG_TYPE_INFO(0, state, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_EventBufferEvent_sslSocket, 0, 4, EventBufferEvent, 0)
	ZEND_ARG_OBJ_INFO(0, base, EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, socket, IS_MIXED, 0)
	ZEND_ARG_OBJ_INFO(0, ctx, EventSslContext, 0)
	ZEND_ARG_TYPE_INFO(0, state, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBufferEvent_sslError, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventBufferEvent_sslRenegotiate, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
#define arginfo_class_EventBufferEvent_sslGetCipherInfo arginfo_class_EventBufferEvent_sslError
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
#define arginfo_class_EventBufferEvent_sslGetCipherName arginfo_class_EventBufferEvent_sslError
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
#define arginfo_class_EventBufferEvent_sslGetCipherVersion arginfo_class_EventBufferEvent_sslError
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
#define arginfo_class_EventBufferEvent_sslGetProtocol arginfo_class_EventBufferEvent_sslError
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_EventSslContext___construct, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, method, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
ZEND_END_ARG_INFO()
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB) && OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_EventSslContext_setMinProtoVersion, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, proto, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB) && OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
#define arginfo_class_EventSslContext_setMaxProtoVersion arginfo_class_EventSslContext_setMinProtoVersion
#endif


ZEND_METHOD(EventConfig, __construct);
ZEND_METHOD(EventConfig, __sleep);
ZEND_METHOD(EventConfig, __wakeup);
ZEND_METHOD(EventConfig, avoidMethod);
ZEND_METHOD(EventConfig, requireFeatures);
#if LIBEVENT_VERSION_NUMBER >= 0x02010000
ZEND_METHOD(EventConfig, setMaxDispatchInterval);
#endif
#if LIBEVENT_VERSION_NUMBER >= 0x02000201 /* 2.0.2-alpha */
ZEND_METHOD(EventConfig, setFlags);
#endif
ZEND_METHOD(EventBase, __construct);
ZEND_METHOD(EventBase, __sleep);
ZEND_METHOD(EventBase, __wakeup);
ZEND_METHOD(EventBase, getMethod);
ZEND_METHOD(EventBase, getFeatures);
ZEND_METHOD(EventBase, priorityInit);
ZEND_METHOD(EventBase, loop);
ZEND_METHOD(EventBase, dispatch);
ZEND_METHOD(EventBase, exit);
ZEND_METHOD(EventBase, set);
ZEND_METHOD(EventBase, stop);
ZEND_METHOD(EventBase, gotStop);
ZEND_METHOD(EventBase, gotExit);
ZEND_METHOD(EventBase, getTimeOfDayCached);
ZEND_METHOD(EventBase, reInit);
ZEND_METHOD(EventBase, free);
#if LIBEVENT_VERSION_NUMBER >= 0x02010100
ZEND_METHOD(EventBase, updateCacheTime);
#endif
#if LIBEVENT_VERSION_NUMBER >= 0x02010200
ZEND_METHOD(EventBase, resume);
#endif
ZEND_METHOD(Event, __construct);
ZEND_METHOD(Event, free);
ZEND_METHOD(Event, set);
ZEND_METHOD(Event, getSupportedMethods);
ZEND_METHOD(Event, add);
ZEND_METHOD(Event, del);
ZEND_METHOD(Event, setPriority);
ZEND_METHOD(Event, pending);
#if LIBEVENT_VERSION_NUMBER >= 0x02010200
ZEND_METHOD(Event, removeTimer);
#endif
ZEND_METHOD(Event, timer);
ZEND_METHOD(Event, setTimer);
ZEND_METHOD(Event, signal);
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventDnsBase, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventDnsBase, parseResolvConf);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventDnsBase, addNameserverIp);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventDnsBase, loadHosts);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventDnsBase, clearSearch);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventDnsBase, addSearch);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventDnsBase, setSearchNdots);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventDnsBase, setOption);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventDnsBase, countNameservers);
#endif
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(EventHttpConnection, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB && !(LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB))
ZEND_METHOD(EventHttpConnection, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpConnection, __sleep);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpConnection, __wakeup);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpConnection, getBase);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpConnection, getPeer);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpConnection, setLocalAddress);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpConnection, setLocalPort);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpConnection, setTimeout);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpConnection, setMaxHeadersSize);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpConnection, setMaxBodySize);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpConnection, setRetries);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpConnection, makeRequest);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpConnection, setCloseCallback);
#endif
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(EventHttp, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB && !(LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB))
ZEND_METHOD(EventHttp, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttp, __sleep);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttp, __wakeup);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttp, accept);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttp, bind);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttp, setCallback);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttp, setDefaultCallback);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttp, setAllowedMethods);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttp, setMaxBodySize);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttp, setMaxHeadersSize);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttp, setTimeout);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttp, addServerAlias);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttp, removeServerAlias);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, __sleep);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, __wakeup);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, free);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, getCommand);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, getHost);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, getUri);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, getResponseCode);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, getInputHeaders);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, getOutputHeaders);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, getInputBuffer);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, getOutputBuffer);
#endif
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02001100
ZEND_METHOD(EventHttpRequest, getBufferEvent);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, getConnection);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, closeConnection);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, sendError);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, sendReply);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, sendReplyChunk);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, sendReplyEnd);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, sendReplyStart);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, cancel);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, addHeader);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, clearHeaders);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, removeHeader);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventHttpRequest, findHeader);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventListener, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventListener, __sleep);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventListener, __wakeup);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventListener, free);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventListener, enable);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventListener, disable);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventListener, setCallback);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventListener, setErrorCallback);
#endif
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02000300
ZEND_METHOD(EventListener, getBase);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(EventListener, getSocketName);
#endif
ZEND_METHOD(EventUtil, __construct);
ZEND_METHOD(EventUtil, getLastSocketErrno);
ZEND_METHOD(EventUtil, getLastSocketError);
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(EventUtil, sslRandPoll);
#endif
ZEND_METHOD(EventUtil, getSocketName);
ZEND_METHOD(EventUtil, getSocketFd);
ZEND_METHOD(EventUtil, setSocketOption);
#if defined(PHP_EVENT_SOCKETS_SUPPORT)
ZEND_METHOD(EventUtil, createSocket);
#endif
ZEND_METHOD(EventBuffer, __construct);
ZEND_METHOD(EventBuffer, freeze);
ZEND_METHOD(EventBuffer, unfreeze);
ZEND_METHOD(EventBuffer, lock);
ZEND_METHOD(EventBuffer, unlock);
ZEND_METHOD(EventBuffer, enableLocking);
ZEND_METHOD(EventBuffer, add);
ZEND_METHOD(EventBuffer, read);
ZEND_METHOD(EventBuffer, addBuffer);
ZEND_METHOD(EventBuffer, appendFrom);
ZEND_METHOD(EventBuffer, expand);
ZEND_METHOD(EventBuffer, prepend);
ZEND_METHOD(EventBuffer, prependBuffer);
ZEND_METHOD(EventBuffer, drain);
ZEND_METHOD(EventBuffer, copyout);
ZEND_METHOD(EventBuffer, readLine);
ZEND_METHOD(EventBuffer, search);
ZEND_METHOD(EventBuffer, searchEol);
ZEND_METHOD(EventBuffer, pullup);
ZEND_METHOD(EventBuffer, write);
ZEND_METHOD(EventBuffer, readFrom);
ZEND_METHOD(EventBuffer, substr);
ZEND_METHOD(EventBufferEvent, __construct);
ZEND_METHOD(EventBufferEvent, free);
ZEND_METHOD(EventBufferEvent, close);
ZEND_METHOD(EventBufferEvent, connect);
#if defined(HAVE_EVENT_EXTRA_LIB)
ZEND_METHOD(EventBufferEvent, connectHost);
#endif
#if !(defined(HAVE_EVENT_EXTRA_LIB))
ZEND_METHOD(EventBufferEvent, connectHost);
#endif
ZEND_METHOD(EventBufferEvent, getDnsErrorString);
ZEND_METHOD(EventBufferEvent, setCallbacks);
ZEND_METHOD(EventBufferEvent, enable);
ZEND_METHOD(EventBufferEvent, disable);
ZEND_METHOD(EventBufferEvent, getEnabled);
ZEND_METHOD(EventBufferEvent, getInput);
ZEND_METHOD(EventBufferEvent, getOutput);
ZEND_METHOD(EventBufferEvent, setWatermark);
ZEND_METHOD(EventBufferEvent, write);
ZEND_METHOD(EventBufferEvent, writeBuffer);
ZEND_METHOD(EventBufferEvent, read);
ZEND_METHOD(EventBufferEvent, readBuffer);
ZEND_METHOD(EventBufferEvent, createPair);
ZEND_METHOD(EventBufferEvent, setPriority);
ZEND_METHOD(EventBufferEvent, setTimeouts);
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(EventBufferEvent, createSslFilter);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(EventBufferEvent, sslSocket);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(EventBufferEvent, sslError);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(EventBufferEvent, sslRenegotiate);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(EventBufferEvent, sslGetCipherInfo);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(EventBufferEvent, sslGetCipherName);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(EventBufferEvent, sslGetCipherVersion);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(EventBufferEvent, sslGetProtocol);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(EventSslContext, __construct);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB) && OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
ZEND_METHOD(EventSslContext, setMinProtoVersion);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB) && OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
ZEND_METHOD(EventSslContext, setMaxProtoVersion);
#endif


static const zend_function_entry class_EventConfig_methods[] = {
	ZEND_ME(EventConfig, __construct, arginfo_class_EventConfig___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(EventConfig, __sleep, arginfo_class_EventConfig___sleep, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	ZEND_ME(EventConfig, __wakeup, arginfo_class_EventConfig___wakeup, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	ZEND_ME(EventConfig, avoidMethod, arginfo_class_EventConfig_avoidMethod, ZEND_ACC_PUBLIC)
	ZEND_ME(EventConfig, requireFeatures, arginfo_class_EventConfig_requireFeatures, ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02010000
	ZEND_ME(EventConfig, setMaxDispatchInterval, arginfo_class_EventConfig_setMaxDispatchInterval, ZEND_ACC_PUBLIC)
#endif
#if LIBEVENT_VERSION_NUMBER >= 0x02000201 /* 2.0.2-alpha */
	ZEND_ME(EventConfig, setFlags, arginfo_class_EventConfig_setFlags, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_EventBase_methods[] = {
	ZEND_ME(EventBase, __construct, arginfo_class_EventBase___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBase, __sleep, arginfo_class_EventBase___sleep, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	ZEND_ME(EventBase, __wakeup, arginfo_class_EventBase___wakeup, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	ZEND_ME(EventBase, getMethod, arginfo_class_EventBase_getMethod, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBase, getFeatures, arginfo_class_EventBase_getFeatures, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBase, priorityInit, arginfo_class_EventBase_priorityInit, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBase, loop, arginfo_class_EventBase_loop, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBase, dispatch, arginfo_class_EventBase_dispatch, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBase, exit, arginfo_class_EventBase_exit, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBase, set, arginfo_class_EventBase_set, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBase, stop, arginfo_class_EventBase_stop, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBase, gotStop, arginfo_class_EventBase_gotStop, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBase, gotExit, arginfo_class_EventBase_gotExit, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBase, getTimeOfDayCached, arginfo_class_EventBase_getTimeOfDayCached, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBase, reInit, arginfo_class_EventBase_reInit, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBase, free, arginfo_class_EventBase_free, ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02010100
	ZEND_ME(EventBase, updateCacheTime, arginfo_class_EventBase_updateCacheTime, ZEND_ACC_PUBLIC)
#endif
#if LIBEVENT_VERSION_NUMBER >= 0x02010200
	ZEND_ME(EventBase, resume, arginfo_class_EventBase_resume, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_Event_methods[] = {
	ZEND_ME(Event, __construct, arginfo_class_Event___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Event, free, arginfo_class_Event_free, ZEND_ACC_PUBLIC)
	ZEND_ME(Event, set, arginfo_class_Event_set, ZEND_ACC_PUBLIC)
	ZEND_ME(Event, getSupportedMethods, arginfo_class_Event_getSupportedMethods, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Event, add, arginfo_class_Event_add, ZEND_ACC_PUBLIC)
	ZEND_ME(Event, del, arginfo_class_Event_del, ZEND_ACC_PUBLIC)
	ZEND_ME(Event, setPriority, arginfo_class_Event_setPriority, ZEND_ACC_PUBLIC)
	ZEND_ME(Event, pending, arginfo_class_Event_pending, ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02010200
	ZEND_ME(Event, removeTimer, arginfo_class_Event_removeTimer, ZEND_ACC_PUBLIC)
#endif
	ZEND_ME(Event, timer, arginfo_class_Event_timer, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Event, setTimer, arginfo_class_Event_setTimer, ZEND_ACC_PUBLIC)
	ZEND_ME(Event, signal, arginfo_class_Event_signal, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_MALIAS(Event, addTimer, add, arginfo_class_Event_addTimer, ZEND_ACC_PUBLIC)
	ZEND_MALIAS(Event, delTimer, del, arginfo_class_Event_delTimer, ZEND_ACC_PUBLIC)
	ZEND_MALIAS(Event, addSignal, add, arginfo_class_Event_addSignal, ZEND_ACC_PUBLIC)
	ZEND_MALIAS(Event, delSignal, del, arginfo_class_Event_delSignal, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_EventDnsBase_methods[] = {
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventDnsBase, __construct, arginfo_class_EventDnsBase___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventDnsBase, parseResolvConf, arginfo_class_EventDnsBase_parseResolvConf, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventDnsBase, addNameserverIp, arginfo_class_EventDnsBase_addNameserverIp, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventDnsBase, loadHosts, arginfo_class_EventDnsBase_loadHosts, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventDnsBase, clearSearch, arginfo_class_EventDnsBase_clearSearch, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventDnsBase, addSearch, arginfo_class_EventDnsBase_addSearch, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventDnsBase, setSearchNdots, arginfo_class_EventDnsBase_setSearchNdots, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventDnsBase, setOption, arginfo_class_EventDnsBase_setOption, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventDnsBase, countNameservers, arginfo_class_EventDnsBase_countNameservers, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_EventHttpConnection_methods[] = {
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(EventHttpConnection, __construct, arginfo_class_EventHttpConnection___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB && !(LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB))
	ZEND_ME(EventHttpConnection, __construct, arginfo_class_EventHttpConnection___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpConnection, __sleep, arginfo_class_EventHttpConnection___sleep, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpConnection, __wakeup, arginfo_class_EventHttpConnection___wakeup, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpConnection, getBase, arginfo_class_EventHttpConnection_getBase, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpConnection, getPeer, arginfo_class_EventHttpConnection_getPeer, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpConnection, setLocalAddress, arginfo_class_EventHttpConnection_setLocalAddress, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpConnection, setLocalPort, arginfo_class_EventHttpConnection_setLocalPort, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpConnection, setTimeout, arginfo_class_EventHttpConnection_setTimeout, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpConnection, setMaxHeadersSize, arginfo_class_EventHttpConnection_setMaxHeadersSize, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpConnection, setMaxBodySize, arginfo_class_EventHttpConnection_setMaxBodySize, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpConnection, setRetries, arginfo_class_EventHttpConnection_setRetries, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpConnection, makeRequest, arginfo_class_EventHttpConnection_makeRequest, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpConnection, setCloseCallback, arginfo_class_EventHttpConnection_setCloseCallback, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_EventHttp_methods[] = {
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(EventHttp, __construct, arginfo_class_EventHttp___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB && !(LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB))
	ZEND_ME(EventHttp, __construct, arginfo_class_EventHttp___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttp, __sleep, arginfo_class_EventHttp___sleep, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttp, __wakeup, arginfo_class_EventHttp___wakeup, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttp, accept, arginfo_class_EventHttp_accept, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttp, bind, arginfo_class_EventHttp_bind, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttp, setCallback, arginfo_class_EventHttp_setCallback, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttp, setDefaultCallback, arginfo_class_EventHttp_setDefaultCallback, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttp, setAllowedMethods, arginfo_class_EventHttp_setAllowedMethods, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttp, setMaxBodySize, arginfo_class_EventHttp_setMaxBodySize, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttp, setMaxHeadersSize, arginfo_class_EventHttp_setMaxHeadersSize, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttp, setTimeout, arginfo_class_EventHttp_setTimeout, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttp, addServerAlias, arginfo_class_EventHttp_addServerAlias, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttp, removeServerAlias, arginfo_class_EventHttp_removeServerAlias, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_EventHttpRequest_methods[] = {
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, __construct, arginfo_class_EventHttpRequest___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, __sleep, arginfo_class_EventHttpRequest___sleep, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, __wakeup, arginfo_class_EventHttpRequest___wakeup, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, free, arginfo_class_EventHttpRequest_free, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, getCommand, arginfo_class_EventHttpRequest_getCommand, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, getHost, arginfo_class_EventHttpRequest_getHost, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, getUri, arginfo_class_EventHttpRequest_getUri, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, getResponseCode, arginfo_class_EventHttpRequest_getResponseCode, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, getInputHeaders, arginfo_class_EventHttpRequest_getInputHeaders, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, getOutputHeaders, arginfo_class_EventHttpRequest_getOutputHeaders, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, getInputBuffer, arginfo_class_EventHttpRequest_getInputBuffer, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, getOutputBuffer, arginfo_class_EventHttpRequest_getOutputBuffer, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02001100
	ZEND_ME(EventHttpRequest, getBufferEvent, arginfo_class_EventHttpRequest_getBufferEvent, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, getConnection, arginfo_class_EventHttpRequest_getConnection, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, closeConnection, arginfo_class_EventHttpRequest_closeConnection, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, sendError, arginfo_class_EventHttpRequest_sendError, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, sendReply, arginfo_class_EventHttpRequest_sendReply, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, sendReplyChunk, arginfo_class_EventHttpRequest_sendReplyChunk, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, sendReplyEnd, arginfo_class_EventHttpRequest_sendReplyEnd, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, sendReplyStart, arginfo_class_EventHttpRequest_sendReplyStart, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, cancel, arginfo_class_EventHttpRequest_cancel, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, addHeader, arginfo_class_EventHttpRequest_addHeader, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, clearHeaders, arginfo_class_EventHttpRequest_clearHeaders, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, removeHeader, arginfo_class_EventHttpRequest_removeHeader, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventHttpRequest, findHeader, arginfo_class_EventHttpRequest_findHeader, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_EventListener_methods[] = {
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventListener, __construct, arginfo_class_EventListener___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventListener, __sleep, arginfo_class_EventListener___sleep, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventListener, __wakeup, arginfo_class_EventListener___wakeup, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventListener, free, arginfo_class_EventListener_free, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventListener, enable, arginfo_class_EventListener_enable, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventListener, disable, arginfo_class_EventListener_disable, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventListener, setCallback, arginfo_class_EventListener_setCallback, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventListener, setErrorCallback, arginfo_class_EventListener_setErrorCallback, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02000300
	ZEND_ME(EventListener, getBase, arginfo_class_EventListener_getBase, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(EventListener, getSocketName, arginfo_class_EventListener_getSocketName, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_EventUtil_methods[] = {
	ZEND_ME(EventUtil, __construct, arginfo_class_EventUtil___construct, ZEND_ACC_PRIVATE)
	ZEND_ME(EventUtil, getLastSocketErrno, arginfo_class_EventUtil_getLastSocketErrno, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(EventUtil, getLastSocketError, arginfo_class_EventUtil_getLastSocketError, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(EventUtil, sslRandPoll, arginfo_class_EventUtil_sslRandPoll, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#endif
	ZEND_ME(EventUtil, getSocketName, arginfo_class_EventUtil_getSocketName, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(EventUtil, getSocketFd, arginfo_class_EventUtil_getSocketFd, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(EventUtil, setSocketOption, arginfo_class_EventUtil_setSocketOption, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#if defined(PHP_EVENT_SOCKETS_SUPPORT)
	ZEND_ME(EventUtil, createSocket, arginfo_class_EventUtil_createSocket, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_EventBuffer_methods[] = {
	ZEND_ME(EventBuffer, __construct, arginfo_class_EventBuffer___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, freeze, arginfo_class_EventBuffer_freeze, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, unfreeze, arginfo_class_EventBuffer_unfreeze, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, lock, arginfo_class_EventBuffer_lock, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, unlock, arginfo_class_EventBuffer_unlock, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, enableLocking, arginfo_class_EventBuffer_enableLocking, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, add, arginfo_class_EventBuffer_add, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, read, arginfo_class_EventBuffer_read, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, addBuffer, arginfo_class_EventBuffer_addBuffer, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, appendFrom, arginfo_class_EventBuffer_appendFrom, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, expand, arginfo_class_EventBuffer_expand, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, prepend, arginfo_class_EventBuffer_prepend, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, prependBuffer, arginfo_class_EventBuffer_prependBuffer, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, drain, arginfo_class_EventBuffer_drain, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, copyout, arginfo_class_EventBuffer_copyout, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, readLine, arginfo_class_EventBuffer_readLine, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, search, arginfo_class_EventBuffer_search, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, searchEol, arginfo_class_EventBuffer_searchEol, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, pullup, arginfo_class_EventBuffer_pullup, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, write, arginfo_class_EventBuffer_write, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, readFrom, arginfo_class_EventBuffer_readFrom, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBuffer, substr, arginfo_class_EventBuffer_substr, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_EventBufferEvent_methods[] = {
	ZEND_ME(EventBufferEvent, __construct, arginfo_class_EventBufferEvent___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, free, arginfo_class_EventBufferEvent_free, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, close, arginfo_class_EventBufferEvent_close, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, connect, arginfo_class_EventBufferEvent_connect, ZEND_ACC_PUBLIC)
#if defined(HAVE_EVENT_EXTRA_LIB)
	ZEND_ME(EventBufferEvent, connectHost, arginfo_class_EventBufferEvent_connectHost, ZEND_ACC_PUBLIC)
#endif
#if !(defined(HAVE_EVENT_EXTRA_LIB))
	ZEND_ME(EventBufferEvent, connectHost, arginfo_class_EventBufferEvent_connectHost, ZEND_ACC_PUBLIC)
#endif
	ZEND_ME(EventBufferEvent, getDnsErrorString, arginfo_class_EventBufferEvent_getDnsErrorString, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, setCallbacks, arginfo_class_EventBufferEvent_setCallbacks, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, enable, arginfo_class_EventBufferEvent_enable, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, disable, arginfo_class_EventBufferEvent_disable, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, getEnabled, arginfo_class_EventBufferEvent_getEnabled, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, getInput, arginfo_class_EventBufferEvent_getInput, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, getOutput, arginfo_class_EventBufferEvent_getOutput, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, setWatermark, arginfo_class_EventBufferEvent_setWatermark, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, write, arginfo_class_EventBufferEvent_write, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, writeBuffer, arginfo_class_EventBufferEvent_writeBuffer, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, read, arginfo_class_EventBufferEvent_read, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, readBuffer, arginfo_class_EventBufferEvent_readBuffer, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, createPair, arginfo_class_EventBufferEvent_createPair, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(EventBufferEvent, setPriority, arginfo_class_EventBufferEvent_setPriority, ZEND_ACC_PUBLIC)
	ZEND_ME(EventBufferEvent, setTimeouts, arginfo_class_EventBufferEvent_setTimeouts, ZEND_ACC_PUBLIC)
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(EventBufferEvent, createSslFilter, arginfo_class_EventBufferEvent_createSslFilter, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(EventBufferEvent, sslSocket, arginfo_class_EventBufferEvent_sslSocket, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(EventBufferEvent, sslError, arginfo_class_EventBufferEvent_sslError, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(EventBufferEvent, sslRenegotiate, arginfo_class_EventBufferEvent_sslRenegotiate, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(EventBufferEvent, sslGetCipherInfo, arginfo_class_EventBufferEvent_sslGetCipherInfo, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(EventBufferEvent, sslGetCipherName, arginfo_class_EventBufferEvent_sslGetCipherName, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(EventBufferEvent, sslGetCipherVersion, arginfo_class_EventBufferEvent_sslGetCipherVersion, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(EventBufferEvent, sslGetProtocol, arginfo_class_EventBufferEvent_sslGetProtocol, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_EventSslContext_methods[] = {
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(EventSslContext, __construct, arginfo_class_EventSslContext___construct, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB) && OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
	ZEND_ME(EventSslContext, setMinProtoVersion, arginfo_class_EventSslContext_setMinProtoVersion, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB) && OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
	ZEND_ME(EventSslContext, setMaxProtoVersion, arginfo_class_EventSslContext_setMaxProtoVersion, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};
