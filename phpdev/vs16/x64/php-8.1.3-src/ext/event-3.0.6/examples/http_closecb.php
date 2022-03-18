<?php
/*
 * Setting up close-connection callback
 *
 * The script handles closed connections using HTTP API.
 *
 * Usage:
 * 1) Launch the server:
 * $ php examples/http_closecb.php 4242
 *
 * 2) Launch a client in another terminal. Telnet-like
 * session should look like the following:
 *
 * $ nc -t 127.0.0.1 4242
 * GET / HTTP/1.0
 * Connection: close
 *
 * The server will output something similar to the following:
 *
 * HTTP/1.0 200 OK
 * Content-Type: multipart/x-mixed-replace;boundary=boundarydonotcross
 * Connection: close
 *
 * <html>
 *
 * 3) Terminate the client connection abruptly,
 * i.e. kill the process, or just press Ctrl-C.
 *
 * 4) Check if the server called _close_callback.
 * The script should output "_close_callback" string to standard output.
 *
 * 5) Check if the server's process has no orphaned connections,
 * e.g. with `lsof` utility.
 */

function _close_callback($conn) {
	echo __FUNCTION__, PHP_EOL;
}

function _http_default($req, $dummy) {
	$conn = $req->getConnection();
	$conn->setCloseCallback('_close_callback', NULL);

	/*
	By enabling Event::READ we protect the server against unclosed conections.
	This is a peculiarity of Libevent. The library disables Event::READ events
 	on this connection, and the server is not notified about terminated
	connections.

	So each time client terminates connection abruptly, we get an orphaned
	connection. For instance, the following is a part of `lsof -p $PID | grep TCP`
	command after client has terminated connection:

	57-php     15057 ruslan  6u  unix 0xffff8802fb59c780   0t0  125187 socket
	58:php     15057 ruslan  7u  IPv4             125189   0t0     TCP *:4242 (LISTEN)
	59:php     15057 ruslan  8u  IPv4             124342   0t0     TCP localhost:4242->localhost:37375 (CLOSE_WAIT)

	where $PID is our process ID. 

	The following block of code fixes such kind of orphaned connections.
	 */
	$bev = $req->getBufferEvent();
	$bev->enable(Event::READ);

	$req->addHeader('Content-Type',
		'multipart/x-mixed-replace;boundary=boundarydonotcross',
		EventHttpRequest::OUTPUT_HEADER);

	$buf = new EventBuffer();
	$buf->add('<html>');

	$req->sendReply(200, "OK");
	$req->sendReplyChunk($buf);
}

$port = 4242;
if ($argc > 1) {
	$port = (int) $argv[1];
}
if ($port <= 0 || $port > 65535) {
	exit("Invalid port");
}

$base = new EventBase();
$http = new EventHttp($base);

$http->setDefaultCallback("_http_default", NULL);
$http->bind("0.0.0.0", $port);
$base->loop();
