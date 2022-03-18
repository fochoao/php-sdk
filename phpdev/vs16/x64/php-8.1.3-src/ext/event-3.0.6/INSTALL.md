INSTALLATION OF EVENT PECL EXTENSION
====================================

Tested under GNU/Linux only. But likely will work on others too. The following
also concerns GNU/Linux. If you find it useful to publish instructions for
other platforms, please drop me a note.


PRELIMINARIES
-------------

Event extension supports `libevent 2.0` or greater. It expects at least
<libevent_core.so> library to be installed. For extra functions
<libevent_extra.so>) is required. However, since `libevent 2.0` both should
come with the libevent distribution.

Note that <libevent.so> exists for historical reasons. Currently it contains
the contents of both <libevent_core.so> and <libevent_extra.so>. But using
<libevent.so> is not reliable as it may go away in future releases.

Most OS distributions have `libevent` package in their repositories:

*Debian*, *Ubuntu* and similar:
	# apt-get install libevent-dev

*Gentoo*
	# emerge dev-libs/libevent
	(`ssl` USE-flag may be needed)

*openSUSE*:
	# zypper in libevent

In a pinch the source code is always available on `libevent`'s homepage:
<http://libevent.org/>


AUTOMATIC INSTALLATION
----------------------

Just run the following as `root`:

	# pecl install package.xml


MANUAL INSTALLATION
-------------------

Clone the project or download it as archive. In the package directory run:

	$ phpize
	$ ./configure --with-event-core --with-event-extra --enable-event-debug
	$ make

Optionally run tests:

	$ make test

Install it (as `root`):

	# make install

##NOTES

Methods of the Event extension accept different types of resources containing a
file descriptor: a castable PHP stream, socket resource, or just a number(the
file descriptor itself). If you don't have _sockets_ extension installed, or
just don't plan to use the standard PHP sockets, then configure Event with
`--disable-event-sockets` option, or choose `n`("No") when the PEAR installer
asks whether `sockets` support is required.


FINALLY
-------

In <php.ini>, or some other configuration like
</usr/local/etc/php/conf.d/ev.ini> write:

	extension=event.so


vim: ft=markdown tw=80
