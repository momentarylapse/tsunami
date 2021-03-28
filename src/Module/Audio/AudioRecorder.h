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
#include "../ModuleConfiguration.h"
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
	void set_channels(int channels);

	void reset_state() override;
	int command(ModuleCommand cmd, int param) override;
	void on_config() override;

	Port *source;
	AudioBuffer buf;
	std::mutex mtx_buf;

	int64 samples_skipped = 0;


	class Config : public ModuleConfiguration {
	public:
		int channels;
		bool accumulating;
		void reset() override;
		string auto_conf(const string &name) const override;
	} config;

	ModuleConfiguration* get_config() const override;
};

#endif /* SRC_MODULE_AUDIO_AUDIORECORDER_H_ */
