#ifndef TSUNAMI_DEVICECONTEXTCOREMIDI_H
#define TSUNAMI_DEVICECONTEXTCOREMIDI_H

#if HAS_LIB_COREMIDI

#include "../interface/DeviceContext.h"

namespace tsunami {

class DeviceContextCoreMidi : public DeviceContext {
public:
	DeviceContextCoreMidi(Session* session);
	~DeviceContextCoreMidi();

	bool init(Session* session) override;
	void update_device(DeviceManager* device_manager, bool serious) override;

	static DeviceContextCoreMidi* instance;
};

}

#endif

#endif //TSUNAMI_DEVICECONTEXTCOREMIDI_H
