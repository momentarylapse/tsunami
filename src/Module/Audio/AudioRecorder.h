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
#include <mutex>

class AudioRecorder : public Module {
public:
	AudioRecorder();

	class Output : public Port {
	public:
		Output(AudioRecorder *j);
		int read_audio(AudioBuffer &buf) override;
		AudioRecorder *rec;
	};

	void _accumulate(bool enable);

	int command(ModuleCommand cmd, int param) override;

	Port *source;
	AudioBuffer buf;
	bool accumulating;
	std::mutex mtx_buf;

	int64 samples_skipped = 0;
};

#endif /* SRC_MODULE_AUDIO_AUDIORECORDER_H_ */
