//
// Created by michi on 5/10/24.
//

#ifndef TSUNAMI_AUDIOOUTPUTSTREAMPORT_H
#define TSUNAMI_AUDIOOUTPUTSTREAMPORT_H

#if HAS_LIB_PORTAUDIO

#include "../AudioOutputStream.h"


typedef void PaStream;
struct PaStreamCallbackTimeInfo;
typedef unsigned long PaStreamCallbackFlags;
typedef int PaError;

class AudioOutputStreamPort : public AudioOutputStream {
public:
	AudioOutputStreamPort(Session *session, Device *device,
											   std::function<bool(float*,int)> _callback_feed,
											   std::function<void()> _callback_out_of_data);
	~AudioOutputStreamPort() override;

	void pause() override;
	void unpause() override;
	int64 flush(int64 samples_offset_since_reset, int64 samples_requested) override;
	base::optional<int64> estimate_samples_played(int64 samples_offset_since_reset, int64 samples_requested) override;


	PaStream *portaudio_stream = nullptr;
	bool _portaudio_test_error(PaError err, const char *msg);

	static int portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
												 unsigned long frames,
												 const PaStreamCallbackTimeInfo* timeInfo,
												 PaStreamCallbackFlags statusFlags,
												 void *userData);


	std::function<bool(float*,int)> callback_feed;
	std::function<void()> callback_out_of_data;
};

#endif

#endif //TSUNAMI_AUDIOOUTPUTSTREAMPORT_H
