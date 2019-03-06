/*
 * AudioEffect.h
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOEFFECT_H_
#define SRC_MODULE_AUDIO_AUDIOEFFECT_H_

#include "../../lib/base/base.h"
#include "../../Data/Range.h"
#include "../Module.h"
#include "../Port/Port.h"

class Plugin;
class Track;
class TrackLayer;
class AudioBuffer;
class Session;

namespace Script{
class Script;
class Type;
};

class AudioEffect : public Module
{
public:
	AudioEffect();

	void _cdecl __init__();
	void _cdecl __delete__() override;

	int sample_rate;

	class Output : public Port
	{
	public:
		Output(AudioEffect *fx);
		int read_audio(AudioBuffer &buf) override;
		AudioEffect *fx;
	};
	Output *out;

	Port *source;

	virtual void _cdecl process(AudioBuffer &buf){};
	virtual int _cdecl read(AudioBuffer &buf);

	void do_process_track(TrackLayer *l, const Range &r);
};

AudioEffect *_cdecl CreateAudioEffect(Session *session, const string &name);

#endif /* SRC_MODULE_AUDIO_AUDIOEFFECT_H_ */
