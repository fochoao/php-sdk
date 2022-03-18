Event - PECL extension
======================

Event is a PECL extension providing interface to `libevent` C library.

ABOUT LIBEVENT
--------------

The `libevent` API provides a mechanism to execute a callback function when a
specific event occurs on a file descriptor or after a timeout has been reached.
Furthermore, libevent also support callbacks due to *signals* or regular
*timeouts*.

`libevent` is meant to replace the event loop found in event driven network
servers. An application just needs to call `event_dispatch()` and then add or
remove events dynamically without having to change the event loop.


SEE ALSO
========

* [Libevent homepage](http://libevent.org/)
* [Installation instructions](INSTALL.md).


AUTHOR
======

Ruslan Osmanov <osmanov@php.net>


LICENSE
=======

[PHP 3.01](LICENSE)
