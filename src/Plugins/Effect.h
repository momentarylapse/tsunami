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

struct EffectParam
{
	string value;
};

class Effect : public Configurable
{
public:
	Effect();
	Effect(Plugin *p);
	virtual ~Effect();

	void __init__();
	virtual void __delete__();

	bool only_on_selection;
	Range range;
	Plugin *plugin;
	bool usable;
	bool enabled;

	Array<EffectParam> legacy_params;

	virtual void ProcessTrack(BufferBox *buf){};

	void DoProcessTrack(Track *t, int level_no, const Range &r);
	bool DoConfigure(bool previewable);

	void Prepare();
	void Apply(BufferBox &buf, Track *t, bool log_error);

	void WriteConfigToFile(const string &name);
	void LoadConfigFromFile(const string &name);

	string GetError();
};

Effect *CreateEffect(const string &name);

#endif /* EFFECT_H_ */
