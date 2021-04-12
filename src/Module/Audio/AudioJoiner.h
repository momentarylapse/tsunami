/*
 * AudioJoiner.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOJOINER_H_
#define SRC_MODULE_AUDIO_AUDIOJOINER_H_

#include "../Port/Port.h"
#include "../Module.h"

class AudioJoiner : public Module {
public:
	AudioJoiner();

	class Output : public Port {
	public:
		Output(AudioJoiner *j);
		int read_audio(AudioBuffer &buf) override;
		AudioJoiner *joiner;
	};

	Port *a, *b;
};

#endif /* SRC_MODULE_AUDIO_AUDIOJOINER_H_ */
