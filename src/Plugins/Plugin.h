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
enum class ModuleType;

// represents a compiled script
class Plugin
{
public:
	Plugin(const string &_filename, ModuleType type);

	string filename;
	int index;
	Kaba::Script *s;

	bool usable;
	ModuleType type;
	string error_message;

	string get_error();

	void *create_instance(Session *session, const string &root_type);
};

#endif /* PLUGIN_H_ */
