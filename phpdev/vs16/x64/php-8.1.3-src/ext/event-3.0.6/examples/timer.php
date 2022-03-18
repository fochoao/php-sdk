<?php
$base = new EventBase();
$e = new Event($base, -1, Event::TIMEOUT, function($fd, $what, $e) {
	echo "1 seconds elapsed\n";
	//$e->delTimer();
});
$e->data = $e;
$e->addTimer(1);
$base->loop();
?>
