--TEST--
vips can load a file
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php 
  $filename = dirname(__FILE__) . "/images/img_0076.jpg";
  $image = vips_image_new_from_file($filename)["out"];
  if ($image != FALSE) {
    echo("pass\n");
  }
?>
--EXPECT--
pass
