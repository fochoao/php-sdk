<?php
/*
 * Simple HTTPS server.
 *
 * 1) Run the server: `php examples/https.php 9999`
 * 2) Test it: `php examples/ssl-connection.php 9999`
 */

function _http_dump($req, $data) {
	static $counter      = 0;
	static $max_requests = 200;

	if (++$counter >= $max_requests)  {
		echo "Counter reached max requests $max_requests. Exiting\n";
		exit();
	}

	echo __METHOD__, " called\n";
	echo "request:"; var_dump($req);
	echo "data:"; var_dump($data);

	echo "\n===== DUMP =====\n";
	echo "Command:", $req->getCommand(), PHP_EOL;
	echo "URI:", $req->getUri(), PHP_EOL;
	echo "Input headers:"; var_dump($req->getInputHeaders());
	echo "Output headers:"; var_dump($req->getOutputHeaders());

	echo "\n >> Sending reply ...";
	$req->sendReply(200, "OK");
	echo "OK\n";

	$buf = $req->getInputBuffer();
	echo "\n >> Reading input buffer (", $buf->length, ") ...\n";
	while ($s = $buf->read(1024)) {
		echo $s;
	}
	echo "\nNo more data in the buffer\n";
}

function _http_about($req) {
	echo __METHOD__, PHP_EOL;
	echo "URI: ", $req->getUri(), PHP_EOL;
	echo "\n >> Sending reply ...";
	$req->sendReply(200, "OK");
	echo "OK\n";
}

function _http_default($req, $data) {
	echo __METHOD__, PHP_EOL;
	echo "URI: ", $req->getUri(), PHP_EOL;
	echo "\n >> Sending reply ...";
	$req->sendReply(200, "OK");
	echo "OK\n";
}

function _http_400($req) {
	$req->sendError(400);
}

function _init_ssl() {
	$local_cert = __DIR__."/ssl-echo-server/cert.pem";
	$local_pk   = __DIR__."/ssl-echo-server/privkey.pem";

	$ctx = new EventSslContext(EventSslContext::SSLv3_SERVER_METHOD, array (
		EventSslContext::OPT_LOCAL_CERT  => $local_cert,
		EventSslContext::OPT_LOCAL_PK    => $local_pk,
		//EventSslContext::OPT_PASSPHRASE  => "test",
		EventSslContext::OPT_ALLOW_SELF_SIGNED => true,
	));

	return $ctx;
}

$port = 9999;
if ($argc > 1) {
	$port = (int) $argv[1];
}
if ($port <= 0 || $port > 65535) {
	exit("Invalid port");
}
$ip = '0.0.0.0';

$base = new EventBase();
$ctx  = _init_ssl();
$http = new EventHttp($base, $ctx);
$http->setAllowedMethods(EventHttpRequest::CMD_GET | EventHttpRequest::CMD_POST);

$http->setCallback("/dump", "_http_dump", array(4, 8));
$http->setCallback("/about", "_http_about");
$http->setCallback("/err400", "_http_400");
$http->setDefaultCallback("_http_default", "custom data value");

$http->bind($ip, $port);
$base->dispatch();
