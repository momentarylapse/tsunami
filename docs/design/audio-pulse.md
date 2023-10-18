# Use of PulseAudio

## Playback

    pa_buffer_attr.prebuf = 0

* this prevents pausing during buffer underruns

## Behaviour

    pa_stream_new()
    pa_stream_set_state_callback()
    pa_stream_connect_playback()

Now the "write-callback" will be called to request data, even with `PA_STREAM_START_CORKED` and `prebuf = 0`.




