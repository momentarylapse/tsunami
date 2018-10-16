/*
 * AudioVisualizer.h
 *
 *  Created on: 04.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOVISUALIZER_H_
#define SRC_MODULE_AUDIO_AUDIOVISUALIZER_H_

#include "../Module.h"
#include "../Port/AudioPort.h"

class AudioBuffer;
class RingBuffer;
class Session;

class AudioVisualizer : public Module
{
public:
	AudioVisualizer();
	~AudioVisualizer() override;

	void _cdecl __init__();
	void _cdecl __delete__() override;

	AudioPort *source;

	class Output : public AudioPort
	{
	public:
		Output(AudioVisualizer *v);
		~Output() override {}
		int _cdecl read(AudioBuffer &buf) override;
		int _cdecl get_pos(int delta) override;
		void _cdecl reset() override;
		AudioVisualizer *visualizer;
	};

	RingBuffer *buffer;
	int chunk_size;
	void set_chunk_size(int chunk_size);

	virtual _cdecl void process(AudioBuffer &buf){}
	virtual _cdecl void reset(){}
};

AudioVisualizer *CreateAudioVisualizer(Session *session, const string &name);

#endif /* SRC_MODULE_AUDIO_AUDIOVISUALIZER_H_ */
