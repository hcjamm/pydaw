#!/usr/bin/perl

#usage:
#perl png_to_base64.pl < your_image.png > your_base64_encoded_image.png

use MIME::Base64 qw(encode_base64);
local($/) = undef;  # slurp
$result =  '"<html><img src=\"data:image/png;base64,' . encode_base64(<STDIN>) . '\"/></html>"';

$result =~ s/\n//g;

print $result;


