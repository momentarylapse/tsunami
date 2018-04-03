/*
 * AudioJoiner.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_AUDIO_AUDIOJOINER_H_
#define SRC_AUDIO_AUDIOJOINER_H_

#include "Source/AudioPort.h"
#include "../Plugins/Configurable.h"

class Session;

class AudioJoiner : public Configurable
{
public:
	AudioJoiner(Session *session);
	virtual ~AudioJoiner();

	class Output : public AudioPort
	{
	public:
		Output(AudioJoiner *j);
		virtual ~Output(){}
		virtual int _cdecl read(AudioBuffer &buf);
		virtual void _cdecl reset();
		virtual int _cdecl get_pos(int delta);
		AudioJoiner *joiner;
	};
	Output *out;

	AudioPort *a, *b;
	void _cdecl set_source_a(AudioPort *a);
	void _cdecl set_source_b(AudioPort *b);
};

#endif /* SRC_AUDIO_AUDIOJOINER_H_ */
