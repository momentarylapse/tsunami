//
// Created by michi on 18.05.24.
//

#ifndef TSUNAMI_DEVICECONTEXTALSA_H
#define TSUNAMI_DEVICECONTEXTALSA_H

#if HAS_LIB_ALSA

#include "../interface/DeviceContext.h"

struct _snd_seq;

namespace tsunami {

class DeviceContextAlsa : public DeviceContext {
public:
	DeviceContextAlsa(Session* session);
	~DeviceContextAlsa();

	bool init(Session* session) override;
	void update_device(DeviceManager* device_manager, bool serious) override;
	MidiInputStream* create_midi_input_stream(Session *session, Device *device, void* shared_data) override;

	_snd_seq *alsa_midi_handle = nullptr;

	static DeviceContextAlsa* instance;
};

}

#endif


#endif //TSUNAMI_DEVICECONTEXTALSA_H
