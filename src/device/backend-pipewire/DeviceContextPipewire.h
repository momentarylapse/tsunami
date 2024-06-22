//
// Created by michi on 18.05.24.
//

#ifndef TSUNAMI_DEVICECONTEXTPIPEWIRE_H
#define TSUNAMI_DEVICECONTEXTPIPEWIRE_H

#if HAS_LIB_PIPEWIRE

#include "../interface/DeviceContext.h"

struct pw_thread_loop;
struct pw_context;
struct pw_core;
struct pw_registry;

namespace tsunami {

class DeviceContextPipewire : public DeviceContext {
public:
	DeviceContextPipewire(Session* session);
	~DeviceContextPipewire();

	bool init(Session* session) override;
	void update_device(DeviceManager* device_manager, bool serious) override;
	AudioOutputStream* create_audio_output_stream(Session *session, Device *device, void* shared_data) override;
	AudioInputStream* create_audio_input_stream(Session *session, Device *device, void* shared_data) override;

	void lock();
	void unlock();

	pw_thread_loop* loop = nullptr;
	pw_context* context = nullptr;
	pw_core* core = nullptr;
	pw_registry* registry = nullptr;

	static DeviceContextPipewire* instance;
};

}

#endif

#endif //TSUNAMI_DEVICECONTEXTPIPEWIRE_H
