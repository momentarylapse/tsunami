/*
 * Effect.h
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#ifndef EFFECT_H_
#define EFFECT_H_

#include "../lib/base/base.h"
#include "../Data/Range.h"
#include "Configurable.h"
#include "../Audio/Source/AudioSource.h"

class Plugin;
class Track;
class AudioBuffer;
class Session;

namespace Script{
class Script;
class Type;
};

class Effect : public Configurable
{
public:
	Effect();
	virtual ~Effect();

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	Plugin *plugin;
	bool usable;
	bool enabled;

	// context
	int sample_rate;

	class Output : public AudioSource
	{
	public:
		Output(Effect *fx);
		virtual int _cdecl read(AudioBuffer &buf);
		virtual void _cdecl reset();
		virtual int _cdecl getPos(int delta);
		virtual int _cdecl getSampleRate();
		void setSource(AudioSource *source);
		Effect *fx;
		AudioSource *source;
	};
	Output *out;

	virtual void _cdecl process(AudioBuffer &buf){};

	void doProcessTrack(Track *t, int layer, const Range &r);

	string getError();
};

Effect *_cdecl CreateEffect(Session *session, const string &name);

#endif /* EFFECT_H_ */
