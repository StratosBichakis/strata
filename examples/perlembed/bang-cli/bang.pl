use strict;
use warnings;

use lib '.';
require "helpers.pl";

# Renamed from 'bang' to '_bang_event' - this is the actual event trigger logic
sub _bang {
    my ($self, $message_from_c) = @_;

    my $bangs = $self->{bangs};
    my $tempo = $self->{tempo};
    $self->{tempo} = 134.4;

#     do "helpers.pl";
    my @floats;
    if (ref($message_from_c) eq 'ARRAY') {
        @floats = @{$message_from_c};
    } else {
        # This case should not happen if C++ is always sending an array ref
        # print STDERR "Perl received a non-array message: '$message_from_c'\n";
    }
    $self->{counter} //= 0; #unless (defined $self->{counter});


    my @notes = (42, 43, 45, 47, 48, 45, 47, 45, 47, 43, 45, 43);
#     my @notes = (42, 43, 45);
    $self->{counter}++ if ($bangs % 2 == 0);

    my $note = $notes[$self->{counter}%@notes]; # Use int for array index
#     my $note = $notes[int($floats[0]*@notes)]; # Use int for array index
    my @vels = (1, 0.2, 0.5);

    my $vel = $floats[1]*127*$vels[$self->{counter} % @vels];
    my $dur = $floats[$#floats]*0.01;
    $self->{message} = ($bangs % 2 == 0)
            ? "NoteOn 0 1 $note $vel\r\n"
            : "NoteOff 0 1 $note 67\r\n";
    return;
}

1;
