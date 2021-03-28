/*
 * AudioChannelSelector.h
 *
 *  Created on: Mar 28, 2021
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOCHANNELSELECTOR_H_
#define SRC_MODULE_AUDIO_AUDIOCHANNELSELECTOR_H_

#include "../Port/Port.h"
#include "../Module.h"
#include "../ModuleConfiguration.h"
#include "PeakMeter.h"

class AudioChannelSelector : public Module {
public:
	AudioChannelSelector();

	class Output : public Port {
	public:
		Output(AudioChannelSelector *cs);
		int read_audio(AudioBuffer &buf) override;
		AudioChannelSelector *cs;
	};
	Output *out;

	Port *source;

	void _cdecl set_channel_map(int num_in, const Array<int> &map);

	void apply(const AudioBuffer &buf_in, AudioBuffer &buf_out);

	int command(ModuleCommand cmd, int param) override;

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
};

#endif /* SRC_MODULE_AUDIO_AUDIOCHANNELSELECTOR_H_ */
