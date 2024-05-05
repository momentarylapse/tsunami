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
#include <mutex>
#include <atomic>

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

	owned<RingBuffer> buffer;
	int chunk_size;
	void set_chunk_size(int chunk_size);


	int next_writing = 0;
	int current_reading = 1;
	std::mutex mutex;
	std::atomic<int> notify_counter;
	void lock();
	void unlock();
	void flip();

	virtual _cdecl void process(AudioBuffer &buf){}
};

AudioVisualizer *CreateAudioVisualizer(Session *session, const string &name);

#endif /* SRC_MODULE_AUDIO_AUDIOVISUALIZER_H_ */
