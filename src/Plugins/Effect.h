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

	string name;
	Array<EffectParam> param;
	bool only_on_selection;
	Range range;
	Plugin *plugin;
	void *state;

	void LoadAndCompile();

	void ExportData();
	void ImportData();
	void ExportState();
	void ImportState();
	void Prepare();
	void CleanUp();
	void Apply(BufferBox &buf, Track *t);
};

#endif /* EFFECT_H_ */
