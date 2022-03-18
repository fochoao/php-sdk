dnl +----------------------------------------------------------------------+
dnl | PHP Version 8                                                        |
dnl +----------------------------------------------------------------------+
dnl | Copyrght (C) 1997-2020 The PHP Group                                 |
dnl +----------------------------------------------------------------------+
dnl | This source file is subject to version 3.01 of the PHP license,      |
dnl | that is bundled with this package in the file LICENSE, and is        |
dnl | available through the world-wide-web at the following url:           |
dnl | http://www.php.net/license/3_01.txt                                  |
dnl | If you did not receive a copy of the PHP license and are unable to   |
dnl | obtain it through the world-wide-web, please send a note to          |
dnl | license@php.net so we can mail you a copy immediately.               |
dnl +----------------------------------------------------------------------+
dnl | Author: Ruslan Osmanov <osmanov@php.net>                             |
dnl +----------------------------------------------------------------------+

PHP_ARG_WITH(event-core, for Event core support,
[  --with-event-core        Include core libevent support])

PHP_ARG_WITH(event-pthreads, for Event thread safety support,
[  ---event-pthreads    Include libevent's pthreads library and enable thread safety support in Event], no, no)

PHP_ARG_WITH(event-extra, for Event extra functionality support,
[  --with-event-extra       Include libevent protocol-specific functionality support including HTTP, DNS, and RPC], yes, no)

PHP_ARG_WITH(event-openssl, for OpenSSL support in Event,
[  --with-event-openssl Include libevent OpenSSL support], yes, no)

PHP_ARG_WITH(event-ns, for custom PHP namespace in Event,
[  --with-event-ns[=NS] Set custom PHP namespace for all Event classes], no, no)

PHP_ARG_WITH(openssl-dir, for OpenSSL installation prefix,
[  --with-openssl-dir[=DIR]  Event: openssl installation prefix], no, no)

PHP_ARG_WITH([event-libevent-dir], [],
[  --with-event-libevent-dir[=DIR] Event: libevent installation prefix], no, no)

PHP_ARG_ENABLE(event-debug, whether Event debugging support enabled,
[  --enable-event-debug     Enable debug support in Event], no, no)

PHP_ARG_ENABLE(event-sockets, whether to enable sockets support in Event,
[  --enable-event-sockets Enable sockets support in Event], yes, no)

if test "$PHP_EVENT_CORE" != "no"; then

  OLD_LDFLAGS=$LDFLAGS
  OLD_LIBS=$LIBS

  dnl {{{ Check for PHP version
  AC_MSG_CHECKING(PHP version)
  if test -d $abs_srcdir/php7; then
    dnl # only for PECL, not for PHP
    export OLD_CPPFLAGS="$CPPFLAGS"
    export CPPFLAGS="$CPPFLAGS $INCLUDES"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <php_version.h>]], [[
    #if PHP_MAJOR_VERSION > 5
    # error PHP > 5
    #endif
    ]])],[
      PHP_EVENT_SUBDIR=php5
      AC_MSG_RESULT([PHP 5.x])
    ],[
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <php_version.h>]], [[
      #if PHP_MAJOR_VERSION > 7
      # error PHP > 7
      #endif
      ]])],[
        PHP_EVENT_SUBDIR=php7
        AC_MSG_RESULT([PHP 7.x])
      ],[
        PHP_EVENT_SUBDIR=php8
        AC_MSG_RESULT([PHP 8.x])
      ])
    ])
    export CPPFLAGS="$OLD_CPPFLAGS"

    PHP_ADD_BUILD_DIR($abs_builddir/$PHP_EVENT_SUBDIR, 1)
    PHP_ADD_INCLUDE([$ext_srcdir/$PHP_EVENT_SUBDIR])
  else
    AC_MSG_ERROR([unknown source])
    PHP_EVENT_SUBDIR="."
  fi
  dnl }}}

  dnl {{{ --enable-event-debug
  if test "$PHP_EVENT_DEBUG" != "no"; then
    CFLAGS="$CFLAGS -Wall -g -ggdb -O0"
    AC_DEFINE(PHP_EVENT_DEBUG, 1, [Enable event debug support])
  else
    AC_DEFINE(NDEBUG, 1, [With NDEBUG defined assert generates no code])
  fi
  dnl }}}

  dnl {{{ Include libevent headers
  AC_MSG_CHECKING([for include/event2/event.h])
  EVENT_DIR=
  for i in "$PHP_EVENT_CORE" "$PHP_EVENT_LIBEVENT_DIR" /usr/local /usr /opt /opt/local; do
    if test -f "$i/include/event2/event.h"; then
      EVENT_DIR=$i
      break
    fi
  done

  if test "x$EVENT_DIR" = "x"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the event library, or provide the installation prefix via --with-event-libevent-dir option])
  fi
  AC_MSG_RESULT([found in $EVENT_DIR])

  PHP_ADD_INCLUDE($EVENT_DIR/include)
  dnl }}}

  dnl {{{ Check if it's at least libevent 2.0.2-alpha
  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES -DHAVE_EV"
  AC_MSG_CHECKING(for libevent version)
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <event2/event.h>]], [[
  #if LIBEVENT_VERSION_NUMBER < 0x02000200
  # error this extension requires at least libevent version 2.0.2-alpha
  #endif
  ]])],
  [AC_MSG_RESULT(ok)],
  [AC_MSG_ERROR([need at least libevent 2.0.2-alpha])])
  export CPPFLAGS="$OLD_CPPFLAGS"
  dnl }}}

  if test -d $EVENT_DIR/$PHP_LIBDIR/event2; then
    dnl FreeBSD
    EVENT_LIBS="-L$EVENT_DIR/$PHP_LIBDIR -L$EVENT_DIR/$PHP_LIBDIR/event2"
    EVENT_LIBDIR=$EVENT_DIR/$PHP_LIBDIR/event2
  else
    EVENT_LIBS="-L$EVENT_DIR/$PHP_LIBDIR"
    EVENT_LIBDIR=$EVENT_DIR/$PHP_LIBDIR
  fi
  LDFLAGS="$EVENT_LIBS $LDFLAGS"
  LIBS="$LIBS -levent_core"

  dnl {{{ event_core
  AC_CHECK_LIB(event_core, event_free, [
    PHP_ADD_LIBRARY_WITH_PATH(event_core, $EVENT_LIBDIR, EVENT_SHARED_LIBADD)
  ], [
    AC_MSG_ERROR([event_free not found in event_core library, or the library is not installed])
  ])

  event_src="$PHP_EVENT_SUBDIR/php_event.c \
    $PHP_EVENT_SUBDIR/src/util.c \
    $PHP_EVENT_SUBDIR/src/fe.c \
    $PHP_EVENT_SUBDIR/src/pe.c \
    $PHP_EVENT_SUBDIR/classes/event.c \
    $PHP_EVENT_SUBDIR/classes/base.c \
    $PHP_EVENT_SUBDIR/classes/event_config.c \
    $PHP_EVENT_SUBDIR/classes/buffer_event.c \
    $PHP_EVENT_SUBDIR/classes/buffer.c \
    $PHP_EVENT_SUBDIR/classes/event_util.c"
  dnl }}}

  dnl {{{ --with-event-pthreads
  if test "$PHP_EVENT_PTHREADS" != "no"; then
    AC_CHECK_LIB(event_pthreads, evthread_use_pthreads, [
      PHP_ADD_LIBRARY_WITH_PATH(event_pthreads, $EVENT_LIBDIR, EVENT_SHARED_LIBADD)
      LDFLAGS="$LDFLAGS -lpthread -levent_pthreads"
      AC_DEFINE(HAVE_EVENT_PTHREADS_LIB, 1, [ ])
    ], [
      AC_MSG_ERROR([evthread_use_pthreads not found in event_pthreads library, or the library is not installed])
    ])
  fi
  dnl }}}

  dnl {{{ --with-event-extra
  if test "$PHP_EVENT_EXTRA" != "no"; then
    AC_CHECK_LIB(event_extra, evdns_base_free, [
      PHP_ADD_LIBRARY_WITH_PATH(event_extra, $EVENT_LIBDIR, EVENT_SHARED_LIBADD)
      LDFLAGS="$LDFLAGS -levent_extra"
      AC_DEFINE(HAVE_EVENT_EXTRA_LIB, 1, [ ])
    ], [
      AC_MSG_ERROR([evdns_base_free not found in event_extra library, or the library is not installed])
    ])

    event_src="$event_src \
      $PHP_EVENT_SUBDIR/classes/dns.c \
      $PHP_EVENT_SUBDIR/classes/listener.c \
      $PHP_EVENT_SUBDIR/classes/http.c \
      $PHP_EVENT_SUBDIR/classes/http_request.c \
      $PHP_EVENT_SUBDIR/classes/http_connection.c"
  fi
  dnl }}}

  dnl {{{ --with-event-openssl
  if test "$PHP_EVENT_OPENSSL" != "no"; then
    test -z "$PHP_OPENSSL" && PHP_OPENSSL=no

    if test -z "$PHP_OPENSSL_DIR" || test $PHP_OPENSSL_DIR == "no"; then
      PHP_OPENSSL_DIR=yes
    else
      PHP_OPENSSL="$PHP_OPENSSL_DIR"
    fi

    PHP_SETUP_OPENSSL(EVENT_SHARED_LIBADD)

    AC_CHECK_LIB(event_openssl, bufferevent_openssl_get_ssl, [
      PHP_ADD_LIBRARY_WITH_PATH(event_openssl, $EVENT_LIBDIR, EVENT_SHARED_LIBADD)
      LDFLAGS="$LDFLAGS -levent_openssl"
      AC_DEFINE(HAVE_EVENT_OPENSSL_LIB, 1, [ ])
    ], [
      AC_MSG_ERROR([bufferevent_openssl_get_ssl not found in event_openssl library, or the library is not installed])
    ])

    event_src="$event_src $PHP_EVENT_SUBDIR/classes/ssl_context.c"
  fi
  dnl }}}

  if test "$PHP_EVENT_NS" != "no" -a "$PHP_EVENT_NS" != "yes"; then
    if test "x$SED" = "x"; then
      AC_PATH_PROG(SED, sed)
    fi
    PHP_EVENT_NS=$(echo "$PHP_EVENT_NS" | $SED -e 's/\\/\\\\/g')
    PHP_EVENT_ALIAS_PREFIX="${PHP_EVENT_NS}\\"
    AC_DEFINE_UNQUOTED(PHP_EVENT_NS, ["$PHP_EVENT_NS"], [Custom PHP namespace for all Event classes])
    AC_DEFINE_UNQUOTED(PHP_EVENT_NS_RAW, [$PHP_EVENT_NS], [Custom PHP namespace for all Event classes for macros])
  else
    PHP_EVENT_NS=
    PHP_EVENT_ALIAS_PREFIX=
  fi
  PHP_EVENT_STUB_PHP_IN="$PHP_EVENT_SUBDIR/php_event.stub.php.in"
  if test -e "$PHP_EVENT_STUB_PHP_IN"; then
    PHP_EVENT_STUB_PHP="$PHP_EVENT_SUBDIR/php_event.stub.php"
    AC_SUBST(PHP_EVENT_NS)
    AC_SUBST(PHP_EVENT_ALIAS_PREFIX)
    AC_CONFIG_FILES(["$PHP_EVENT_STUB_PHP":"$PHP_EVENT_STUB_PHP_IN"])
    dnl# $SED -i.bak -e 's/^namespace *PHP_EVENT_NS/namespace '"$PHP_EVENT_NS"'/g' "$PHP_EVENT_STUB_PHP_IN"
  fi

  PHP_NEW_EXTENSION(event, $event_src, $ext_shared,,$CFLAGS -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
  PHP_ADD_BUILD_DIR($ext_builddir/$PHP_EVENT_SUBDIR/src)
  PHP_ADD_BUILD_DIR($ext_builddir/$PHP_EVENT_SUBDIR/classes)
  PHP_ADD_INCLUDE($ext_builddir/$PHP_EVENT_SUBDIR/src)
  PHP_ADD_INCLUDE($ext_builddir/$PHP_EVENT_SUBDIR/classes)
  PHP_ADD_INCLUDE($ext_builddir/$PHP_EVENT_SUBDIR)
  PHP_SUBST(EVENT_SHARED_LIBADD)

  LDFLAGS=$OLD_LDFLAGS
  LIBS=$OLD_LIBS

  dnl This works with static building only
  dnl test -z $PHP_SOCKETS && PHP_SOCKETS="no"

  if test "$PHP_EVENT_SOCKETS" != "no"; then
    AC_CHECK_HEADERS([$phpincludedir/ext/sockets/php_sockets.h], ,
      [
        AC_MSG_ERROR([Couldn't find $phpincludedir/sockets/php_sockets.h. Please check if sockets extension installed])
      ]
    )
    PHP_ADD_EXTENSION_DEP(event, sockets)
    AC_DEFINE(PHP_EVENT_SOCKETS, 1, [Whether sockets extension is required])
    dnl Hack for distroes installing sockets separately
    AC_DEFINE(HAVE_SOCKETS, 1, [Whether sockets extension is enabled])
  fi

  PHP_ADD_MAKEFILE_FRAGMENT
fi

dnl vim: ft=m4.sh fdm=marker cms=dnl\ %s
dnl vim: et ts=2 sts=2 sw=2
