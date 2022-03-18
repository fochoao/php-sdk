<?php
/* TODO: Maybe use bufferevent pairs to complete example? */

function read_callback_uppercase($bev, $unused) {
	/* This callback removes the data from $bev's input buffer 128
		bytes at a time, uppercases it, and starts sending it
		back.
	 */

	$tmp = NULL;

	while (1) {
		$tmp = $bev->read(128);
		(!empty($tmp)) or break;
		$bev->write(strtoupper($tmp), $n);
	}
}
?>
