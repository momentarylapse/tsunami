//
// Created by michi on 18.05.24.
//

#if HAS_LIB_PULSEAUDIO

#include "DeviceContextPulse.h"
#include "AudioInputStreamPulse.h"
#include "AudioOutputStreamPulse.h"
#include "../Device.h"
#include "../DeviceManager.h"
#include "../../Session.h"
#include "../../lib/hui/hui.h"

#include <pulse/pulseaudio.h>

namespace tsunami {

DeviceContextPulse* DeviceContextPulse::instance = nullptr;


// inside lock() ... unlock()
void DeviceContextPulse::wait_op(Session *session, pa_operation *op) {
	if (!op)
		return;
	//printf("-w-\n");
	int n = 0;
	while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
		//printf(".\n");
		//pa_mainloop_iterate(m, 1, NULL);
		n ++;
		if (n > 3000)
			break;
		//hui::Sleep(0.010f);
		pa_threaded_mainloop_wait(DeviceContextPulse::instance->pulse_mainloop);
	}
	auto status = pa_operation_get_state(op);
	//printf("%d\n", status);
	if (status != PA_OPERATION_DONE) {
		if (status == PA_OPERATION_RUNNING)
			session->e("pulse_wait_op() failed: still running");
		else if (status == PA_OPERATION_CANCELLED)
			session->e("pulse_wait_op() failed: cancelled");
		else
			session->e("pulse_wait_op() failed: ???");
	}
	pa_operation_unref(op);
	//printf("-o-\n");
}

void DeviceContextPulse::ignore_op(Session *session, pa_operation *op) {
	if (!op) {
		session->e("pulse_ignore_op:  op=nil");
		return;
	}
	pa_operation_unref(op);
}

void pulse_subscription_callback(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata) {
	auto *ctx = static_cast<DeviceContextPulse*>(userdata);
	//msg_write(format("event  %d  %d", (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK), (t & PA_SUBSCRIPTION_EVENT_TYPE_MASK)));

	if (((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_NEW) or ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)) {
		//printf("----change   %d\n", idx);

		hui::run_in_gui_thread([ctx]{
			ctx->out_request_update();
		});
	}
	pa_threaded_mainloop_signal(ctx->pulse_mainloop, 0);
}


DeviceContextPulse::DeviceContextPulse(Session* session) : DeviceContext(session) {
	instance = this;
}

DeviceContextPulse::~DeviceContextPulse() {

	if (pulse_mainloop)
		pa_threaded_mainloop_stop(pulse_mainloop);
	//_test_error(session, "pa_threaded_mainloop_stop");

	if (pulse_context)
		pa_context_disconnect(pulse_context);
	//_test_error(session, "pa_context_disconnect");

	if (pulse_context)
		pa_context_unref(pulse_context);
	//_test_error(session, "pa_context_unref"); // would require a context...

	if (pulse_mainloop)
		pa_threaded_mainloop_free(pulse_mainloop);
	//_test_error(session, "pa_threaded_mainloop_free");
}

bool DeviceContextPulse::init(Session* session) {
	pulse_mainloop = pa_threaded_mainloop_new();
	if (!pulse_mainloop) {
		session->e("pa_threaded_mainloop_new failed");
		return false;
	}

	pa_mainloop_api *mainloop_api = pa_threaded_mainloop_get_api(pulse_mainloop);
	if (!mainloop_api) {
		session->e("pa_threaded_mainloop_get_api failed");
		return false;
	}

	pulse_context = pa_context_new(mainloop_api, "tsunami");
	if (_test_error(session, "pa_context_new"))
		return false;

	pa_context_set_state_callback(pulse_context, &DeviceContextPulse::state_callback, this);

	lock();

	pa_threaded_mainloop_start(pulse_mainloop);
	if (_test_error(session, "pa_threaded_mainloop_start")) {
		unlock();
		return false;
	}

	pa_context_connect(pulse_context, nullptr, PA_CONTEXT_NOAUTOSPAWN, nullptr);
	if (_test_error(session, "pa_context_connect")) {
		unlock();
		return false;
	}

	if (!wait_context_ready()) {
		session->e("pulse audio context does not turn 'ready'");
		unlock();
		return false;
	}

	pa_context_set_subscribe_callback(pulse_context, &pulse_subscription_callback, this);
	_test_error(session, "pa_context_set_subscribe_callback");
	pa_context_subscribe(pulse_context, (pa_subscription_mask_t)(PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE), nullptr, this);
	_test_error(session, "pa_context_subscribe");

	unlock();
	return true;
}


void DeviceContextPulse::lock() {
	//printf("-lock...\n");
	if (pulse_mainloop)
		pa_threaded_mainloop_lock(pulse_mainloop);
	//printf("...ok-\n");
}

void DeviceContextPulse::unlock() {
	//printf("-unlock...\n");
	if (pulse_mainloop)
		pa_threaded_mainloop_unlock(pulse_mainloop);
	//printf("...ok-\n");
}


