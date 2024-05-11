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


AudioOutputStreamPulse::AudioOutputStreamPulse(Session *session, Device *device, SharedData& shared_data) : AudioOutputStream(session, shared_data) {

	pa_sample_spec ss;
	ss.rate = session->sample_rate();
	ss.channels = 2;
	ss.format = PA_SAMPLE_FLOAT32NE;
	//ss.format = PA_SAMPLE_S16NE;
	pulse_stream = pa_stream_new(device_manager->pulse_context, "stream", &ss, nullptr);
	if (!pulse_stream)
		_pulse_test_error("pa_stream_new");

	pa_stream_set_state_callback(pulse_stream, &pulse_stream_state_callback, this);
	pa_stream_set_write_callback(pulse_stream, &pulse_stream_request_callback, this);
	pa_stream_set_underflow_callback(pulse_stream, &pulse_stream_underflow_callback, this);

	pa_buffer_attr attr_out;
	attr_out.fragsize = -1; // recording only
	attr_out.maxlength = hui::config.get_int("Output.Pulseaudio.maxlength", -1);
	attr_out.minreq = hui::config.get_int("Output.Pulseaudio.minreq", 1024);
	attr_out.tlength = hui::config.get_int("Output.Pulseaudio.tlength", 1024);
	attr_out.prebuf = hui::config.get_int("Output.Pulseaudio.prebuf", 0); // don't prebuffer
	// prebuf = 0 also prevents pausing during buffer underruns

	const char *dev = nullptr;
	if (!device->is_default())
		dev = device->internal_name.c_str();
	auto flags = (pa_stream_flags_t)(PA_STREAM_START_CORKED|PA_STREAM_AUTO_TIMING_UPDATE|PA_STREAM_INTERPOLATE_TIMING);

	if (pa_stream_connect_playback(pulse_stream, dev, &attr_out, flags, nullptr, nullptr) != 0)
		_pulse_test_error("pa_stream_connect_playback");


	if (!pulse_wait_stream_ready(pulse_stream, device_manager)) {
		session->w("retry");

		// retry with default device
		if (pa_stream_connect_playback(pulse_stream, nullptr, &attr_out, flags, nullptr, nullptr) != 0)
			_pulse_test_error("pa_stream_connect_playback");

		if (!pulse_wait_stream_ready(pulse_stream, device_manager)) {
			device_manager->unlock();
			// still no luck... give up
			session->e("pulse_wait_stream_ready");
//				pa_threaded_mainloop_unlock(device_manager->pulse_mainloop);
			error = true;
			return;
		}
	}
}

AudioOutputStreamPulse::~AudioOutputStreamPulse() {
	_pulse_flush_op();
	device_manager->lock();
	pa_stream_set_state_callback(pulse_stream, nullptr, nullptr);
	pa_stream_set_write_callback(pulse_stream, nullptr, nullptr);
	pa_stream_set_underflow_callback(pulse_stream, nullptr, nullptr);

	if (pa_stream_disconnect(pulse_stream) != 0)
		_pulse_test_error("pa_stream_disconnect");

	pa_stream_unref(pulse_stream);
	//_pulse_test_error("pa_stream_unref");
	pulse_stream = nullptr;
	device_manager->unlock();
}

void AudioOutputStreamPulse::pause() {
	device_manager->lock();
	auto op = pa_stream_cork(pulse_stream, true, &pulse_stream_success_callback, this);
	device_manager->unlock();
	_pulse_start_op(op, "pa_stream_cork");

}
void AudioOutputStreamPulse::unpause() {
	device_manager->lock();
	pa_operation *op = pa_stream_cork(pulse_stream, false, &pulse_stream_success_callback, this);
	device_manager->unlock();
	_pulse_start_op(op, "pa_stream_cork");
	_pulse_flush_op();

}

void AudioOutputStreamPulse::pre_buffer() {
	/*device_manager->lock();
	pa_operation *op = pa_stream_prebuf(pulse_stream, &pulse_stream_success_callback, this);
	device_manager->unlock();
	_pulse_start_op(op, "pa_stream_prebuf");*/

	/*pa_operation *op = pa_stream_trigger(pulse_stream, &pulse_stream_success_callback, nullptr);
	_pulse_test_error("pa_stream_trigger");
	pa_wait_op(session, op);*/
}

