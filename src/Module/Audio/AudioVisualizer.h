/*
 * AudioVisualizer.h
 *
 *  Created on: 04.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOVISUALIZER_H_
#define SRC_MODULE_AUDIO_AUDIOVISUALIZER_H_

#include "../Module.h"
#include "../Port/Port.h"
#include "../../lib/base/pointer.h"
#include <mutex>

class AudioBuffer;
class RingBuffer;
class Session;

class AudioVisualizer : public Module {
public:
	AudioVisualizer();

	void _cdecl __init__();
	void _cdecl __delete__() override;

	Port *source;

	class Output : public Port {
	public:
		Output(AudioVisualizer *v);
		int read_audio(AudioBuffer &buf) override;
		AudioVisualizer *visualizer;
	};

	owned<RingBuffer> buffer;
	int chunk_size;
	void set_chunk_size(int chunk_size);


	int next_writing = 0;
	std::mutex mutex;
	void lock();
	void unlock();
	void flip();

	virtual _cdecl void process(AudioBuffer &buf){}
};

AudioVisualizer *CreateAudioVisualizer(Session *session, const string &name);

#endif /* SRC_MODULE_AUDIO_AUDIOVISUALIZER_H_ */
