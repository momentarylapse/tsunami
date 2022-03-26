/*
 * Plugin.h
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#ifndef PLUGIN_H_
#define PLUGIN_H_

#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../lib/file/path.h"

namespace kaba {
	class Module;
	class Class;
};

class Session;
enum class ModuleCategory;

// represents a compiled script
class Plugin {
public:
	Plugin(const Path &filename, ModuleCategory type);

	Path filename;
	int file_date;
	int index;
	shared<kaba::Module> module;

	bool usable(Session *session);
	ModuleCategory type;
	string error_message;

	string get_error();

	bool file_changed();
	void recompile(Session *session);

	void *create_instance(Session *session, const string &root_type);
};

#endif /* PLUGIN_H_ */
