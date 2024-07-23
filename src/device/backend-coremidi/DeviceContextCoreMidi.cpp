#if HAS_LIB_COREMIDI

#include <CoreServices/CoreServices.h>
#include <CoreMidi/CoreMidi.h>

#include "DeviceContextCoreMidi.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../Session.h"
#include "../../lib/os/msg.h"


namespace tsunami {

DeviceContextCoreMidi* DeviceContextCoreMidi::instance;

DeviceContextCoreMidi::DeviceContextCoreMidi(Session* session) : DeviceContext(session) {
	instance = this;
}

DeviceContextCoreMidi::~DeviceContextCoreMidi() {
}

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

		Device* dev = device_manager->get_device_create(DeviceType::MIDI_INPUT, name);
		dev->name = display_name;
		dev->index_in_lib = (int)d;
		dev->present = true;
		//out_device_found(dev);
		device_manager->set_device_config(dev);
	}
}

}

#endif
