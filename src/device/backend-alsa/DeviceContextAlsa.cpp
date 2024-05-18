//
// Created by michi on 18.05.24.
//

#if HAS_LIB_ALSA

#include "DeviceContextAlsa.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../Session.h"

#include <alsa/asoundlib.h>

DeviceContextAlsa* DeviceContextAlsa::instance;

DeviceContextAlsa::DeviceContextAlsa(Session* session) : DeviceContext(session) {
	instance = this;
}

DeviceContextAlsa::~DeviceContextAlsa() {
	if (alsa_midi_handle)
		snd_seq_close(alsa_midi_handle);
}

bool DeviceContextAlsa::init() {
	int r = snd_seq_open(&alsa_midi_handle, "hw", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
	if (r < 0) {
		session->e(string("Error opening ALSA sequencer: ") + snd_strerror(r));
		return false;
	}

	snd_seq_set_client_name(alsa_midi_handle, "Tsunami");
	return true;
}

void DeviceContextAlsa::lock() {

}

void DeviceContextAlsa::unlock() {

}


void DeviceContextAlsa::update_device(DeviceManager* device_manager, bool serious) {
	if (!alsa_midi_handle)
		return;

	for (Device *d: device_manager->midi_input_devices) {
		d->present_old = d->present;
		d->present = false;
	}

	// default
	auto *def = device_manager->get_device_create(DeviceType::MIDI_INPUT, "");
	def->default_by_lib = true;
	def->present = true;

	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;

	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_client_info_set_client(cinfo, -1);
	while (snd_seq_query_next_client(alsa_midi_handle, cinfo) >= 0) {
		snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
		snd_seq_port_info_set_port(pinfo, -1);
		while (snd_seq_query_next_port(alsa_midi_handle, pinfo) >= 0) {
			if ((snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_READ) == 0)
				continue;
			if ((snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_SUBS_READ) == 0)
				continue;
			Device *d = device_manager->get_device_create(DeviceType::MIDI_INPUT, format("%s/%s", snd_seq_client_info_get_name(cinfo), snd_seq_port_info_get_name(pinfo)));
			d->name = d->internal_name;
			d->client = snd_seq_client_info_get_client(cinfo);
			d->port = snd_seq_port_info_get_port(pinfo);
			d->present = true;
		}
	}


	bool changed = false;
	for (Device *d: device_manager->midi_input_devices)
		if (d->present_old != d->present)
			changed = true;
	if (changed)
		device_manager->out_changed.notify();

}

#endif
