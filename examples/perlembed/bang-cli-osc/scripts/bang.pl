use strict;
use warnings;

use lib ".";
require './helpers.pl';

sub _bang_event {
    my ($self) = @_;
    my $data = $self->{osc_data} // [0,0,0,0,0,0,0,0]; # Fallback

    my $c_1 = $self->{clocks}->{"main"};
    my $c_2 = $self->{clocks}->{"sec"};

    pattern($c_1, "simple", [0.5, 0.25, 0.25, 0.444], sub {
        play($self, $c_1, 45, 0.1);
    });

    sched($c_1, "lo", sub {
        play($self, $c_1, 65, 0.3);
        return 0.4432;
    });

    # Use Sensor 4 to trigger a one-shot event when it crosses a threshold
        if ($data->[4] > 0.8) {
            once($self, "sensor_threshold", sub {
                play($self, $c_1, 80, 1.0);
            });
        }
        if ($data->[4] < 0.5) {
            # Reset the 'once' so it can trigger again when the sensor drops/rises
            once_reset($self, "sensor_threshold");
        }

#     once($self, "do_", sub {
#         play($self, $c_1, 45, 0.1);
#     });

    loop($c_2, 12, 7.7, sub {
        play($self, $c_2, 53, 0.5);
    });
}

sub clock {
    my ($self, $name, $tempo) = @_;
    $self->{clocks} //= {};

    my $c = $self->{clocks}->{$name} //= { accumulated_beats => 0, tempo => $tempo // 120};
    $c->{tempo} = $tempo // $c->{tempo};

    my $delta_sec = $self->{current_unix_time} - $self->{last_unix_time};
    $c->{previous_beats} = $c->{accumulated_beats};
    $c->{accumulated_beats} += $delta_sec * ($c->{tempo} / 60);
}

sub tick {
    my ($self, $msg) = @_;
    $self->{last_unix_time} //= $self->{current_unix_time};
    $self->{message} = "";
#     do "./helpers.pl";

    if (ref($msg) eq 'ARRAY') {
        $self->{osc_data} = $msg;
    } else {
        # This case should not happen if C++ is always sending an array ref
#         print STDERR "Perl received a non-array message: '$msg'\n";
    }

    # Example: Map first 2 sensors to clock tempos
    my $tempo_1 = 120 + ($self->{osc_data}->[0] * 40); # Range 120-160
    my $tempo_2 = 110 + ($self->{osc_data}->[1] * 30); # Range 110-140

    clock($self, "main", 136);
    clock($self, "sec", 125);

    _bang_event($self);
    process_queue($self);

    $self->{last_unix_time} = $self->{current_unix_time};
}

1;