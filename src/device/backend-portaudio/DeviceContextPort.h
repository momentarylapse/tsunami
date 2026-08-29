//
// Created by michi on 18.05.24.
//

#ifndef TSUNAMI_DEVICECONTEXTPORT_H
#define TSUNAMI_DEVICECONTEXTPORT_H

#if HAS_LIB_PORTAUDIO

#include "../interface/DeviceContext.h"

namespace obs {
	class LogSource;
}

typedef int PaError;

namespace tsunami {

class DeviceContextPort : public DeviceContext {
public:
	explicit DeviceContextPort(DeviceManager* device_manager);
	~DeviceContextPort() override;

	bool init() override;
	void update_device_list(bool serious) override;
	AudioOutputStream* create_audio_output_stream(Session *session, Device *device, void* shared_data) override;
	AudioInputStream* create_audio_input_stream(Session *session, Device *device, void* shared_data) override;

	bool _test_error(PaError err, obs::LogSource* log_source, const string &msg);

	static DeviceContextPort* instance;
};

}

#endif

#endif //TSUNAMI_DEVICECONTEXTPORT_H
