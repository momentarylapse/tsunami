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

#endif /* PLUGIN_H_ */
