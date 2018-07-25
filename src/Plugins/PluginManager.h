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
namespace hui{
	class Window;
	class Panel;
}

class PluginManager
{
public:
	PluginManager();
	virtual ~PluginManager();

	string plugin_dir();

	void LinkAppScriptData();
	void FindPlugins();
	void AddPluginsToMenu(TsunamiWindow *win);

	Plugin *LoadAndCompilePlugin(ModuleType type, const string &filename);
	Plugin *GetPlugin(Session *session, ModuleType type, const string &name);

	void ApplyFavorite(Module *c, const string &name);
	void SaveFavorite(Module *c, const string &name);
	string SelectFavoriteName(hui::Window *win, Module *c, bool save);

	Array<string> FindAudioEffects();
	Array<string> FindModuleSubTypes(ModuleType type);

	string ChooseModule(hui::Panel *parent, Session *session, ModuleType type, const string &old_name = "");


	// not compiled yet
	struct PluginFile
	{
		string name;
		string filename;
		string image;
		ModuleType type;
		Array<string> title;
	};

	Array<PluginFile> plugin_files;

	Array<string> plugin_favorite_names;

	Array<Plugin*> plugins;

	FavoriteManager *favorites;
};

#endif /* PLUGINMANAGER_H_ */
