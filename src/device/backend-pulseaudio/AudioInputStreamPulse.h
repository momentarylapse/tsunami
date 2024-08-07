//
// Created by michi on 5/11/24.
//

#ifndef TSUNAMI_AUDIOINPUTSTREAMPULSE_H
#define TSUNAMI_AUDIOINPUTSTREAMPULSE_H

#if HAS_LIB_PULSEAUDIO

#include "../interface/AudioInputStream.h"

struct pa_stream;

namespace tsunami {

class AudioInputStreamPulse : public AudioInputStream {
public:
	AudioInputStreamPulse(Session *session, Device *device, SharedData& shared_data);
	~AudioInputStreamPulse() override;

	void pause() override;
	void unpause() override;
	base::optional<int64> estimate_samples_captured() override;

	pa_stream *pulse_stream;

	bool _pulse_test_error(const char *msg);
	static void pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata);
	static void pulse_input_notify_callback(pa_stream *p, void *userdata);
	static void pulse_stream_success_callback(pa_stream *s, int success, void *userdata);
	static void pulse_stream_state_callback(pa_stream *s, void *userdata);
};

}

#endif

#endif //TSUNAMI_AUDIOINPUTSTREAMPULSE_H
