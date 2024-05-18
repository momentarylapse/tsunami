//
// Created by michi on 18.05.24.
//

#ifndef TSUNAMI_DEVICECONTEXTPULSE_H
#define TSUNAMI_DEVICECONTEXTPULSE_H

#if HAS_LIB_PULSEAUDIO

#include "../interface/DeviceContext.h"

class string;

struct pa_context;
struct pa_threaded_mainloop;
struct pa_sink_info;
struct pa_source_info;
struct pa_operation;
struct pa_stream;

class DeviceContextPulse : public DeviceContext {
public:
	DeviceContextPulse(Session* session);
	~DeviceContextPulse();

	bool init() override;
	void lock() override;
	void unlock() override;
	void update_device(DeviceManager* device_manager, bool serious) override;

	pa_context *pulse_context = nullptr;
	pa_threaded_mainloop *pulse_mainloop = nullptr;
	bool _test_error(Session *session, const string &msg);
	bool wait_context_ready();
	void _update_devices();

	static void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
	static void source_info_callback(pa_context *c, const pa_source_info *i, int eol, void *userdata);
	static void state_callback(pa_context* context, void* userdata);

	static void wait_op(Session *session, pa_operation *op);
	static void ignore_op(Session *session, pa_operation *op);
	bool wait_stream_ready(pa_stream *s);

	static DeviceContextPulse* instance;
};

#endif

#endif //TSUNAMI_DEVICECONTEXTPULSE_H
