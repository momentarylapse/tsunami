//
// Created by michi on 5/10/24.
//

#ifndef TSUNAMI_AUDIOOUTPUTSTREAMPORT_H
#define TSUNAMI_AUDIOOUTPUTSTREAMPORT_H

#if HAS_LIB_PORTAUDIO

#include "../interface/AudioOutputStream.h"


typedef void PaStream;
struct PaStreamCallbackTimeInfo;
typedef unsigned long PaStreamCallbackFlags;
typedef int PaError;

namespace tsunami {

class AudioOutputStreamPort : public AudioOutputStream {
public:
	AudioOutputStreamPort(Session *session, Device *device, SharedData& shared_data);
	~AudioOutputStreamPort() override;

	void pause() override;
	void unpause() override;
	void flush() override;
	base::optional<int64> estimate_samples_played() override;


	PaStream *portaudio_stream = nullptr;
	bool _portaudio_test_error(PaError err, const char *msg);

	static int portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
												 unsigned long frames,
												 const PaStreamCallbackTimeInfo* timeInfo,
												 PaStreamCallbackFlags statusFlags,
												 void *userData);
};

}

#endif

#endif //TSUNAMI_AUDIOOUTPUTSTREAMPORT_H
