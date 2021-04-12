/*
 * AudioSource.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOSOURCE_H_
#define SRC_MODULE_AUDIO_AUDIOSOURCE_H_

#include "../Module.h"
#include "../Port/Port.h"

class BeatSource;

class AudioSource : public Module {
public:
	AudioSource();

	void _cdecl __init__();
	void _cdecl __delete__() override;

	class Output : public Port {
	public:
		Output(AudioSource *s);
		int read_audio(AudioBuffer &buf) override;
		AudioSource *source;
	};

	// to be implemented by plugins
	virtual int _cdecl read(AudioBuffer &buf){ return 0; }
};

AudioSource *_cdecl CreateAudioSource(Session *session, const string &name);

#endif /* SRC_MODULE_AUDIO_AUDIOSOURCE_H_ */
