//
// Created by michi on 5/10/24.
//

#ifndef TSUNAMI_AUDIOOUTPUTSTREAMPULSE_H
#define TSUNAMI_AUDIOOUTPUTSTREAMPULSE_H


#if HAS_LIB_PULSEAUDIO

#include "../AudioOutputStream.h"

#include "../../lib/base/base.h"
#include "../../lib/base/optional.h"
#include "../../data/audio/RingBuffer.h"
#include "../../module/Module.h"
#include "../../module/ModuleConfiguration.h"
#include "../../module/port/Port.h"
#include <atomic>

class DeviceManager;
class Device;
class Session;

struct pa_stream;
struct pa_operation;

class AudioOutputStreamPulse : public AudioOutputStream {
public:
	AudioOutputStreamPulse(Session *session, Device *device, std::function<bool(float*,int)> callback_feed, std::function<void()> callback_out_of_data);
	~AudioOutputStreamPulse() override;

	void pause() override;
	void unpause() override;
	int64 flush(int64 samples_offset_since_reset, int64 samples_requested) override;
	base::optional<int64> estimate_samples_played(int64 samples_offset_since_reset, int64 samples_requested) override;


	pa_stream *pulse_stream = nullptr;
	pa_operation *operation = nullptr;
	void _pulse_flush_op();
	void _pulse_start_op(pa_operation *op, const char *msg);
	bool _pulse_test_error(const char *msg);

	static void pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata);
	static void pulse_stream_underflow_callback(pa_stream *s, void *userdata);
	static void pulse_stream_success_callback(pa_stream *s, int success, void *userdata);
	static void pulse_stream_state_callback(pa_stream *s, void *userdata);

	std::function<bool(float*,int)> callback_feed;
	std::function<void()> callback_out_of_data;
};

#endif

#endif //TSUNAMI_AUDIOOUTPUTSTREAMPULSE_H
