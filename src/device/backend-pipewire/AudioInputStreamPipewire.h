//
// Created by michi on 6/7/24.
//

#ifndef TSUNAMI_AUDIOINPUTSTREAMPIPEWIRE_H
#define TSUNAMI_AUDIOINPUTSTREAMPIPEWIRE_H

#if HAS_LIB_PIPEWIRE

#include "../interface/AudioInputStream.h"

struct pw_stream;

namespace tsunami {

class AudioInputStreamPipewire : public AudioInputStream {
public:
	AudioInputStreamPipewire(Session *session, Device *device, SharedData &shared_data);
	~AudioInputStreamPipewire() override;

	void pause() override;
	void unpause() override;

	base::optional<int64> estimate_samples_captured() override;
	pw_stream *stream;
};

}

#endif

#endif //TSUNAMI_AUDIOINPUTSTREAMPIPEWIRE_H
