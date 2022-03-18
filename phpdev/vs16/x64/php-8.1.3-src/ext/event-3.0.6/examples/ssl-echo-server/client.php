<?php
/*
 * Sample OpenSSL client.
 *
 * Usage:
 * 1) Launch a server, e.g.:
 * $ php examples/ssl-echo-server/server.php 9800
 *
 * 2) Launch the client in another terminal:
 * $ php examples/ssl-echo-server/client.php 9988
 *
 * Both client and server should output something similar to the following:
 * Received 5 bytes
 * ----- data ----
 * 5:	test
 */

// Allow to override the port
$port = 9999;
if ($argc > 1) {
	$port = (int) $argv[1];
}
if ($port <= 0 || $port > 65535) {
	exit("Invalid port\n");
}

class MySslEchoServerClient {
	public $port,
		$base,
		$bev,
		$ctx;

	function __construct ($port, $host = "127.0.0.1") {
		$this->port = $port;
		$this->ctx = $this->init_ssl();
		if (!$this->ctx) {
			trigger_error("Failed creating SSL context", E_USER_ERROR);
		}

		$this->base = new EventBase();
		if (!$this->base) {
			trigger_error("Failed to initialize event base", E_USER_ERROR);
		}

		$this->bev = EventBufferEvent::sslSocket($this->base, NULL, $this->ctx,
			EventBufferEvent::SSL_CONNECTING, EventBufferEvent::OPT_CLOSE_ON_FREE);
		if (!$this->bev) {
			trigger_error("Failedi to initialize buffer event", E_USER_ERROR);
		}
		$this->bev->setCallbacks(array($this, "ssl_read_cb"), NULL, array($this, "ssl_event_cb"));
		if (!$this->bev->connectHost(NULL, $host, $port, EventUtil::AF_INET)) {
			trigger_error("connectHost failed", E_USER_ERROR);
		}
		$this->bev->enable(Event::READ);
	}

	function __destruct() {
		if ($this->bev) {
			$this->bev->free();
		}
	}

	function dispatch() {
		$this->base->dispatch();
	}

	function init_ssl() {
		$local_cert = __DIR__."/cert.pem";
		$local_pk   = __DIR__."/privkey.pem";

		$ctx = new EventSslContext(EventSslContext::SSLv3_CLIENT_METHOD, array (
 			EventSslContext::OPT_LOCAL_CERT  => $local_cert,
 			EventSslContext::OPT_LOCAL_PK    => $local_pk,
             //EventSslContext::OPT_PASSPHRASE  => "test",
 			EventSslContext::OPT_ALLOW_SELF_SIGNED => true,
		));

		return $ctx;
	}

	// This callback is invoked when there is data to read on $bev.
	function ssl_read_cb($bev, $ctx) {
		$in = $bev->input; //$bev->getInput();

		printf("Received %ld bytes\n", $in->length);
    	printf("----- data ----\n");
    	printf("%ld:\t%s\n", (int) $in->length, $in->pullup(-1));

		$this->bev->free();
		$this->bev = NULL;
		$this->base->exit(NULL);
	}

	// This callback is invoked when some even occurs on the event listener,
	// e.g. connection closed, or an error occured
	function ssl_event_cb($bev, $events, $ctx) {
		if ($events & EventBufferEvent::ERROR) {
			// Fetch errors from the SSL error stack
			while ($err = $bev->sslError()) {
				fprintf(STDERR, "Bufferevent error %s.\n", $err);
			}
		}

		if ($events & (EventBufferEvent::EOF | EventBufferEvent::ERROR)) {
			$bev->free();
			$bev = NULL;
		} elseif ($events & EventBufferEvent::CONNECTED) {
			$bev->output->add("test\n");
		}
	}
}

$cl = new MySslEchoServerClient($port);
$cl->dispatch();
?>
