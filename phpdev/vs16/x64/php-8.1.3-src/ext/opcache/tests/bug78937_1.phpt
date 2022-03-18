--TEST--
Bug #78937.1 (Preloading unlinkable anonymous class can segfault)
--INI--
opcache.enable=1
opcache.enable_cli=1
opcache.optimization_level=-1
opcache.preload={PWD}/preload_bug78937.inc
--EXTENSIONS--
opcache
--SKIPIF--
<?php
if (PHP_OS_FAMILY == 'Windows') die('skip Preloading is not supported on Windows');
?>
--FILE--
<?php
class Bar {
}
var_dump(foo());
?>
--EXPECTF--
Warning: Can't preload unlinked class Bar@anonymous: Unknown parent Bar in %spreload_bug78937.inc on line 3
object(Bar@anonymous)#%d (0) {
}
