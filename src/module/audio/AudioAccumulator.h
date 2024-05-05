/*
 * AudioAccumulator.h
 *
 *  Created on: 07.03.2019
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOACCUMULATOR_H_
#define SRC_MODULE_AUDIO_AUDIOACCUMULATOR_H_


#include "../../data/audio/AudioBuffer.h"
#include "../Module.h"
#include "../ModuleConfiguration.h"
#include "../port/Port.h"
#include <mutex>

class AudioAccumulator : public Module {
public:
	AudioAccumulator();

	AudioOutPort out{this};
	AudioInPort in{this};

	int read_audio(int port, AudioBuffer &buf) override;

	void _accumulate(bool enable);
	void set_channels(int channels);

	void reset_state() override;
	base::optional<int64> command(ModuleCommand cmd, int64 param) override;
	void on_config() override;

	AudioBuffer buffer;
	std::mutex mtx_buf;

	int64 samples_skipped = 0;


	class Config : public ModuleConfiguration {
	public:
		int channels;
		bool accumulate;
		void reset() override;
		string auto_conf(const string &name) const override;
	} config;

	ModuleConfiguration* get_config() const override;
};

#endif /* SRC_MODULE_AUDIO_AUDIOACCUMULATOR_H_ */
