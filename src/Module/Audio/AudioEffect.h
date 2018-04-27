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
#include "../Port/AudioPort.h"

class Plugin;
class Track;
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
	virtual ~AudioEffect();

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	int sample_rate;

	class Output : public AudioPort
	{
	public:
		Output(AudioEffect *fx);
		virtual int _cdecl read(AudioBuffer &buf);
		virtual void _cdecl reset();
		virtual int _cdecl get_pos(int delta);
		AudioEffect *fx;
	};
	Output *out;

	AudioPort *source;
	void set_source(AudioPort *source);

	virtual void _cdecl process(AudioBuffer &buf){};

	void do_process_track(Track *t, int layer, const Range &r);
};

AudioEffect *_cdecl CreateAudioEffect(Session *session, const string &name);

#endif /* SRC_MODULE_AUDIO_AUDIOEFFECT_H_ */