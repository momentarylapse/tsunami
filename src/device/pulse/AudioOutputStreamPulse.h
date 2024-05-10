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
	AudioOutputStreamPulse(Session *session, Device *device);
	~AudioOutputStreamPulse() override;

	void pause() override;
	void unpause() override;
	void flush() override;
	base::optional<int64> estimate_samples_played() override;

};

#endif

#endif //TSUNAMI_AUDIOOUTPUTSTREAMPULSE_H
