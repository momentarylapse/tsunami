/*
 * AudioRecorder.h
 *
 *  Created on: 07.03.2019
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIORECORDER_H_
#define SRC_MODULE_AUDIO_AUDIORECORDER_H_


#include "../../Data/Audio/AudioBuffer.h"
#include "../Module.h"
#include "../Port/Port.h"

class AudioRecorder : public Module
{
public:
	AudioRecorder();

	class Output : public Port
	{
	public:
		Output(AudioRecorder *j);
		int read_audio(AudioBuffer &buf) override;
		AudioRecorder *rec;
	};

	void accumulate(bool enable);

	void reset_state() override;
	void command(ModuleCommand cmd) override;

	Port *source;
	AudioBuffer buf;
	bool accumulating;
};

#endif /* SRC_MODULE_AUDIO_AUDIORECORDER_H_ */
