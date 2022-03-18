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
#ifndef PHP_EVENT_STRUCTS_H
#define PHP_EVENT_STRUCTS_H

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

/* Object's internal struct name */
#define Z_EVENT_X_OBJ_T(x) php_event_ ## x ## _t

#define PHP_EVENT_OBJECT_TAIL \
	HashTable   *prop_handler; \
	zend_object  zo

typedef struct _php_event_callback_t {
	zval                  func_name;
	zend_fcall_info_cache fci_cache;
} php_event_callback_t;

/* EventBase object */
typedef struct _php_event_base_t {
	struct event_base *base;
	zend_bool          internal;   /* Whether is obtained with evconnlistener_get_base() */

	PHP_EVENT_OBJECT_TAIL;
} Z_EVENT_X_OBJ_T(base);

/* Event object */
typedef struct _php_event_t {
	struct event         *event;       /* Pointer returned by event_new     */
	zend_resource        *stream_res;  /* Resource with the file descriptor */
	zval                  data;        /* User custom data                  */
	php_event_callback_t  cb;

	PHP_EVENT_OBJECT_TAIL;
} php_event_t;
typedef php_event_t Z_EVENT_X_OBJ_T(event);

/* EventConfig object */
typedef struct _php_event_config_t {
	struct event_config *ptr;

	PHP_EVENT_OBJECT_TAIL;
} Z_EVENT_X_OBJ_T(config);

/* EventBufferEvent object */
typedef struct _php_event_bevent_t {
	struct bufferevent   *bevent;
	int                   _internal;
	zval                  self;        /* Object itself. For callbacks */
	zval                  data;        /* User callback data           */
	zval                  input;       /* Input buffer                 */
	zval                  output;      /* Output buffer                */
	zval                  base;
	php_event_callback_t  cb_read;
	php_event_callback_t  cb_write;
	php_event_callback_t  cb_event;

	PHP_EVENT_OBJECT_TAIL;
} Z_EVENT_X_OBJ_T(bevent);

/* EventBuffer object */
typedef struct _php_event_buffer_t {
	zend_bool internal; /* Whether is an internal buffer of a bufferevent */
	struct evbuffer *buf;

	PHP_EVENT_OBJECT_TAIL;
} Z_EVENT_X_OBJ_T(buffer);

#ifdef HAVE_EVENT_EXTRA_LIB/* {{{ */
enum {
	PHP_EVENT_REQ_HEADER_INPUT  = 1,
	PHP_EVENT_REQ_HEADER_OUTPUT = 2,
};

/* EventDnsBase object */
typedef struct _php_event_dns_base_t {
	struct evdns_base *dns_base;

	PHP_EVENT_OBJECT_TAIL;
} Z_EVENT_X_OBJ_T(dns_base);

/* EventListener object */
typedef struct _php_event_listener_t {
	struct evconnlistener *listener;
	zval                   self;       /* Object itself. For callbacks        */
	zval                   data;       /* User custom data passed to callback */
	php_event_callback_t   cb;         /* Accept callback                     */
	php_event_callback_t   cb_err;     /* Error callback                      */

	PHP_EVENT_OBJECT_TAIL;
} Z_EVENT_X_OBJ_T(listener);

/* Type for an HTTP server callback */
typedef struct _php_event_http_cb_t php_event_http_cb_t;
struct _php_event_http_cb_t {
	php_event_http_cb_t  *next;   /* Linked list                         */
	zval                  data;   /* User custom data passed to callback */
	zval                  base;
	php_event_callback_t  cb;
};

/* EventHttp object */
typedef struct _php_event_http_t {
	struct evhttp        *ptr;
	zval                  base;      /* Event base associated with the listener              */
	zval                  data;      /* User custom data passed to the gen(default) callback */
	php_event_callback_t  cb;        /* Callback for evhttp_gencb()                          */
	php_event_http_cb_t  *cb_head;   /* Linked list of attached callbacks                    */

	PHP_EVENT_OBJECT_TAIL;
} Z_EVENT_X_OBJ_T(http);

/* EventHttpConnection object */
typedef struct _php_event_http_conn_t {
	struct evhttp_connection *conn;
	zval                      base;           /* Event base associated with the listener */
	zval                      dns_base;       /* Associated EventDnsBase                 */
	zval                      self;           /* $this                                   */
	zval                      data_closecb;   /* User data for cb_close                  */
	php_event_callback_t      cb_close;       /* Connection close callback               */
	zend_bool                 internal;       /* Whether to free conn pointer            */

	PHP_EVENT_OBJECT_TAIL;
} Z_EVENT_X_OBJ_T(http_conn);

