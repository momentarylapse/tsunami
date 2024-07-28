//
// Created by michi on 18.05.24.
//

#if HAS_LIB_PIPEWIRE

#include "DeviceContextPipewire.h"
#include "AudioInputStreamPipewire.h"
#include "AudioOutputStreamPipewire.h"
#include "../Device.h"
#include "../../lib/os/msg.h"
#include "../../lib/hui/hui.h"

#include <pipewire/pipewire.h>

namespace tsunami {

DeviceContextPipewire* DeviceContextPipewire::instance = nullptr;


DeviceContextPipewire::DeviceContextPipewire(Session* session) : DeviceContext(session) {
	instance = this;
}

DeviceContextPipewire::~DeviceContextPipewire() {
	lock();
	pw_proxy_destroy((struct pw_proxy*)registry);
	pw_core_disconnect(core);
	pw_context_destroy(context);
	unlock();
	pw_thread_loop_destroy(loop);
}


static struct spa_hook registry_listener;

static void registry_event_global(void *data, uint32_t id,
                                  uint32_t permissions, const char *type, uint32_t version,
                                  const struct spa_dict *props)
{
	auto ctx = reinterpret_cast<DeviceContextPipewire*>(data);
	bool is_sink = false;
	for (int i=0; i<props->n_items; i++) {
		if (strcmp(props->items[i].key, "media.class") == 0 and strcmp(props->items[i].value, "Audio/Sink") == 0)
			is_sink = true;
	}
	if (is_sink) {
	//	printf("object: id:%u type:%s/%d\n", id, type, version);
		Device d;
		d.type = DeviceType::AudioOutput;
		d.index_in_lib = (int)id;
		d.channels = 2;
		d.present = true;
		string desc, nick;
		for (int i=0; i<props->n_items; i++) {
			if (strcmp(props->items[i].key, PW_KEY_NODE_NAME) == 0)
				d.internal_name = props->items[i].value;
			if (strcmp(props->items[i].key, PW_KEY_NODE_NICK) == 0)
				nick = props->items[i].value;
			if (strcmp(props->items[i].key, PW_KEY_NODE_DESCRIPTION) == 0)
				desc = props->items[i].value;
			if (strcmp(props->items[i].key, PW_KEY_OBJECT_SERIAL) == 0)
				d.index_in_lib = string(props->items[i].value)._int();
	//		printf("   %s   %s\n", props->items[i].key, props->items[i].value);
		}
		d.name = nick;
		if (nick.num == 0)
			d.name = desc;

	//	msg_write(d.index_in_lib);
		hui::run_in_gui_thread([ctx, d] {
			ctx->out_device_found(d);
		});
	}
}

static const struct pw_registry_events registry_events = {
	.version = PW_VERSION_REGISTRY_EVENTS,
	.global = registry_event_global,
};



bool DeviceContextPipewire::init(Session* session) {
	pw_init(nullptr, nullptr);


	loop = pw_thread_loop_new(nullptr, nullptr /* properties */);
	context = pw_context_new(pw_thread_loop_get_loop(loop),
	                         nullptr /* properties */,
	                         0 /* user_data size */);

	core = pw_context_connect(context,
	                          nullptr /* properties */,
	                          0 /* user_data size */);

	registry = pw_core_get_registry(core, PW_VERSION_REGISTRY,
	                                0 /* user_data size */);

	spa_zero(registry_listener);
	pw_registry_add_listener(registry, &registry_listener,
	                         &registry_events, this);

	{
		// default output device
		Device d;
		d.type = DeviceType::AudioOutput;
		d.channels = 2;
		d.default_by_lib = true;
		d.present = true;
		out_device_found(d);
	}

	pw_thread_loop_start(loop);

	return true;
}

void DeviceContextPipewire::lock() {
	pw_thread_loop_lock(loop);
}

void DeviceContextPipewire::unlock() {
	pw_thread_loop_unlock(loop);
}

void DeviceContextPipewire::update_device(DeviceManager* device_manager, bool serious) {
}

AudioOutputStream* DeviceContextPipewire::create_audio_output_stream(Session *session, Device *device, void* shared_data) {
	return new AudioOutputStreamPipewire(session, device, *reinterpret_cast<AudioOutputStream::SharedData*>(shared_data));
}

AudioInputStream* DeviceContextPipewire::create_audio_input_stream(Session *session, Device *device, void* shared_data) {
	return new AudioInputStreamPipewire(session, device, *reinterpret_cast<AudioInputStream::SharedData*>(shared_data));
}

}

#endif
