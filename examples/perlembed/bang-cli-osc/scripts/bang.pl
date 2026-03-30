use strict;
use warnings;

use lib ".";
require './helpers.pl';

my %melodies = (
    apokoronas_a   => [-5, -4, -2, 0, 2, 3],
    apokoronas_b   => [2, 3, 5, 7, 5, 3, 2, 0],
    kolybarianos_a => [-3, -2, -2, 2, 0, 2, -2, 0, -3, -3, -3, -5, -3],
    kolybarianos_b => [0, 0, 0, -2, -2, -4, -5, -7, -5, -4, -5, -2, -2, -4, -5],
    lousaakiano_a  => [-1, 0, 2, -1, -1, 0, -1, -4, -5, -5],
    lousaakiano_b  => [2, 0, 2, 0, -1, -4, -5, -7, -5, -4, -5, -1, 0, -4, -5],
    kalerianos_a   => [4, 2, -1, 0, 2, 0, -1, -3, -5],
    kalerianos_b   => [-1, -1, 0, -1, -3, -5, -5, -3, -6, -8],
);


sub _bang_event {
    my ($self) = @_;
    my $v = $self->{influx} // [0,0,0,0,0,0,0,0];
    my $c = $self->{clocks}->{"main"};

    my @available_keys = sort keys %melodies;

    # 1. Higher Level: Select Two Segments based on influx[0] and [1]
    my $num_melodies = scalar @available_keys;
    my $num_pairs = int($num_melodies / 2);

    # 2. Pick a pair index (0 to $num_pairs - 1) based on influx[0]
    # my $pair_idx = int(norm_v($v->[0]) * ($num_pairs - 1));
    my $pair_idx = 1; #0 - 3
    
    # 3. Force k1 to be the even start of the pair, and k2 to be the next one
    my $idx1 = $pair_idx * 2;
    my $idx2 = $idx1 + 1;

    my $k1 = $available_keys[$idx1];
    my $k2 = $available_keys[$idx2];
    
    my $unit1 = $melodies{$k1};
    my $unit2 = $melodies{$k2};

    # 4. Higher Level: Select Shakespeare Schema based on influx[2]
    # -1..-0.5: Chiasmus (1221), -0.5..0.5: Linear (12), 0.5..1: Entangled (1212)
    my $schema = [1, 1, 2, 2];
    if ($v->[2] > 0.5)    { $schema = [1, 2, 1, 2]; }
    elsif ($v->[2] < -0.5) { $schema = [1, 2, 2, 1]; }

    # 5. Structural Assembly
    my $raw_notes = apply_structural_rhetoric([$unit1, $unit2], $schema);
    
    # 6. Symbolize & Regex Mutation (Probabilistic Anadiplosis based on influx[3])
    my $text = symbolize($raw_notes);
    $text = apply_rhetoric_schema($text, $v);
    my $final_notes = desymbolize($text);


# --- VOICE 1: HIGH (The Ornament) ---
    sched($c, "voice_high", sub {
        my ($s) = @_;
        my $note_val = $final_notes->[($s->{idx}++) % scalar(@$final_notes)];
        my $rh = get_ornamental_rhetoric($note_val, 1.0, $v);
        play($self, $c, 75 + $rh->{note}, $rh->{duration}, $rh->{velocity}*0.6);
        return $rh->{interval}; 
    });

    # --- VOICE 2: MID (The Relator) ---
    sched($c, "voice_mid", sub {
        my ($s) = @_;
        # Offset the index so it's not playing the exact same note as the high voice
        my $note_val = $final_notes->[($s->{idx}++ + 2) % scalar(@$final_notes)];
        
        # Macro-slot of 2.0 beats (half the speed of high, double the speed of bass)
        my $rh = get_mid_rhetoric($note_val, 2.0, $v);
        
        # Transpose to the middle register
        play($self, $c, 63 + $rh->{note}, $rh->{duration}, $rh->{velocity}* 0.4);
        return $rh->{interval};
    });

    # --- VOICE 3: BASS (The Foundation) ---
    sched($c, "voice_bass", sub {
        my ($s) = @_;
        my $note_val = $final_notes->[($s->{idx}++) % scalar(@$final_notes)];
        my $rh = get_low_rhetoric($note_val, 4.0, $v);
        play($self, $c, 51 + $rh->{note}, $rh->{duration}, $rh->{velocity}*0.3);
        return $rh->{interval};
    });
}


sub tick {
    my ($self, $msg) = @_;
    $self->{last_unix_time} //= $self->{current_unix_time};
    $self->{message} = "";

    if (ref($msg) eq 'ARRAY') {
        $self->{influx} = $msg;
    } else {
        # This case should not happen if C++ is always sending an array ref
#         print STDERR "Perl received a non-array message: '$msg'\n";
    }

    clock($self, "main", 104);

    _bang_event($self);
    process_queue($self);

    $self->{last_unix_time} = $self->{current_unix_time};
}

1;