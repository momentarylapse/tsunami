/*
 * PluginManager.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#include "../Data/Song.h"
#include "../lib/kaba/kaba.h"

class Plugin;
class Effect;
class MidiEffect;
class Synthesizer;
class Configurable;
class FavoriteManager;
class TsunamiWindow;
class SongPlugin;
class TsunamiPlugin;
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

	void _ExecutePlugin(TsunamiWindow *win, const string &filename);

	Plugin *LoadAndCompilePlugin(int type, const string &filename);
	Plugin *GetPlugin(int type, const string &name);

	void ApplyFavorite(Configurable *c, const string &name);
	void SaveFavorite(Configurable *c, const string &name);
	string SelectFavoriteName(hui::Window *win, Configurable *c, bool save);

	Array<string> FindSynthesizers();
	Array<string> FindEffects();
	Array<string> FindMidiEffects();
	Array<string> FindConfigurable(int type);
	Synthesizer *__LoadSynthesizer(const string &name, Song *song);
	Synthesizer *CreateSynthesizer(const string &name, Song *song);

	Effect *ChooseEffect(hui::Panel *parent, Song *song);
	MidiEffect *ChooseMidiEffect(hui::Panel *parent, Song *song);
	Synthesizer *ChooseSynthesizer(hui::Window *parent, Song *song, const string &old_name = "");


	// not compiled yet
	struct PluginFile
	{
		string name;
		string filename;
		string image;
		int type;
		Array<string> title;
	};

	Array<PluginFile> plugin_files;

	Array<string> plugin_favorite_names;

	Array<Plugin*> plugins;

	FavoriteManager *favorites;
};

#endif /* PLUGINMANAGER_H_ */