int64 AudioOutputStreamPulse::flush(int64 samples_offset_since_reset, int64 samples_requested) {
	device_manager->lock();
	pa_operation *op = pa_stream_flush(pulse_stream, &pulse_stream_success_callback, this);
	device_manager->unlock();
	_pulse_start_op(op, "pa_stream_flush");
	_pulse_flush_op();

	auto get_write_offset = [this, samples_offset_since_reset, samples_requested] () {
		if (auto info = pa_stream_get_timing_info(pulse_stream))
			if (info->read_index_corrupt == 0)
				return info->write_index / 8 - samples_offset_since_reset;
		return samples_requested;
	};
	return get_write_offset();

}
base::optional<int64> AudioOutputStreamPulse::estimate_samples_played(int64 samples_offset_since_reset, int64 samples_requested) {
	// PA_STREAM_INTERPOLATE_TIMING
	if (auto info = pa_stream_get_timing_info(pulse_stream)) {
		auto dbuffer = (info->write_index - info->read_index) / 8;
		if (false)
			printf("%6ld  %6ld  %6ld | w=%-6lld  r=%-6lld  d=%-6ld | req=%-8lld\n", info->configured_sink_usec, info->sink_usec, info->transport_usec, info->write_index/8-samples_offset_since_reset, info->read_index/8-samples_offset_since_reset, dbuffer, samples_requested);
		double samples_per_usec = session->sample_rate() / 1000000.0;
		double delay_samples = (double)(info->sink_usec + info->transport_usec) * samples_per_usec;
		if (info->read_index_corrupt == 0)
			return max(info->write_index / 8 - samples_offset_since_reset - (int64)delay_samples, (int64)0);
		//printf("    %d %d\n", info->write_index, info->read_index);
	}
	/*pa_usec_t t;
	if (pa_stream_get_time(pulse_stream, &t) == 0) {
		double usec2samples = session->sample_rate() / 1000000.0;
		printf("%lld  %.3f\n", fake_samples_played, t/1000000.0);
		return (double)t * usec2samples - fake_samples_played;
	}*/
	return samples_requested;
}
// NOT inside lock()/unlock()
void AudioOutputStreamPulse::_pulse_flush_op() {
	if (operation) {
		device_manager->lock();
		pulse_wait_op(session, operation);
		device_manager->unlock();
	}
	operation = nullptr;
}

// NOT inside lock()/unlock()
void AudioOutputStreamPulse::_pulse_start_op(pa_operation *op, const char *msg) {
	if (!op) {
		_pulse_test_error(msg);
		return;
	}
	_pulse_flush_op();
	operation = op;
	//pulse_wait_op(session, op);
}

bool AudioOutputStreamPulse::_pulse_test_error(const char *msg) {
	int e = pa_context_errno(device_manager->pulse_context);
	if (e != 0) {
		session->e(format("AudioOutput: %s: %s", msg, pa_strerror(e)));
		return true;
	}
	return false;
}

void AudioOutputStreamPulse::pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata) {
	//printf("output request %d\n", (int)nbytes/8);
	auto stream = static_cast<AudioOutputStreamPulse*>(userdata);

	if (nbytes == 0)
		return;

	void *data = nullptr;
	int r = pa_stream_begin_write(p, &data, &nbytes);
	if (r != 0) {
		stream->_pulse_test_error("pa_stream_begin_write");
		return;
	}
	//printf("%d  %p  %d\n", r, data, (int)nbytes);

	int frames = nbytes / 8;
	float *out = static_cast<float*>(data);

	bool out_of_data = stream->shared_data.callback_feed(out, frames);

//	bool out_of_data = stream->feed_stream_output(frames, out);

	if (pa_stream_write(p, data, nbytes, nullptr, 0, (pa_seek_mode_t)PA_SEEK_RELATIVE) != 0)
		stream->_pulse_test_error("pa_stream_write");

	if (out_of_data)
		stream->signal_out_of_data();
	//pa_threaded_mainloop_signal(stream->device_manager->pulse_mainloop, 0);*/
}

void AudioOutputStreamPulse::pulse_stream_success_callback(pa_stream *s, int success, void *userdata) {
	auto stream = static_cast<AudioOutputStreamPulse*>(userdata);
	//msg_write("--success");
	pa_threaded_mainloop_signal(stream->device_manager->pulse_mainloop, 0);
}

void AudioOutputStreamPulse::pulse_stream_state_callback(pa_stream *s, void *userdata) {
	auto stream = static_cast<AudioOutputStreamPulse*>(userdata);
	//printf("--state\n");
	pa_threaded_mainloop_signal(stream->device_manager->pulse_mainloop, 0);
}

void AudioOutputStreamPulse::pulse_stream_underflow_callback(pa_stream *s, void *userdata) {
	//auto stream = static_cast<AudioOutput*>(userdata);
	//stream->session->w("pulse: underflow\n");
	if (STREAM_WARNINGS)
		printf("pulse: underflow\n");
	//pa_threaded_mainloop_signal(stream->device_manager->pulse_mainloop, 0);
}

#endif