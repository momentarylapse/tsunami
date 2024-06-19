/*
 * AudioEffect.h
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOEFFECT_H_
#define SRC_MODULE_AUDIO_AUDIOEFFECT_H_

#include "../../lib/base/base.h"
#include "../../data/Range.h"
#include "../Module.h"
#include "../port/Port.h"

namespace tsunami {

class Plugin;
class Track;
class TrackLayer;
class AudioBuffer;
class Session;

class AudioEffect : public Module {
public:
	AudioEffect();

	void _cdecl __init__();
	void _cdecl __delete__() override;

	int sample_rate;
	bool apply_to_whole_buffer;
	float wetness;

	int read_audio(int port, AudioBuffer &buf) override;

	AudioOutPort out{this};
	AudioInPort in{this};

	virtual void _cdecl process(AudioBuffer &buf){};
	void apply_with_wetness(AudioBuffer &buf);
	virtual int _cdecl read(AudioBuffer &buf);
};

AudioEffect *_cdecl CreateAudioEffect(Session *session, const string &name);

}

#endif /* SRC_MODULE_AUDIO_AUDIOEFFECT_H_ */
