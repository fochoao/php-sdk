<?php
$base = new EventBase();
$http = new EventHttp($base);

$addresses = array (
 	8091 => "127.0.0.1",
 	8092 => "127.0.0.2",
);
$i = 0;

$socket = array();

foreach ($addresses as $port => $ip) {
	echo $ip, " ", $port, PHP_EOL;
	$socket[$i] = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if (!socket_bind($socket[$i], $ip, $port)) {
		exit("socket_bind failed\n");
	}
	socket_listen($socket[$i], 0);
	socket_set_nonblock($socket[$i]);

	if (!$http->accept($socket[$i])) {
		echo "Accept failed\n";
		exit(1);
	}

	++$i;
}

$http->setDefaultCallback(function($req) {
	echo "URI: ", $req->getUri(), PHP_EOL;
	$req->sendReply(200, "OK");
	echo "OK\n";
});

$signal = Event::signal($base, SIGINT, function () use ($base) {
	echo "Caught SIGINT. Stopping...\n";
	$base->stop();
});
$signal->add();

$base->dispatch();
echo "END\n";
// We didn't close sockets, since Libevent already sets CLOSE_ON_FREE and CLOSE_ON_EXEC flags on the file 
// descriptor associated with the sockets.
?>
