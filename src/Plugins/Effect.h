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

class Plugin;
class Track;
class BufferBox;

namespace Script{
class Script;
class Type;
};

class Effect : public Configurable
{
public:
	Effect();
	Effect(Plugin *p);
	virtual ~Effect();

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	bool only_on_selection;
	Range range;
	Plugin *plugin;
	bool usable;
	bool enabled;

	virtual void _cdecl processTrack(BufferBox *buf){};

	void doProcessTrack(Track *t, int level_no, const Range &r);

	void prepare();
	void apply(BufferBox &buf, Track *t, bool log_error);

	string getError();
};

Effect *_cdecl CreateEffect(const string &name, Song *song);

#endif /* EFFECT_H_ */
