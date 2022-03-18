--TEST--
Test compress.zlib:// scheme with the file_get_contents
--EXTENSIONS--
zlib
--FILE--
<?php
$outputFileName = __FILE__.'tmp';
$outFile = "compress.zlib://$outputFileName";
$data = <<<EOT
Here is some plain
text to be read
and displayed.
EOT;

file_put_contents($outFile, $data);
$h = gzopen($outputFileName, 'r');
gzpassthru($h);
gzclose($h);
echo "\n";
unlink($outputFileName);
?>
--EXPECT--
Here is some plain
text to be read
and displayed.
