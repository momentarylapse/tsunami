/*
 * AudioVisualizer.h
 *
 *  Created on: 04.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOVISUALIZER_H_
#define SRC_MODULE_AUDIO_AUDIOVISUALIZER_H_

#include "../Module.h"
#include "../port/Port.h"
#include "../../lib/base/pointer.h"

namespace tsunami {

class AudioBuffer;
class RingBuffer;
class Session;

class AudioVisualizer : public Module {
public:
	AudioVisualizer();
	~AudioVisualizer() override;

	void _cdecl __init__();
	void _cdecl __delete__() override;

	AudioOutPort out{this};
	AudioInPort in{this};

	int read_audio(int port, AudioBuffer &buf) override;
	void feed(const AudioBuffer& buf);

	owned<RingBuffer> buffer;
	int chunk_size;
	void set_chunk_size(int chunk_size);

	int id_runner;

	// will be called in ui thread!
	virtual void process(AudioBuffer &buf){}
};

}

#endif /* SRC_MODULE_AUDIO_AUDIOVISUALIZER_H_ */