/* EventHttpRequest object */
typedef struct {
	struct evhttp_request *ptr;
	zend_bool              internal;   /* Whether is created internally(must not free 'ptr') */
	zval                   self;       /* $this                                              */
	zval                   data;       /* User data for cb                                   */
	php_event_callback_t   cb;         /* Callback for evhttp_gencb()                        */

	PHP_EVENT_OBJECT_TAIL;
} Z_EVENT_X_OBJ_T(http_req);

#endif/* HAVE_EVENT_EXTRA_LIB }}} */

#ifdef HAVE_EVENT_OPENSSL_LIB/*{{{*/
enum {
	PHP_EVENT_OPT_LOCAL_CERT               = 1,
	PHP_EVENT_OPT_LOCAL_PK                 = 2,
	PHP_EVENT_OPT_PASSPHRASE               = 3,
	PHP_EVENT_OPT_CA_FILE                  = 4,
	PHP_EVENT_OPT_CA_PATH                  = 5,
	PHP_EVENT_OPT_ALLOW_SELF_SIGNED        = 6,
	PHP_EVENT_OPT_VERIFY_PEER              = 7,
	PHP_EVENT_OPT_VERIFY_DEPTH             = 8,
	PHP_EVENT_OPT_CIPHERS                  = 9,
	PHP_EVENT_OPT_NO_SSLv2                 = 10,
	PHP_EVENT_OPT_NO_SSLv3                 = 11,
	PHP_EVENT_OPT_NO_TLSv1                 = 12,
	PHP_EVENT_OPT_NO_TLSv1_1               = 13,
	PHP_EVENT_OPT_NO_TLSv1_2               = 14,
	PHP_EVENT_OPT_CIPHER_SERVER_PREFERENCE = 15,
	PHP_EVENT_OPT_REQUIRE_CLIENT_CERT      = 16,
	PHP_EVENT_OPT_VERIFY_CLIENT_ONCE       = 17
};

enum {
	PHP_EVENT_SSLv2_CLIENT_METHOD  = 1,
	PHP_EVENT_SSLv3_CLIENT_METHOD  = 2,
	PHP_EVENT_SSLv23_CLIENT_METHOD = 3,
	PHP_EVENT_TLS_CLIENT_METHOD    = 4,
	PHP_EVENT_SSLv2_SERVER_METHOD  = 5,
	PHP_EVENT_SSLv3_SERVER_METHOD  = 6,
	PHP_EVENT_SSLv23_SERVER_METHOD = 7,
	PHP_EVENT_TLS_SERVER_METHOD    = 8,
	PHP_EVENT_TLSv11_CLIENT_METHOD = 9,
	PHP_EVENT_TLSv11_SERVER_METHOD = 10,
	PHP_EVENT_TLSv12_CLIENT_METHOD = 11,
	PHP_EVENT_TLSv12_SERVER_METHOD = 12
};

/* EventSslContext object */
typedef struct _php_event_ssl_context_t {
	SSL_CTX   *ctx;
	HashTable *ht;
	zend_bool  allow_self_signed;

	PHP_EVENT_OBJECT_TAIL;
} Z_EVENT_X_OBJ_T(ssl_context);
#endif /* HAVE_EVENT_OPENSSL_LIB }}} */

typedef double php_event_timestamp_t;

/* Property handler types */
typedef zval *(*php_event_prop_read_t)(void *obj, zval *retval);
typedef int (*php_event_prop_write_t)(void *obj, zval *newval);
typedef zval *(*php_event_prop_get_ptr_ptr_t)(void *obj);

/* Property entry */
typedef struct _php_event_property_entry_t {
	const char                   *name;
	size_t                        name_length;
	php_event_prop_read_t         read_func;
	php_event_prop_write_t        write_func;
	php_event_prop_get_ptr_ptr_t  get_ptr_ptr_func;
} php_event_property_entry_t;

/* Property entry handler */
typedef struct _php_event_prop_handler_t {
	zend_string                  *name;
	php_event_prop_read_t         read_func;
	php_event_prop_write_t        write_func;
	php_event_prop_get_ptr_ptr_t  get_ptr_ptr_func;
} php_event_prop_handler_t;

#ifndef LIBEVENT_VERSION_NUMBER
# error "<event2/*.h> must be included before " ## #__FILE__
#endif

#if LIBEVENT_VERSION_NUMBER < 0x02000200
/* These types are introduced in libevent 2.0.2-alpha */
typedef void (*bufferevent_data_cb)(struct bufferevent *bev, void *ctx);
typedef void (*bufferevent_event_cb)(struct bufferevent *bev, short events, void *ctx);
#endif

#endif /* PHP_EVENT_STRUCTS_H */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
