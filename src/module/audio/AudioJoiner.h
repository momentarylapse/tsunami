/*
 * AudioJoiner.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOJOINER_H_
#define SRC_MODULE_AUDIO_AUDIOJOINER_H_

#include "../port/Port.h"
#include "../Module.h"

namespace tsunami {

class AudioJoiner : public Module {
public:
	AudioJoiner();

	AudioOutPort out{this};
	AudioInPort in_a{this, "a"};
	AudioInPort in_b{this, "b"};
	AudioInPort in_c{this, "c"};
	AudioInPort in_d{this, "d"};

	int read_audio(int port, AudioBuffer &buf) override;
};

}

#endif /* SRC_MODULE_AUDIO_AUDIOJOINER_H_ */
