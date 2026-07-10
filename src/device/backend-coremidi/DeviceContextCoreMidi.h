#ifndef TSUNAMI_DEVICECONTEXTCOREMIDI_H
#define TSUNAMI_DEVICECONTEXTCOREMIDI_H

#if HAS_LIB_COREMIDI

#include "../interface/DeviceContext.h"

namespace tsunami {

class DeviceContextCoreMidi : public DeviceContext {
public:
	explicit DeviceContextCoreMidi(DeviceManager* device_manager);
	~DeviceContextCoreMidi() override;

	bool init() override;
	void update_device(bool serious) override;

	MidiInputStream* create_midi_input_stream(Session *session, Device *device, void* shared_data) override;

	int client = 0;

	static DeviceContextCoreMidi* instance;
};

}

#endif

#endif //TSUNAMI_DEVICECONTEXTCOREMIDI_H
