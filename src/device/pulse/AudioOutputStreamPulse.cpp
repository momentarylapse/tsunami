//
// Created by michi on 5/10/24.
//


#if HAS_LIB_PULSEAUDIO

#include "AudioOutputStreamPulse.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../Session.h"
#include <pulse/pulseaudio.h>

static const bool STREAM_WARNINGS = true;

extern void pulse_wait_op(Session*, pa_operation*); // -> DeviceManager.cpp
//extern void pulse_ignore_op(Session*, pa_operation*);


// DeviceManager needs to be locked!
bool pulse_wait_stream_ready(pa_stream *s, DeviceManager *dm) {
	//msg_write("wait stream ready");
	int n = 0;
	while (pa_stream_get_state(s) != PA_STREAM_READY) {
		//printf(".\n");
		//pa_mainloop_iterate(m, 1, NULL);
		//hui::Sleep(0.01f);
		pa_threaded_mainloop_wait(dm->pulse_mainloop);
		n ++;
		if (n >= 1000)
			return false;
		if (pa_stream_get_state(s) == PA_STREAM_FAILED)
			return false;
	}
	//msg_write("ok");
	return true;
}

static int nnn = 0;
static int xxx_total_read = 0;


AudioOutputStreamPulse::AudioOutputStreamPulse(Session *session, Device *device) : AudioOutputStream(session) {

}

AudioOutputStreamPulse::~AudioOutputStreamPulse() {

}

void AudioOutputStreamPulse::pause() {

}
void AudioOutputStreamPulse::unpause() {

}
void AudioOutputStreamPulse::flush() {

}
base::optional<int64> AudioOutputStreamPulse::estimate_samples_played() {
	return base::None;
}

#endif