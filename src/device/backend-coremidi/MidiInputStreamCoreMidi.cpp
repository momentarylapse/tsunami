
#if HAS_LIB_COREMIDI
#include <CoreServices/CoreServices.h>
#include <CoreMidi/CoreMidi.h>

#include "MidiInputStreamCoreMidi.h"
#include "DeviceContextCoreMidi.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../data/midi/MidiData.h"
#include "../../Session.h"
#include "../../lib/os/msg.h"

namespace tsunami {

/*string mac_error_str(OSStatus err) {
	auto s = SecCopyErrorMessageString(err, nullptr);
}*/

[[maybe_unused]] static void MyMIDIReceiveBlock(const MIDIEventList *evtlist, void * __nullable srcConnRefCon) {
	msg_write("in");
}

void MyMIDIReadProc(const MIDIPacketList *pktlist, void* readProcRefCon, void* srcConnRefCon) {
	auto stream = reinterpret_cast<MidiInputStreamCoreMidi*>(srcConnRefCon);
	//msg_write(p2s(stream));
	//msg_write("in");
	for (int i=0; i<pktlist->numPackets; i++) {
		auto pk = &pktlist->packet[i];
		const auto data = bytes(pk->data, pk->length);
	//	msg_write(data.hex());
		int cmd = data[0];
		if ((cmd & 0xf0) == 0x80 and data.num >= 3) {
			// note off
			stream->buffer.add(MidiEvent(0, data[1], 0).with_raw(data));
			//msg_write("off " + i2s(data[1]));

		} else if ((cmd & 0xf0) == 0x90 and data.num >= 3) {
			// note on
			stream->buffer.add(MidiEvent(0, data[1], (float)data[2] / 127.0f).with_raw(data));
			//msg_write("on " + i2s(data[1]));
		} else {
			// whatever
			stream->buffer.add(MidiEvent::special(data));
		}
	}
}

MidiInputStreamCoreMidi::MidiInputStreamCoreMidi(Session *session, Device *device, MidiInputStream::SharedData &shared_data) : MidiInputStream(session, shared_data) {
	CFStringRef name = CFStringCreateWithCString(kCFAllocatorDefault, "midi-input", kCFStringEncodingMacRoman);

	// deprecated
	//msg_write(p2s(this));
	auto result = MIDIInputPortCreate((MIDIClientRef)DeviceContextCoreMidi::instance->client, name, (MIDIReadProc)&MyMIDIReadProc, this, (MIDIPortRef*)&port);

	// more modern version?
	//MIDIInputPortCreateWithProtocol((MIDIClientRef)DeviceContextCoreMidi::instance->client, name, kMIDIProtocol_2_0, (MIDIPortRef*)&port, (MIDIReceiveBlock)&MyMIDIReceiveBlock);
	CFRelease(name);

	if (result) {
		session->e(string("Error creating midi port: ") + i2s(result));
		error = true;
	}

	if (device->index_in_lib >= 0) {
		result = MIDIPortConnectSource((MIDIPortRef)port, (MIDIEndpointRef)device->index_in_lib, this);
		if (result) {
			session->e(string("Error connecting to midi source: ") + i2s(result));
			error = true;
		} else {
			endpoit_ref = device->index_in_lib;
		}
	}
}

MidiInputStreamCoreMidi::~MidiInputStreamCoreMidi() {
	MidiInputStreamCoreMidi::unconnect();
	if (port >= 0)
		MIDIPortDispose((MIDIPortRef)port);
}

bool MidiInputStreamCoreMidi::start() {
	return true;
}

bool MidiInputStreamCoreMidi::stop() {
	return true;
}

bool MidiInputStreamCoreMidi::update_device(Device* device) {
	if ((device->client < 0) or (device->port < 0))
		return true;

	return true;
}

bool MidiInputStreamCoreMidi::unconnect() {
	if (endpoit_ref >= 0)
		MIDIPortDisconnectSource((MIDIPortRef)port, (MIDIEndpointRef)endpoit_ref);
	endpoit_ref = -1;
	return true;
}

void MidiInputStreamCoreMidi::clear_input_queue() {
}


void MidiInputStreamCoreMidi::read(MidiEventBuffer& _buffer) {
	if (buffer.num == 0)
		return;
	buffer.samples = _buffer.samples;
	_buffer = buffer;
	buffer.clear();

}

}

#endif