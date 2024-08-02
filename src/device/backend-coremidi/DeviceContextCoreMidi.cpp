#if HAS_LIB_COREMIDI

#include <CoreServices/CoreServices.h>
#include <CoreMidi/CoreMidi.h>

#include "DeviceContextCoreMidi.h"
#include "MidiInputStreamCoreMidi.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../Session.h"
#include "../../lib/os/msg.h"


namespace tsunami {

DeviceContextCoreMidi* DeviceContextCoreMidi::instance;

void on_coremidi_notify(const MIDINotification *message, void* refCon) {
	auto ctx = reinterpret_cast<DeviceContextCoreMidi*>(refCon);
	//msg_write(format("notify   %d", (int)message->messageID));
	if (message->messageID == kMIDIMsgObjectAdded or message->messageID == kMIDIMsgObjectRemoved)
		ctx->out_request_update();
}

DeviceContextCoreMidi::DeviceContextCoreMidi(Session* session) : DeviceContext(session) {
	instance = this;

	CFStringRef name = CFStringCreateWithCString(kCFAllocatorDefault, "tsunami", kCFStringEncodingMacRoman);
	MIDIClientCreate(name, &on_coremidi_notify, this, (MIDIClientRef*)&client);
	//MIDIClientCreateWithBlock(name, )
	CFRelease(name);
}

DeviceContextCoreMidi::~DeviceContextCoreMidi() = default;

bool DeviceContextCoreMidi::init(Session* session) {
	return true;
}


void DeviceContextCoreMidi::update_device(DeviceManager* device_manager, bool serious) {
	if (!fully_initialized)
		return;

	int n = MIDIGetNumberOfSources();
	for (int i=0; i<n; i++) {
		auto d = MIDIGetSource(i);
		CFStringRef s = nullptr;
		MIDIObjectGetStringProperty(d, kMIDIPropertyName, &s);
		string name = CFStringGetCStringPtr(s, kCFStringEncodingUTF8);
		CFRelease(s);

		s = nullptr;
		MIDIObjectGetStringProperty(d, kMIDIPropertyDisplayName, &s);
		string display_name = CFStringGetCStringPtr(s, kCFStringEncodingUTF8);
		CFRelease(s);

		int value;
		MIDIObjectGetIntegerProperty(d, kMIDIPropertyUniqueID, &value);
		//msg_write(value);

		Device dev;
		dev.type = DeviceType::MidiInput;
		dev.internal_name = name;
		dev.name = display_name;
		dev.index_in_lib = (int)d;
		dev.present = true;
		out_device_found(dev);
	}

	// default
	auto *def = device_manager->get_device_create(DeviceType::MidiInput, "");
	def->default_by_lib = true;
	def->present = true;
	device_manager->set_device_config(def);
}

MidiInputStream* DeviceContextCoreMidi::create_midi_input_stream(Session *session, Device *device, void* shared_data) {
	return new MidiInputStreamCoreMidi(session, device, *reinterpret_cast<MidiInputStream::SharedData*>(shared_data));
}

}

#endif
