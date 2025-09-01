#!/usr/bin/perl
use strict;
use warnings;

sub run {
    my @values = @_;
    # turn 30 floats into binary string
    my $binary = pack("f*", @values);

    # optionally base64 it or hexify for readability
    my $hex = unpack("H*", $binary);

    # Example: wrap in SKINI message
    return "RawData 0 1 $hex\r\n";
}