void DeviceContextPulse::update_device(DeviceManager* device_manager, bool serious) {
	if (!fully_initialized)
		return;
	Session* session = device_manager->session;

	for (Device *d: device_manager->output_devices)
		d->present = false;
	for (Device *d: device_manager->input_devices)
		d->present = false;

	// system default
	auto *def = device_manager->get_device_create(DeviceType::AudioOutput, "");
	def->channels = 2;
	def->default_by_lib = true;
	def->present = true;

	lock();

	pa_operation *op = pa_context_get_sink_info_list(pulse_context, &DeviceContextPulse::sink_info_callback, this);
	if (!op)
		_test_error(session, "pa_context_get_sink_info_list");
	wait_op(session, op);

	// system default
	def = device_manager->get_device_create(DeviceType::AudioInput, "");
	def->channels = 2;
	def->default_by_lib = true;
	def->present = true;

	op = pa_context_get_source_info_list(pulse_context, &DeviceContextPulse::source_info_callback, this);
	if (!op)
		_test_error(session, "pa_context_get_source_info_list");
	wait_op(session, op);

	unlock();
}

AudioOutputStream* DeviceContextPulse::create_audio_output_stream(Session *session, Device *device, void* shared_data) {
	return new AudioOutputStreamPulse(session, device, *reinterpret_cast<AudioOutputStream::SharedData*>(shared_data));
}

AudioInputStream* DeviceContextPulse::create_audio_input_stream(Session *session, Device *device, void* shared_data) {
	return new AudioInputStreamPulse(session, device, *reinterpret_cast<AudioInputStream::SharedData*>(shared_data));
}

bool DeviceContextPulse::wait_context_ready() {
	//msg_write("wait stream ready");
	int n = 0;
	while (pa_context_get_state(pulse_context) != PA_CONTEXT_READY) {
		//pa_mainloop_iterate(m, 1, NULL);
		//hui::Sleep(0.01f);
		pa_threaded_mainloop_wait(pulse_mainloop);
		n ++;
		if (n >= 500)
			return false;
		if (pa_context_get_state(pulse_context) == PA_CONTEXT_FAILED)
			return false;
	}
	//msg_write("ok");
	return true;
}

// DeviceManager needs to be locked!
bool DeviceContextPulse::wait_stream_ready(pa_stream *s) {
	//msg_write("wait stream ready");
	int n = 0;
	while (pa_stream_get_state(s) != PA_STREAM_READY) {
		//printf(".\n");
		//pa_mainloop_iterate(m, 1, NULL);
		//hui::Sleep(0.01f);
		pa_threaded_mainloop_wait(pulse_mainloop);
		n ++;
		if (n >= 1000)
			return false;
		if (pa_stream_get_state(s) == PA_STREAM_FAILED)
			return false;
	}
	//msg_write("ok");
	return true;
}

void DeviceContextPulse::sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
	auto *ctx = static_cast<DeviceContextPulse*>(userdata);
	if (eol > 0 or !i or !userdata) {
		pa_threaded_mainloop_signal(ctx->pulse_mainloop, 0);
		return;
	}
	//printf("output  %s ||  %s   %d   %d\n", i->name, i->description, i->index, i->channel_map.channels);

	Device d;
	d.type = DeviceType::AudioOutput;
	d.internal_name = i->name;
	d.index_in_lib = (int)i->index;
	d.name = i->description;
	d.channels = i->channel_map.channels;
	d.present = true;
	ctx->out_device_found(d);
	pa_threaded_mainloop_signal(ctx->pulse_mainloop, 0);
}

void DeviceContextPulse::source_info_callback(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
	auto *ctx = static_cast<DeviceContextPulse*>(userdata);
	if (eol > 0 or !i or !userdata) {
		pa_threaded_mainloop_signal(ctx->pulse_mainloop, 0);
		return;
	}
	//printf("input  %s ||  %s   %d   %d\n", i->name, i->description, i->index, i->channel_map.channels);

	Device d;
	d.type = DeviceType::AudioInput;
	d.internal_name = i->name;
	d.index_in_lib = (int)i->index;
	d.name = i->description;
	d.channels = i->channel_map.channels;
	d.present = true;
	ctx->out_device_found(d);
	pa_threaded_mainloop_signal(ctx->pulse_mainloop, 0);
}

void DeviceContextPulse::state_callback(pa_context* context, void* userdata) {
	auto *ctx = static_cast<DeviceContextPulse*>(userdata);
	pa_threaded_mainloop_signal(ctx->pulse_mainloop, 0);
}

bool DeviceContextPulse::_test_error(Session *session, const string &msg) {
	int e = pa_context_errno(pulse_context);
	if (e != 0)
		session->e(msg + ": " + pa_strerror(e));
	return (e != 0);
}

}

#endif
