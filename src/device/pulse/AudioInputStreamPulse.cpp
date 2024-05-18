//
// Created by michi on 5/11/24.
//

#if HAS_LIB_PULSEAUDIO

#include "AudioInputStreamPulse.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../Session.h"
#include <stdio.h>

#include <pulse/pulseaudio.h>

extern void pulse_wait_op(Session *session, pa_operation *op); // -> DeviceManager.cpp
extern bool pulse_wait_stream_ready(pa_stream *s, DeviceManager *dm); // -> OutputStream.cpp


AudioInputStreamPulse::AudioInputStreamPulse(Session *session, Device *device, SharedData& shared_data) : AudioInputStream(session, shared_data) {

	pa_sample_spec ss;
	ss.rate = session->sample_rate();
	ss.channels = shared_data.num_channels;
	msg_write(shared_data.num_channels);
	ss.format = PA_SAMPLE_FLOAT32NE;
	pulse_stream = pa_stream_new(session->device_manager->pulse_context, "stream-in", &ss, nullptr);
	if (!pulse_stream)
		_pulse_test_error("pa_stream_new");


	pa_stream_set_read_callback(pulse_stream, &pulse_stream_request_callback, this);
	pa_stream_set_state_callback(pulse_stream, &pulse_stream_state_callback, this);

	pa_buffer_attr attr_in;
	//	attr_in.fragsize = -1;
	attr_in.fragsize = shared_data.chunk_size;
	attr_in.maxlength = -1;
	attr_in.minreq = -1;
	attr_in.tlength = -1;
	attr_in.prebuf = -1;
	const char *dev = nullptr;
	if (!device->is_default())
		dev = device->internal_name.c_str();
	auto flags = (pa_stream_flags_t)(PA_STREAM_ADJUST_LATENCY|PA_STREAM_AUTO_TIMING_UPDATE|PA_STREAM_INTERPOLATE_TIMING);
	// without PA_STREAM_ADJUST_LATENCY, we will get big chunks (split into many small ones, but still "clustered")
	if (pa_stream_connect_record(pulse_stream, dev, &attr_in, flags) != 0)
		_pulse_test_error("pa_stream_connect_record");

	if (!pulse_wait_stream_ready(pulse_stream, device_manager)) {
		device_manager->unlock();
		session->e("pulse_wait_stream_ready");
		return;
	}
}

AudioInputStreamPulse::~AudioInputStreamPulse() {
	pa_stream_set_state_callback(pulse_stream, nullptr, nullptr);
	pa_stream_set_read_callback(pulse_stream, nullptr, nullptr);

	if (pa_stream_disconnect(pulse_stream) != 0)
		_pulse_test_error("pa_stream_disconnect");

/*		// FIXME really necessary?!?!?!?
		for (int i=0; i<1000; i++) {
			if (pa_stream_get_state(pulse_stream) == PA_STREAM_TERMINATED) {
				break;
			}
			hui::Sleep(0.001f);
		}*/

	pa_stream_unref(pulse_stream);
	//_pulse_test_error("pa_stream_unref");
}

void AudioInputStreamPulse::pause() {
	pa_operation *op = pa_stream_cork(pulse_stream, true, &pulse_stream_success_callback, this);
	if (!op)
		_pulse_test_error("pa_stream_cork");
	pulse_wait_op(session, op);
}

void AudioInputStreamPulse::unpause() {
	if (!pulse_stream)
		return;
	pa_operation *op = pa_stream_cork(pulse_stream, false, &pulse_stream_success_callback, this);
	if (!op)
		_pulse_test_error("pa_stream_cork");
	pulse_wait_op(session, op);
}

base::optional<int64> AudioInputStreamPulse::estimate_samples_captured() {
	pa_usec_t t;
	if (pa_stream_get_time(pulse_stream, &t) == 0) {
		double usec2samples = session->sample_rate() / 1000000.0;
		return (int64)((double)t * usec2samples);
	}
	return base::None;
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

	int frames = nbytes / sizeof(float) / input->shared_data.num_channels;

	if (data) {
		float *in = (float*)data;

		RingBuffer &buf = input->shared_data.buffer;
		AudioBuffer b;
		buf.write_ref(b, frames);
		b.deinterleave(in, input->shared_data.num_channels);
		buf.write_ref_done(b);

		int done = b.length;
		if (done < frames) {
			buf.write_ref(b, frames - done);
			b.deinterleave(&in[input->shared_data.num_channels * done], input->shared_data.num_channels);
			buf.write_ref_done(b);
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

bool AudioInputStreamPulse::_pulse_test_error(const char *msg) {
	int e = pa_context_errno(session->device_manager->pulse_context);
	if (e != 0)
		session->e(format("%s (input): %s", msg, pa_strerror(e)));
	return (e != 0);
}

#endif
