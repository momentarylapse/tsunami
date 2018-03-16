/*
 * Plugin.h
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#ifndef PLUGIN_H_
#define PLUGIN_H_

#include "../lib/base/base.h"

namespace Kaba{
class Script;
class Class;
};

class Session;

// represents a compiled script
class Plugin
{
public:
	Plugin(const string &_filename, int type);

	string filename;
	int index;
	Kaba::Script *s;

	bool usable;
	int type;
	enum Type{
		EFFECT,
		MIDI_EFFECT,
		SONG_PLUGIN,
		TSUNAMI_PLUGIN,
		SYNTHESIZER,
		OTHER
	};
	string error_message;

	string getError();

	void *createInstance(Session *session, const string &root_type);
};

#endif /* PLUGIN_H_ */
