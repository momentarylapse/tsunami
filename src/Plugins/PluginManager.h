/*
 * PluginManager.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#include "../lib/kaba/kaba.h"

class Plugin;
class Module;
enum class ModuleType;
class FavoriteManager;
class TsunamiWindow;
class SongPlugin;
class TsunamiPlugin;
class Session;
namespace hui {
	class Window;
	class Panel;
	class Menu;
}
namespace Kaba {
	class Script;
	class Class;
}

class PluginManager {
public:
	PluginManager();
	virtual ~PluginManager();

	Path plugin_dir_static();
	Path plugin_dir_local();

	void link_app_script_data();
	void find_plugins();
	void add_plugins_to_menu(TsunamiWindow *win);

	Plugin *load_and_compile_plugin(ModuleType type, const Path &filename);
	Plugin *get_plugin(Session *session, ModuleType type, const string &name);

	void apply_favorite(Module *c, const string &name, bool notify);
	void save_favorite(Module *c, const string &name);
	string select_favorite_name(hui::Window *win, Module *c, bool save);

	Array<string> find_module_sub_types(ModuleType type);
	Array<string> find_module_sub_types_grouped(ModuleType type);

	static string choose_module(hui::Panel *parent, Session *session, ModuleType type, const string &old_name = "");


	// not compiled yet
	struct PluginFile {
		string name;
		string group;
		Path filename;
		string image;
		ModuleType type;
		Array<string> title;
	};

	Array<PluginFile> plugin_files;

	Array<string> plugin_favorite_names;

	Array<Plugin*> plugins;

	FavoriteManager *favorites;


	Kaba::Script *package;
	Kaba::Class *get_class(const string &name);

private:
	void find_plugins_in_dir_absolute(const Path &_dir, const string &group, ModuleType type);
	void find_plugins_in_dir(const Path &rel, const string &group, ModuleType type);
	void add_plugins_in_dir(const Path &dir, hui::Menu *m, const string &name_space, TsunamiWindow *win, void (TsunamiWindow::*function)());
};

#endif /* PLUGINMANAGER_H_ */
