//
// Created by michi on 18.05.24.
//

#ifndef TSUNAMI_MIDIINPUTSTREAMALSA_H
#define TSUNAMI_MIDIINPUTSTREAMALSA_H

#if HAS_LIB_ALSA

#include "../interface/MidiInputStream.h"


struct _snd_seq_port_subscribe;

class MidiInputStreamAlsa : public MidiInputStream {
public:
	MidiInputStreamAlsa(Session *session, Device *device, SharedData& shared_data);
	~MidiInputStreamAlsa() override;

	bool start() override;
	bool stop() override;
	bool unconnect() override;
	bool update_device(Device* device) override;
	void clear_input_queue() override;
	void read(MidiEventBuffer& buffer) override;

	_snd_seq_port_subscribe* subs = nullptr;
};

#endif

#endif //TSUNAMI_MIDIINPUTSTREAMALSA_H
