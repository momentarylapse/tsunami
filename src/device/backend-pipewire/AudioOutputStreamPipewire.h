//
// Created by michi on 18.05.24.
//

#ifndef TSUNAMI_AUDIOOUTPUTSTREAMPIPEWIRE_H
#define TSUNAMI_AUDIOOUTPUTSTREAMPIPEWIRE_H

#if HAS_LIB_PIPEWIRE

#include "../interface/AudioOutputStream.h"

struct pw_stream;

namespace tsunami {

class AudioOutputStreamPipewire : public AudioOutputStream {
public:
	AudioOutputStreamPipewire(Session *session, Device *device, SharedData& shared_data);
	~AudioOutputStreamPipewire() override;

	void pause() override;
	void unpause() override;
	void pre_buffer() override;
	void flush() override;
	base::optional<int64> estimate_samples_played() override;

	pw_stream *stream;
};

}

#endif

#endif //TSUNAMI_AUDIOOUTPUTSTREAMPIPEWIRE_H
