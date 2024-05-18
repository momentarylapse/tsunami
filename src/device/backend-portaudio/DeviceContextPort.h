//
// Created by michi on 18.05.24.
//

#ifndef TSUNAMI_DEVICECONTEXTPORT_H
#define TSUNAMI_DEVICECONTEXTPORT_H

#if HAS_LIB_PORTAUDIO

#include "../interface/DeviceContext.h"

typedef int PaError;

class DeviceContextPort : public DeviceContext {
public:
	DeviceContextPort(Session* session);
	~DeviceContextPort();

	bool init() override;
	void lock() override;
	void unlock() override;
	void update_device(DeviceManager* device_manager, bool serious) override;

	bool _test_error(PaError err, Session *session, const string &msg);

	static DeviceContextPort* instance;
};

#endif

#endif //TSUNAMI_DEVICECONTEXTPORT_H
