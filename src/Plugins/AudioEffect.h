/*
 * AudioEffect.h
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#ifndef AUDIOEFFECT_H_
#define AUDIOEFFECT_H_

#include "../lib/base/base.h"
#include "../Data/Range.h"
#include "Configurable.h"
#include "../Audio/Source/AudioPort.h"

class Plugin;
class Track;
class AudioBuffer;
class Session;

namespace Script{
class Script;
class Type;
};

class AudioEffect : public Configurable
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

#endif /* AUDIOEFFECT_H_ */
