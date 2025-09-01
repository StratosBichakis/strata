#!/usr/bin/perl
use strict;
use warnings;

sub make_note_on {
    my ($pitch, $vel) = @_;
    return "NoteOn 0 1 $pitch $vel\r\n";
}

sub make_note_off {
    my ($pitch) = @_;
    return "NoteOff 0 1 $pitch 64.0\r\n";
}