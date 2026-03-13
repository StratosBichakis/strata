use strict;
use warnings;

# New tick function, called every 1ms from C++
sub tick {
    my ($self, $message_from_c) = @_;

    # Initialize state variables within the persistent hash
    $self->{last_tick_time} = $self->{current_unix_time} unless defined $self->{last_tick_time}; # Use C++ time for first init
    $self->{bangs} = 0 unless defined $self->{bangs};
    $self->{tempo} = 120.0 unless defined $self->{tempo}; # Default tempo
    $self->{time_since_last_bang_check} = 0.0 unless defined $self->{time_since_last_bang_check};
    $self->{last_tempo_change} = $self->{current_unix_time} unless defined $self->{last_tempo_change}; # Initialize with current time
    $self->{_previous_tempo} = $self->{tempo} unless defined $self->{_previous_tempo}; # Initialize to current tempo

    my $current_c_time = $self->{current_unix_time}; # Time from C++

    # Calculate time difference since last tick (in seconds)
    my $time_delta = $current_c_time - $self->{last_tick_time};
    $self->{last_tick_time} = $current_c_time; # Update for next tick

    # Check for tempo change and update last_tempo_change
#     if ($self->{tempo} != $self->{_previous_tempo}) {
#         $self->{last_tempo_change} = $current_c_time;
#         $self->{_previous_tempo} = $self->{tempo};
#         # Optionally reset bangs and time_since_last_bang_check on tempo change
# #         $self->{bangs} = 0;
#         $self->{time_since_last_bang_check} = 0.0;
#     }

    # Define the interval at which a 'bang' should occur based on tempo
    # Original C++ calculation: double interval = 60.0 / (details.tempo*8.0);
    my $bang_interval_seconds = 60.0 / ($self->{tempo}*6) ;

    # Accumulate time
    $self->{time_since_last_bang_check} += $time_delta;

    # Check if enough time has passed to trigger a 'bang'
    if ($self->{time_since_last_bang_check} >= $bang_interval_seconds) {
        $self->{bangs}++; # Increment bang counter
#         print STDERR "Perl Bang! bangs: " . $self->{bangs} . "\n";
        _bang($self, $message_from_c); # Call the event logic
        # Subtract the interval to handle potential overshoot and keep timing consistent
        $self->{time_since_last_bang_check} -= $bang_interval_seconds;
    }

    # Any other 1ms-based logic can go here
    return;
}

1; # Crucial: require files must return true