<?php
function base64_encode_image ($filename=string,$filetype=string) {
    if ($filename) {
        $imgbinary = fread(fopen($filename, "r"), filesize($filename));
        return 'data:image/' . $filetype . ';base64,' . base64_encode($imgbinary);
    }
}

echo '"<html><img src=\"' . base64_encode_image ($argv[1],'png') . '\"/></html>"';
?>

