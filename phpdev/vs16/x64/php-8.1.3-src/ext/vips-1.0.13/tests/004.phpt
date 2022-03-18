--TEST--
vips can get image header fields 
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php 
  $filename = dirname(__FILE__) . "/images/img_0076.jpg";
  $image = vips_image_new_from_file($filename)["out"];
  $width = vips_image_get($image, "width")["out"];
  if ($width == 1600) {
    echo("pass\n");
  }
?>
--EXPECT--
pass
