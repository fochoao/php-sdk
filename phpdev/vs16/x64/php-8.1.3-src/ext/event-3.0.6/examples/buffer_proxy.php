<?php
/* TODO: Maybe use bufferevent pairs to complete example? */

function read_callback_proxy($bev, $other_bev) {
	/* One might use a function like this implementing
	   	a simple proxy: it will take data from one connection (on
	   	$bev), and write it to another, copying as little as
	   	possible.
	 */
	bufferevent_read_buffer($bev,
		bufferevent_get_output($other_bev));
}

?>
