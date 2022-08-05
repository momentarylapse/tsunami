/*
 * AudioSucker.h
 *
 *  Created on: 17.09.2017
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOSUCKER_H_
#define SRC_MODULE_AUDIO_AUDIOSUCKER_H_

#include "../Module.h"
#include "../ModuleConfiguration.h"

class Port;

class AudioSucker : public Module {
public:
	AudioSucker(Session *session);

	int do_suck(int buffer_size);

	Port *source;

	int command(ModuleCommand cmd, int param) override;

	void set_channels(int channels);


	class Config : public ModuleConfiguration {
	public:
		int channels;
		void reset() override;
		string auto_conf(const string &name) const override;
	} config;

	ModuleConfiguration* get_config() const override;
};

#endif /* SRC_MODULE_AUDIO_AUDIOSUCKER_H_ */