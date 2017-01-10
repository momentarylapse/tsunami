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

class PluginManager : public HuiEventHandler
{
public:
	PluginManager();
	virtual ~PluginManager();

	void LinkAppScriptData();
	void FindPlugins();
	void AddPluginsToMenu(TsunamiWindow *win);

	void _ExecutePlugin(TsunamiWindow *win, const string &filename);

	Plugin *LoadAndCompilePlugin(int type, const string &filename);
	Plugin *GetPlugin(int type, const string &name);

	void ApplyFavorite(Configurable *c, const string &name);
	void SaveFavorite(Configurable *c, const string &name);
	string SelectFavoriteName(HuiWindow *win, Configurable *c, bool save);

	Array<string> FindSynthesizers();
	Synthesizer *LoadSynthesizer(const string &name, Song *song);

	Effect *ChooseEffect(HuiPanel *parent, Song *song);
	MidiEffect *ChooseMidiEffect(HuiPanel *parent, Song *song);
	//Synthesizer *ChooseSynthesizer(HuiPanel *parent);


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
