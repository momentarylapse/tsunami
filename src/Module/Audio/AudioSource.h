/*
 * AudioSource.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOSOURCE_H_
#define SRC_MODULE_AUDIO_AUDIOSOURCE_H_

#include "../Module.h"
#include "../Port/AudioPort.h"

class BeatSource;

class AudioSource : public Module
{
public:
	AudioSource();

	void _cdecl __init__();
	void _cdecl __delete__() override;

	class Output : public AudioPort
	{
	public:
		Output(AudioSource *s);
		int _cdecl read(AudioBuffer &buf) override;
		void _cdecl reset() override;
		int _cdecl get_pos(int delta) override;
		AudioSource *source;
	};
	Output *out;

	virtual int _cdecl read(AudioBuffer &buf){ return 0; }
	virtual void _cdecl reset(){}
	virtual int _cdecl get_pos(int delta){ return 0; }
};

AudioSource *_cdecl CreateAudioSource(Session *session, const string &name);

#endif /* SRC_MODULE_AUDIO_AUDIOSOURCE_H_ */
