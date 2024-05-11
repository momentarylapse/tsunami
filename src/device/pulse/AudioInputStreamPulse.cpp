//
// Created by michi on 5/11/24.
//

#if HAS_LIB_PULSEAUDIO

#include "AudioInputStreamPulse.h"
#include "../DeviceManager.h"
#include <stdio.h>

#include <pulse/pulseaudio.h>

extern void pulse_wait_op(Session *session, pa_operation *op); // -> DeviceManager.cpp
extern bool pulse_wait_stream_ready(pa_stream *s, DeviceManager *dm); // -> OutputStream.cpp

AudioInputStreamPulse::SharedData::SharedData() :
		buffer(1048576)
{

}

void AudioInputStreamPulse::pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata) {
	//printf("input request %d\n", (int)nbytes);
	auto input = static_cast<AudioInputStreamPulse*>(userdata);

	const void *data;
	if (pa_stream_peek(p, &data, &nbytes) < 0)
		input->_pulse_test_error("pa_stream_peek");

	// empty buffer
	if (nbytes == 0)
		return;

	int frames = nbytes / sizeof(float) / input->num_channels;

	if (data) {
		if (input->is_capturing()) {
			float *in = (float*)data;

			RingBuffer &buf = input->buffer;
			AudioBuffer b;
			buf.write_ref(b, frames);
			b.deinterleave(in, input->num_channels);
			buf.write_ref_done(b);

			int done = b.length;
			if (done < frames) {
				buf.write_ref(b, frames - done);
				b.deinterleave(&in[input->num_channels * done], input->num_channels);
				buf.write_ref_done(b);
			}
		}

		if (pa_stream_drop(p) != 0)
			input->_pulse_test_error("pa_stream_drop");
	} else if (nbytes > 0) {
		// holes
		if (pa_stream_drop(p) != 0)
			input->_pulse_test_error("pa_stream_drop");
	}
	//msg_write(">");
	//pa_threaded_mainloop_signal(input->dev_man->pulse_mainloop, 0);
}

void AudioInputStreamPulse::pulse_stream_success_callback(pa_stream *s, int success, void *userdata) {
	auto stream = static_cast<AudioInputStreamPulse*>(userdata);
	//msg_write("--success");
	pa_threaded_mainloop_signal(stream->device_manager->pulse_mainloop, 0);
}

void AudioInputStreamPulse::pulse_stream_state_callback(pa_stream *s, void *userdata) {
	auto stream = static_cast<AudioInputStreamPulse*>(userdata);
	//printf("--state\n");
	pa_threaded_mainloop_signal(stream->device_manager->pulse_mainloop, 0);
}

void AudioInputStreamPulse::pulse_input_notify_callback(pa_stream *p, void *userdata) {
	auto stream = static_cast<AudioInputStreamPulse*>(userdata);
	printf("sstate... %p:  ", p);
	int s = pa_stream_get_state(p);
	if (s == PA_STREAM_UNCONNECTED)
		printf("unconnected");
	if (s == PA_STREAM_READY)
		printf("ready");
	if (s == PA_STREAM_TERMINATED)
		printf("terminated");
	printf("\n");
	pa_threaded_mainloop_signal(stream->device_manager->pulse_mainloop, 0);
}

#endif
