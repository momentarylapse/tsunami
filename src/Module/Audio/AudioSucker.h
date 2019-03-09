/*
 * AudioSucker.h
 *
 *  Created on: 17.09.2017
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOSUCKER_H_
#define SRC_MODULE_AUDIO_AUDIOSUCKER_H_

#include "../Module.h"

class Port;

class AudioSucker : public Module
{
public:
	AudioSucker();
	~AudioSucker() override;

	int do_suck(int buffer_size);

	Port *source;

	int command(ModuleCommand cmd, int param) override;
};

#endif /* SRC_MODULE_AUDIO_AUDIOSUCKER_H_ */
