use strict;
use warnings;

our $LATENCY = 0.05;

# # --- Core Math ---
# sub f_mod {
#     my ($x, $y) = @_;
#     my $f = $x / $y;
#     return ($f - int($f)) * $y;
# }

# # --- 1. CLOCK SCHEDULER (Imperative) ---
sub sched {
    my ($clock, $name, $code) = @_;
    $clock->{shreds} //= {};
    my $s = $clock->{shreds}->{$name} //= { next => $clock->{accumulated_beats} };

    while ($s->{next} <= $clock->{accumulated_beats}) {
        my $dur = $code->($s);
        $dur //= 1.0; # Default 1 beat
        $s->{next} += $dur;
    }
}

# --- 2. SC PATTERN (Declarative) ---
sub pattern {
    my ($clock, $name, $durs, $code) = @_;
    $clock->{patterns} //= {};
    my $p = $clock->{patterns}->{$name} //= { next => $clock->{accumulated_beats}, idx => 0 };

    while ($p->{next} <= $clock->{accumulated_beats}) {
        $code->($p);
        my $dur = $durs->[$p->{idx} % scalar(@$durs)];
        $p->{next} += $dur;
        $p->{idx}++;
    }
}

# --- 3. PHASE CROSS (Spatial) ---
sub loop {
    my ($clock, $point, $div, $code) = @_;
    my $old = $clock->{previous_beats} // $clock->{accumulated_beats};
    my $new = $clock->{accumulated_beats};

    # Check if we crossed an integer multiple of $point
    if (int($old * $div / $point) < int($new * $div / $point)) {
        $code->();
    }
}

# --- Queue Logic with Pattern Matching ---
sub process_queue {
    my ($self) = @_;
    $self->{queue} //= [];

    my @future;

    foreach my $item (@{$self->{queue}}) {
        my ($target_beat, $skini) = ($item->{beat_time}, $item->{message});
        my $now = $item->{clock}->{accumulated_beats};
        my $bps = $item->{clock}->{tempo} / 60.0;

        if ($now >= $target_beat) {
            # 1. Calculate the fresh delta in seconds
            my $delta = (($target_beat - $now) / $bps);
            $delta = 0 if $delta < 0;

            # 2. Regex to substitute the time field
            # SKINI format: MessageType Time Channel Data...
            # This regex captures the first word, then replaces the second word (the time).
            $skini =~ s/^(\S+)\s+\S+/$1 . sprintf(" %.6f", $delta)/e;

            $self->{message} .= $skini . "\r\n";
        } else {
            push @future, $item;
        }
    }
    $self->{queue} = \@future;
}

sub play {
    my ($self, $clock, $note, $dur_beats) = @_;
    my $target_beat = $clock->{accumulated_beats};
    # 1. Immediate NoteOn
    my $bps = $clock->{tempo} / 60.0;
    my $on_delta = (($target_beat - $clock->{accumulated_beats}) / $bps);
    $self->{message} .= sprintf("NoteOn %.6f 1 %d 100\r\n", $on_delta, $note);

    # 2. Store NoteOff in queue
    my $off_beat = $target_beat + $dur_beats;

    my $skini_template = "NoteOff 0.000000 1 $note 0";
    push @{$self->{queue}}, {
        message => "$skini_template",
        clock => $clock,
        beat_time => $off_beat
        };
}

sub once {
    my ($self, $key, $code) = @_;
    $self->{once_registry} //= {};

    if (!$self->{once_registry}->{$key}) {
        $code->();
        $self->{once_registry}->{$key} = 1;
    }
}

sub once_reset {
    my ($self, $key) = @_;
    if ($self->{once_registry}->{$key}){
        $self->{once_registry}->{$key} = 0;
    }
}

1;