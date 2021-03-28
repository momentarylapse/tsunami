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

	int num_in;
	Array<int> map;
	void _cdecl set_map(int num_in, const Array<int> &map);

	void apply(const AudioBuffer &buf_in, AudioBuffer &buf_out);

	int command(ModuleCommand cmd, int param) override;
};

#endif /* SRC_MODULE_AUDIO_AUDIOCHANNELSELECTOR_H_ */
