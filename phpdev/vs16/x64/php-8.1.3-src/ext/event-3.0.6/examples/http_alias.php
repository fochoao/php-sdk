<?php

$base = new EventBase();
$http = new EventHttp($base);

$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

if (!$http->bind("127.0.0.1", 8088)) {
	exit("bind(1) failed\n");
};

if (!$http->addServerAlias("local.net")) {
	exit("Failed to add server alias\n");
}

$http->setCallback("/about", function($req) {
	echo "URI: ", $req->getUri(), PHP_EOL;
	$req->sendReply(200, "OK");
	echo "OK\n";
});
$base->dispatch();

?>
