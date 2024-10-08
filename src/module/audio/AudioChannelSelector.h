/*
 * AudioChannelSelector.h
 *
 *  Created on: Mar 28, 2021
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOCHANNELSELECTOR_H_
#define SRC_MODULE_AUDIO_AUDIOCHANNELSELECTOR_H_

#include "../port/Port.h"
#include "../Module.h"
#include "../ModuleConfiguration.h"
#include "PeakMeter.h"

namespace tsunami {

class AudioChannelSelector : public Module {
public:
	AudioChannelSelector();
	~AudioChannelSelector() override;

	AudioOutPort out{this};
	AudioInPort in{this};

	int read_audio(int port, AudioBuffer &buf) override;

	void set_channel_map(int num_in, const Array<int> &map);

	void apply(const AudioBuffer &buf_in, AudioBuffer &buf_out);

	base::optional<int64> command(ModuleCommand cmd, int64 param) override;

	ConfigPanel *create_panel() override;


	class Config : public ModuleConfiguration {
	public:
		int channels;
		Array<int> map;
		void reset() override;
		string auto_conf(const string &name) const override;
	} config;

	ModuleConfiguration* get_config() const override;

	owned<PeakMeter> peak_meter;
	int id_runner;
};

}

#endif /* SRC_MODULE_AUDIO_AUDIOCHANNELSELECTOR_H_ */
