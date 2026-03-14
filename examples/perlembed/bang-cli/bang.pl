use strict;
use warnings;

use lib ".";
require './helpers.pl';

sub _bang_event {
    my ($self) = @_;

    my $c_1 = $self->{clocks}->{"main"};
    my $c_2 = $self->{clocks}->{"sec"};

    pattern($c_1, "simple", [0.5, 0.25, 0.25, 0.444], sub {
        play($self, $c_1, 72, 0.1);
    });

    sched($c_1, "lo", sub {
        play($self, $c_1, 65, 0.3);
        return 0.4432;
    });

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
    clock($self, "main", 136);
    clock($self, "sec", 125);

    _bang_event($self);
    process_queue($self);

    $self->{last_unix_time} = $self->{current_unix_time};
}

1;