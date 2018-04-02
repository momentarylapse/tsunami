/*
 * AudioSource.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_AUDIO_SOURCE_AUDIOSOURCE_H_
#define SRC_AUDIO_SOURCE_AUDIOSOURCE_H_

#include "../../Plugins/Configurable.h"
#include "AudioPort.h"

class AudioSource : public Configurable
{
public:
	AudioSource();
	virtual ~AudioSource();

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	class Output : public AudioPort
	{
	public:
		Output(AudioSource *s);
		virtual int _cdecl read(AudioBuffer &buf);
		virtual void _cdecl reset();
		virtual int _cdecl get_pos(int delta);
		AudioSource *source;
	};
	Output *out;

	virtual int _cdecl read(AudioBuffer &buf){ return 0; }
	virtual void _cdecl reset(){}
	virtual int _cdecl get_pos(int delta){ return 0; }
};

AudioSource *_cdecl CreateAudioSource(Session *session, const string &name);

#endif /* SRC_AUDIO_SOURCE_AUDIOSOURCE_H_ */
