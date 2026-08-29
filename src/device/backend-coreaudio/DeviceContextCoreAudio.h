#ifndef TSUNAMI_DEVICECONTEXTCOREAUDIO_H
#define TSUNAMI_DEVICECONTEXTCOREAUDIO_H

#if HAS_LIB_COREAUDIO

#include "../interface/DeviceContext.h"

namespace tsunami {

class DeviceContextCoreAudio : public DeviceContext {
public:
	explicit DeviceContextCoreAudio(DeviceManager* device_manager);
	~DeviceContextCoreAudio() override;

	bool init() override;
	void update_device_list(bool serious) override;
	AudioOutputStream* create_audio_output_stream(Session *session, Device *device, void* shared_data) override;
	AudioInputStream* create_audio_input_stream(Session *session, Device *device, void* shared_data) override;

	static DeviceContextCoreAudio* instance;
};

}

#endif

#endif //TSUNAMI_DEVICECONTEXTCOREAUDIO_H
