/*
 * PluginManager.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#include "../lib/kaba/kaba.h"
#include "../lib/base/optional.h"
#include "../lib/base/future.h"


namespace hui {
	class Window;
	class Panel;
	class Menu;
}
namespace kaba {
	class Module;
	class Class;
}

namespace tsunami {

class Plugin;
class Module;
enum class ModuleCategory;
class PresetManager;
class TsunamiWindow;
class SongPlugin;
class TsunamiPlugin;
class Session;

class PluginManager {
public:
	PluginManager();
	virtual ~PluginManager();

	Path plugin_dir_static();
	Path plugin_dir_local();

	void link_app_data();
	void find_plugins();
	void add_plugins_to_menu(TsunamiWindow *win);

	Plugin *load_and_compile_plugin(ModuleCategory type, const Path &filename);
	Plugin *get_plugin(Session *session, ModuleCategory type, const string &name);

	void apply_module_preset(Module *c, const string &name, bool notify);
	void save_module_preset(Module *c, const string &name);
	base::future<string> select_module_preset_name(hui::Window *win, Module *c, bool save);

	Array<string> find_module_sub_types(ModuleCategory type);
	Array<string> find_module_sub_types_grouped(ModuleCategory type);

	// (potentially) not compiled yet
	struct PluginFile {
		string name;
		string group;
		Path filename;
		string image;
		ModuleCategory type;
		Array<string> title;
	};

	Array<PluginFile> plugin_files;

	Array<string> plugin_favorite_names;

	Array<Plugin*> plugins;

	owned<PresetManager> presets;


	shared<kaba::Module> package;
	kaba::Class *get_class(const string &name);

	void set_favorite(Session *session, ModuleCategory type, const string &name, bool favorite);
	bool is_favorite(Session *session, ModuleCategory type, const string &name);

private:
	void find_plugins_in_dir_absolute(const Path &_dir, const string &group, ModuleCategory type);
	void find_plugins_in_dir(const Path &rel, const string &group, ModuleCategory type);
	using PluginCallback = std::function<void(const string &)>;
	void add_plugins_in_dir(const Path &dir, hui::Menu *m, const string &name_space, TsunamiWindow *win, PluginCallback cb);
};

}

#endif /* PLUGINMANAGER_H_ */
