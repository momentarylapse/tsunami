//
// Created by michi on 18.05.24.
//

#if HAS_LIB_ALSA

#include "MidiInputStreamAlsa.h"
#include "DeviceContextAlsa.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../data/midi/MidiData.h"
#include "../../Session.h"
#include <alsa/asoundlib.h>

namespace tsunami {

MidiInputStreamAlsa::MidiInputStreamAlsa(Session *session, Device *device, MidiInputStream::SharedData &shared_data) : MidiInputStream(session, shared_data) {

	shared_data.portid = snd_seq_create_simple_port(DeviceContextAlsa::instance->alsa_midi_handle, "Tsunami MIDI in",
	                                    SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
	                                    SND_SEQ_PORT_TYPE_APPLICATION);
	if (shared_data.portid < 0) {
		session->e(string("Error creating sequencer port: ") + snd_strerror(shared_data.portid));
		error = true;
	}
}

MidiInputStreamAlsa::~MidiInputStreamAlsa() {
	snd_seq_delete_simple_port(DeviceContextAlsa::instance->alsa_midi_handle, shared_data.portid);
}

bool MidiInputStreamAlsa::start() {
	if (!DeviceContextAlsa::instance->alsa_midi_handle){
		session->e(_("no alsa midi handler"));
		return false;
	}
	return true;
}

bool MidiInputStreamAlsa::stop() {
	return true;
}

bool MidiInputStreamAlsa::update_device(Device* device) {
	if ((device->client < 0) or (device->port < 0))
		return true;

	if (!DeviceContextAlsa::instance->alsa_midi_handle)
		return false;

	snd_seq_addr_t sender, dest;
	sender.client = device->client;
	sender.port = device->port;
	dest.client = snd_seq_client_id(DeviceContextAlsa::instance->alsa_midi_handle);
	dest.port = shared_data.portid;

	snd_seq_port_subscribe_malloc(&subs);
	snd_seq_port_subscribe_set_sender(subs, &sender);
	snd_seq_port_subscribe_set_dest(subs, &dest);
	int r = snd_seq_subscribe_port(DeviceContextAlsa::instance->alsa_midi_handle, subs);
	if (r != 0) {
		session->e(string("Error connecting to midi port: ") + snd_strerror(r));
		snd_seq_port_subscribe_free(subs);
		subs = nullptr;
	}
	return true;// r == 0;

	// simple version raises "no permission" error...?!?
	/*int r = snd_seq_connect_to(handle, portid, p.client, p.port);
	if (r != 0)
		tsunami->log->Error(string("Error connecting to midi port: ") + snd_strerror(r));
	return r == 0;*/
}

bool MidiInputStreamAlsa::unconnect() {
	if (!subs)
		return true;
	int r = snd_seq_unsubscribe_port(DeviceContextAlsa::instance->alsa_midi_handle, subs);
	if (r != 0)
		session->e(_("Error unconnecting from midi port: ") + snd_strerror(r));
	snd_seq_port_subscribe_free(subs);
	subs = nullptr;
	return r == 0;
}

void MidiInputStreamAlsa::clear_input_queue() {
	while (true) {
		snd_seq_event_t *ev;
		int r = snd_seq_event_input(DeviceContextAlsa::instance->alsa_midi_handle, &ev);
		if (r < 0)
			break;
		snd_seq_free_event(ev);
	}
}


void MidiInputStreamAlsa::read(MidiEventBuffer& buffer) {
	while (true){
		snd_seq_event_t *ev;
		int r = snd_seq_event_input(DeviceContextAlsa::instance->alsa_midi_handle, &ev);
		if (r < 0)
			break;
		float pitch = (float)ev->data.note.note;
		switch (ev->type) {
			case SND_SEQ_EVENT_NOTEON:
				buffer.add(MidiEvent(0, pitch, (float)ev->data.note.velocity / 127.0f));
				break;
			case SND_SEQ_EVENT_NOTEOFF:
				buffer.add(MidiEvent(0, pitch, 0));
				break;
		}
		snd_seq_free_event(ev);
	}
}

}

#endif
