/*
 * AudioJoiner.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOJOINER_H_
#define SRC_MODULE_AUDIO_AUDIOJOINER_H_

#include "../Port/AudioPort.h"
#include "../Module.h"

class AudioJoiner : public Module
{
public:
	AudioJoiner();

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
};

#endif /* SRC_MODULE_AUDIO_AUDIOJOINER_H_ */
