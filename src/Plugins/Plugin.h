/*
 * Plugin.h
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#ifndef PLUGIN_H_
#define PLUGIN_H_

#include "../lib/base/base.h"
#include "../Data/AudioFile.h"

namespace Script{
class Script;
class Type;
};

// represents a compiled script
class Plugin
{
public:
	typedef void void_func();
	typedef void process_track_func(BufferBox*);

	Plugin(const string &_filename);

	string filename;
	int index;
	Script::Script *s;

	bool usable;
	int type;
	enum{
		TYPE_EFFECT,
		TYPE_OTHER
	};
	string error_message;

	void ProcessTrack(Track *t, int level_no, const Range &r);

	string GetError();
};

class PluginData : public VirtualBase
{
public:
	virtual ~PluginData(){}
	void __init__();
	virtual void __delete__();
	virtual void reset(){}
	Script::Type *type;
};

class Configurable : public VirtualBase
{
public:
	virtual ~Configurable(){}
	void __init__();
	virtual void __delete__();

	virtual void ResetConfig();
	virtual void ResetState();
	virtual void Configure(){};
	virtual void UpdateDialog(){};

	PluginData *get_config();
	PluginData *get_state();

	string ConfigToString();
	void ConfigFromString(const string &options);
};

#endif /* PLUGIN_H_ */
