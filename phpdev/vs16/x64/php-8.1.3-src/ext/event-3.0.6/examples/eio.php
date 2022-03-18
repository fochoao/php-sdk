<?php
/*
 * Dispatch eio_*() via event
 */

/* Callback for eio_nop() */
function my_nop_cb($d, $r) {
	echo "step 6\n";
}

$dir = "/tmp/abc-eio-temp";
if (file_exists($dir)) {
	rmdir($dir);
}

echo "step 1\n";

$base = new EventBase();

echo "step 2\n";

eio_init();

eio_mkdir($dir, 0750, EIO_PRI_DEFAULT, "my_nop_cb");

$event = new Event($base, eio_get_event_stream(),
	Event::READ | Event::PERSIST, function ($fd, $events, $base) {
	echo "step 5\n";

	while (eio_nreqs()) {
		eio_poll();
	}

	$base->stop();
}, $base);

echo "step 3\n";

$event->add();

echo "step 4\n";

$base->dispatch();

echo "Done\n";
