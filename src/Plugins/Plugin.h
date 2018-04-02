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
		AUDIO_SOURCE,
		AUDIO_EFFECT,
		MIDI_SOURCE,
		MIDI_EFFECT,
		SYNTHESIZER,
		BEAT_SOURCE,
		SONG_PLUGIN,
		TSUNAMI_PLUGIN,
		OTHER
	};
	string error_message;

	string get_error();

	void *create_instance(Session *session, const string &root_type);
};

#endif /* PLUGIN_H_ */
