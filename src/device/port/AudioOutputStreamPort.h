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
	static int portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
												 unsigned long frames,
												 const PaStreamCallbackTimeInfo* timeInfo,
												 PaStreamCallbackFlags statusFlags,
												 void *userData);
};

#endif

#endif //TSUNAMI_AUDIOOUTPUTSTREAMPORT_H
