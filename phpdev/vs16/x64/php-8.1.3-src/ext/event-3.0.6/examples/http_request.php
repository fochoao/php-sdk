<?php

function _request_handler($req, $base) {
	echo __FUNCTION__, PHP_EOL;

	if (is_null($req)) {
		echo "Timed out\n";
	} else {
		$response_code = $req->getResponseCode();

		if ($response_code == 0) {
			echo "Connection refused\n";
		} elseif ($response_code != 200) {
			echo "Unexpected response: $response_code\n";
		} else {
			echo "Success: $response_code\n";
			$buf = $req->getInputBuffer();
			echo "Body:\n";
			while ($s = $buf->readLine(EventBuffer::EOL_ANY)) {
				echo $s, PHP_EOL;
			}
		}
	}

	$base->exit(NULL);
}


$address = "127.0.0.1";
$port = 80;

$base = new EventBase();
$conn = new EventHttpConnection($base, NULL, $address, $port);
$conn->setTimeout(5);
$req = new EventHttpRequest("_request_handler", $base);

$req->addHeader("Host", $address, EventHttpRequest::OUTPUT_HEADER);
$req->addHeader("Content-Length", "0", EventHttpRequest::OUTPUT_HEADER);
$conn->makeRequest($req, EventHttpRequest::CMD_GET, "/index.cphp");

$base->loop();
?>
