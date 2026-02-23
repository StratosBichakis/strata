use strict;
use warnings;

# push @INC, ".";
# require "helpers.pl"; # Loads the file once
use lib '.';
use Strata;

sub bang {
    my ($self, $message_from_c) = @_;

    my $num_bangs = $self->{num_bangs};
    my $tempo = $self->{tempo};
    my $current_unix_time = $self->{current_unix_time};

    my @floats;
    if (ref($message_from_c) eq 'ARRAY') {
        @floats = @{$message_from_c};
#         print STDERR "Perl received an array of " . scalar(@floats) . " floats.\n";
    } else {
#         print STDERR "Perl received a string message: '$message_from_c'\n";
    }
    $self->{counter} = 0 unless (defined $self->{counter});

    # Acceleration logic
    unless (defined $self->{is_accelerating}) {
        $self->{is_accelerating} = 0;
        $self->{start_acceleration_time} = 0;
    }

    if ($num_bangs == 1 && !$self->{is_accelerating}) {
        $self->{start_acceleration_time} = $current_unix_time;
        $self->{is_accelerating} = 1;
#         print STDERR "Acceleration started at tempo $self->{tempo} at time $current_unix_time\n";
    }

    if ($self->{is_accelerating}) {
        my $elapsed_time = $current_unix_time - $self->{start_acceleration_time};
        my $acceleration_duration = 30.0; # seconds
        my $start_tempo = 150.0;
        my $end_tempo = 120.0;

        if ($elapsed_time < $acceleration_duration) {
            my $elapsed_time_ratio = $elapsed_time / $acceleration_duration;
            my $new_tempo = $start_tempo + ($end_tempo - $start_tempo) * $elapsed_time_ratio;
            $self->{tempo} = $new_tempo;
        } else {
            $self->{tempo} = $end_tempo;
        }
    }

#     my @notes = (42, 43, 45, 47, 48, 45, 47, 45, 47, 43, 45, 43);
    my @notes = (42, 43, 45, 47);
    $self->{counter}++ if ($num_bangs % 2 == 0);

    my $note = $notes[$floats[2]*@notes];
    my @vels = (1, 0.2, 0.7, 0.6);
    my $vel = $floats[0]*127*$vels[$self->{counter} % @vels];
    my $dur = $floats[$#floats]*0.04;
    $self->{message} = ($num_bangs % 2 == 0)
            ? "NoteOn 0 1 $note $vel\r\nNoteOff $dur 1 $note 114\r\n":"";
#             : "NoteOff 0 1 60 114\r\n";
    
    return;
}
1;
