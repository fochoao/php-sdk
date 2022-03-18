<?php
function write_callback_fibonacci($bev, $c) {
	/* Here's a callback that adds some Fibonacci numbers to the
	   	output buffer of $bev.  It stops once we have added 1k of
	   	data; once this data is drained, we'll add more. */

	$tmp = new EventBuffer();
	while ($tmp->length < 1024) {
		$next = $c[0] + $c[1];
		$c[0] = $c[1];
		$c[1] = $next;

		$tmp->add($next);
	}

	// Now we add the whole contents of tmp to bev
	$bev->writeBuffer($tmp);

	// We don't need tmp any longer
	$tmp->free();
}
?>
