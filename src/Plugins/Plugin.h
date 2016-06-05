/*
 * Plugin.h
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#ifndef PLUGIN_H_
#define PLUGIN_H_

#include "../Data/Song.h"
#include "../lib/base/base.h"

namespace Script{
class Script;
class Type;
};

// represents a compiled script
class Plugin
{
public:
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

	string getError();

	void *createInstance(const string &root_type);
};

#endif /* PLUGIN_H_ */
