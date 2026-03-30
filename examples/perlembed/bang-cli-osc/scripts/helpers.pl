use strict;
use warnings;

our $LATENCY = 0.05;

# # --- Core Math ---
# sub f_mod {
#     my ($x, $y) = @_;
#     my $f = $x / $y;
#     return ($f - int($f)) * $y;
# }

# Symbolizer/Desymbolizer remains the same
sub symbolize { join('', map { chr($_ + 100) } @{$_[0]}) }
sub desymbolize { [ map { ord($_) - 100 } split('', $_[0]) ] }

# Scales influx -1..1 to a 0..1 range
sub norm_v { return ($_[0] + 1) / 2; }

# Calculates the total "Energy" of the system
sub get_total_influx {
    my ($v) = @_;
    my $sum = 0;
    foreach my $val (@$v) {
        $sum += abs($val);
    }
    return $sum; # Range will be roughly 0 to 8.0
}

sub apply_rhetoric_schema {
    my ($str, $v) = @_;

    # --- 1. PROBABILISTIC ANADIPLOSIS (Original v[0] logic) ---
    if ($v->[0] > 0.1) {
        $str =~ s/(.)$/ (rand(1) < norm_v($v->[0])) ? "$1$1" : "$1" /e;
    }

    # --- 2. GREEDY REPETITION (Original v[1] logic) ---
    if ($v->[1] > 0.4) {
        my $repeats = int(norm_v($v->[1]) * 8) + 1;
        $str =~ s/(..)$/ $1 x $repeats /e;
    }

    # --- 3. NEW: STRUCTURAL MOTIF MIRRORING (Complex v[3] logic) ---
    # Instead of just doubling the end, we capture a 3-note motif 
    # and transform it into a mirrored "Shakespearean" structure (12321)
    if ($v->[3] > 0.2) {
        # Capture 3 chars, replace with 1-2-3-2-1 sequence
        # Probability based on the intensity of v[3]
        $str =~ s/(.)(.)(.)$/ (rand(1) < norm_v($v->[3])) ? "$1$2$3$2$1" : "$1$2$3" /e;
    }

    return $str;
}

sub get_low_rhetoric {
    my ($note, $total_beat_width, $v) = @_;

    # 1. Lower Density Subdivision
    # We use v[6] instead of v[4]. Range is 1 to 4 subdivisions (slower pulses)
    my $density_val = norm_v($v->[6]); 
    my $count = int($density_val * 3) + 1;
    my $sub_interval = $total_beat_width / $count;

    # 2. Heavier Articulation
    # We use v[7] for a "Drone" vs "Pizzicato" feel (0.3 to 1.2 for overlap)
    my $articulation_val = norm_v($v->[7]);
    my $articulation = 0.3 + ($articulation_val * 0.9);
    my $sub_duration = $sub_interval * $articulation;

    # 3. Independent Volume (v[6] intensity + global energy)
    my $energy = get_total_influx($v);
    my $low_voice_vol = 30 + (abs($v->[6]) * 40) + ($energy * 5);
    $low_voice_vol = 100 if $low_voice_vol > 100;

    return {
        interval => $sub_interval,
        duration => $sub_duration,
        velocity => $low_voice_vol,
        note     => $note
    };
}

sub get_mid_rhetoric {
    my ($note, $total_beat_width, $v) = @_;

    # 1. Moderate Density (v[4] + v[6] average)
    # This makes the mid voice a "mediator" between the fast and slow sensors
    my $density_val = (norm_v($v->[4]) + norm_v($v->[6])) / 2;
    my $count = int($density_val * 5) + 1; # 1 to 6 subdivisions
    my $sub_interval = $total_beat_width / $count;

    # 2. Balanced Articulation
    my $articulation = 0.4 + (norm_v($v->[5]) * 0.4); 
    my $sub_duration = $sub_interval * $articulation;

    # 3. Mid-range Volume
    my $energy = get_total_influx($v);
    my $mid_vol = 45 + ($energy * 8);
    $mid_vol = 105 if $mid_vol > 105;

    return {
        interval => $sub_interval,
        duration => $sub_duration,
        velocity => $mid_vol,
        note     => $note
    };
}

sub get_ornamental_rhetoric {
    my ($note, $total_beat_width, $v) = @_;

    # 1. Calculate Total Energy/Entropy for Volume
    my $total_influx = get_total_influx($v);
    
    # Map the sum (0..8) to a MIDI velocity range (e.g., 30..110)
    my $global_volume = 40 + ($total_influx * 15);
    $global_volume = 100 if $global_volume > 100;
    $global_volume = 30 if $global_volume < 30; # Noise floor
    
    # Use influx[4] for Subdivision density (1 to 12)
    my $density_val = norm_v($v->[4]);
    my $count = int($density_val * 8) + 1;
    
    # Use influx[5] for Legato/Staccato (Duration vs Interval)
    # 0.1 = very staccato, 1.0 = overlapping/legato
    my $articulation = 0.1 + (norm_v($v->[5]) * 0.9);
    
    # Use influx[6] for Velocity (Dynamics)
    my $velocity = 10 + (norm_v($v->[7]) * $global_volume);
    # my $velocity =  $global_volume);
    
    my $sub_interval = $total_beat_width / $count;
    my $sub_duration = $sub_interval * $articulation;
    
    return {
        count    => $count,
        interval => $sub_interval,
        duration => $sub_duration,
        velocity => $velocity,
        note     => $note
    };
}

# Converts a Shakespeare Number (e.g., 1212) into a sequence of segments
sub apply_structural_rhetoric {
    my ($segments, $schema) = @_;
    my @combined;
    foreach my $id (@$schema) {
        my $idx = $id - 1; # 1-indexed to 0-indexed
        push @combined, @{$segments->[$idx]} if $segments->[$idx];
    }
    return \@combined;
}

# Advanced Greedy Division: 
# Instead of just repeating a note, it returns a list of (note, duration, velocity)
# so that the total duration of the "figure" remains musically consistent.
sub greedy_divide {
    my ($note, $total_dur, $influx_val) = @_;
    
    # Scale -1..1 to 1..8 repetitions
    my $count = int(abs($influx_val) * 7) + 1;
    my $sub_dur = $total_dur / $count;
    
    # Dynamic Velocity: higher influx = stronger accent
    my $base_vel = 40 + (abs($influx_val) * 60);
    
    my @events;
    for (1..$count) {
        # Add a tiny bit of "human" velocity variation
        my $v = $base_vel + rand(10);
        push @events, { note => $note, dur => $sub_dur, vel => $v };
    }
    return @events;
}


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

sub clock {
    my ($self, $name, $tempo) = @_;
    $self->{clocks} //= {};

    my $c = $self->{clocks}->{$name} //= { accumulated_beats => 0, tempo => $tempo // 120};
    $c->{tempo} = $tempo // $c->{tempo};

    my $delta_sec = $self->{current_unix_time} - $self->{last_unix_time};
    $c->{previous_beats} = $c->{accumulated_beats};
    $c->{accumulated_beats} += $delta_sec * ($c->{tempo} / 60);
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
    my ($self, $clock, $note, $dur_beats, $vel) = @_;
    my $target_beat = $clock->{accumulated_beats};
    # 1. Immediate NoteOn
    my $bps = $clock->{tempo} / 60.0;
    my $on_delta = (($target_beat - $clock->{accumulated_beats}) / $bps);
    $self->{message} .= sprintf("NoteOn %.6f 1 %.3f %.2f\r\n", $on_delta, $note, $vel);

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