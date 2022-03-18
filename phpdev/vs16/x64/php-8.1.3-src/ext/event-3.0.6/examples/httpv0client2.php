<?php
/*
 * 1. Connect to 127.0.0.1 at port 80
 * by means of EventBufferEvent::connect().
 *
 * 2. Request /index.cphp via HTTP/1.0
 * using the output buffer.
 *
 * 3. Asyncronously read the response and print it to stdout.
 */

/* Read callback */
function readcb($bev, $base) {
	$input = $bev->getInput();

	while (!empty($buf = $input->read(1024))) {
		echo $buf;
	}
}

/* Event callback */
function eventcb($bev, $events, $base) {
	if ($events & EventBufferEvent::CONNECTED) {
		echo "Connected.\n";
	} elseif ($events & (EventBufferEvent::ERROR | EventBufferEvent::EOF)) {
		if ($events & EventBufferEvent::ERROR) {
			echo "DNS error: ", $bev->getDnsErrorString(), PHP_EOL;
		}

		echo "Closing\n";
		$base->exit();
		exit("Done\n");
	}
}

$base = new EventBase();

echo "step 1\n";
$bev = new EventBufferEvent($base, /* use internal socket */ NULL,
	EventBufferEvent::OPT_CLOSE_ON_FREE | EventBufferEvent::OPT_DEFER_CALLBACKS);
if (!$bev) {
	exit("Failed creating bufferevent socket\n");
}

echo "step 2\n";
$bev->setCallbacks("readcb", /* writecb */ NULL, "eventcb", $base);
$bev->enable(Event::READ | Event::WRITE);

echo "step 3\n";
/* Send request */
$output = $bev->getOutput();
if (!$output->add(
	"GET /index.cphp HTTP/1.0\r\n".
	"Connection: Close\r\n\r\n"
)) {
	exit("Failed adding request to output buffer\n");
}

/* Connect to the host syncronously.
 * We know the IP, and don't need to resolve DNS. */
if (!$bev->connect("127.0.0.1:80")) {
	exit("Can't connect to host\n");
}

/* Dispatch pending events */
$base->dispatch();
