/*
 * PluginManager.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#include "../Data/Song.h"
#include "../lib/hui/hui.h"
#include "../lib/script/script.h"

class Plugin;
class Effect;
class MidiEffect;
class Synthesizer;
class Configurable;
class FavoriteManager;
class TsunamiWindow;
class SongPlugin;
class TsunamiPlugin;

class PluginManager : public HuiEventHandler
{
public:
	PluginManager();
	virtual ~PluginManager();

	void LinkAppScriptData();
	void FindPlugins();
	void AddPluginsToMenu(TsunamiWindow *win);

	void _ExecutePlugin(TsunamiWindow *win, const string &filename);

	Plugin *LoadAndCompilePlugin(const string &filename);
	Plugin *GetPlugin(const string &name, const string &sub_dir);

	void ApplyFavorite(Configurable *c, const string &name);
	void SaveFavorite(Configurable *c, const string &name);
	string SelectFavoriteName(HuiWindow *win, Configurable *c, bool save);

	Effect *LoadEffect(const string &name);
	MidiEffect *LoadMidiEffect(const string &name);

	Array<string> FindSynthesizers();
	Synthesizer *LoadSynthesizer(const string &name, Song *song);

	SongPlugin *LoadSongPlugin(const string &name);
	TsunamiPlugin *LoadTsunamiPlugin(const string &name);

	Effect *ChooseEffect(HuiPanel *parent, Song *song);
	MidiEffect *ChooseMidiEffect(HuiPanel *parent, Song *song);
	//Synthesizer *ChooseSynthesizer(HuiPanel *parent);


	// not compiled yet
	struct PluginFile
	{
		string name;
		string filename;
		string image;
		Array<string> title;
	};

	Array<PluginFile> plugin_files;

	Array<string> plugin_favorite_names;

	Array<Plugin*> plugins;

	FavoriteManager *favorites;
};

#endif /* PLUGINMANAGER_H_ */
