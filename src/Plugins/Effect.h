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

class Plugin;
class Track;
class BufferBox;

namespace Script{
class Script;
class Type;
};

class EffectParam
{
public:
	string name, type;
	string value;
};

class Effect
{
public:
	Effect();
	Effect(Plugin *p);

	string name;
	Array<EffectParam> param;
	bool only_on_selection;
	Range range;
	Plugin *plugin;
	bool usable;

	Script::Type *state_type;
	void *state;
	Script::Type *data_type;
	void *data;

	void ExportData();
	void ImportData();
	void ExportState();
	void ImportState();
	void Prepare();
	void CleanUp();
	void Apply(BufferBox &buf, Track *t, bool log_error);

	void WriteDataToFile(const string &name);
	void LoadDataFromFile(const string &name);

	void make_usable();
	string GetError();
};

#endif /* EFFECT_H_ */
