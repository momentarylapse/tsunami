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

class PluginManager : public HuiEventHandler
{
public:
	PluginManager();
	virtual ~PluginManager();

	void LinkAppScriptData();
	void FindPlugins();
	void AddPluginsToMenu(HuiWindow *win, void (TsunamiWindow::*function)());
	void FindAndExecutePlugin(TsunamiWindow *win);

	void ExecutePlugin(TsunamiWindow *win, const string &filename);

	Plugin *GetPlugin(const string &name);

	Plugin *LoadAndCompilePlugin(const string &filename);

	void ApplyFavorite(Configurable *c, const string &name);
	void SaveFavorite(Configurable *c, const string &name);
	string SelectFavoriteName(HuiWindow *win, Configurable *c, bool save);

	Effect *LoadEffect(const string &name, Song *song);
	MidiEffect *LoadMidiEffect(const string &name, Song *song);

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
		Array<string> title;
	};

	Array<PluginFile> plugin_files;

	Array<string> plugin_favorite_names;

	Array<Plugin*> plugins;

	struct PluginContext
	{
		Range range;
		Song *song;
		Track *track;
		int track_no;
		int level;
		void set(Track *t, int l, const Range &r);
	};
	PluginContext context;

	FavoriteManager *favorites;
};

#endif /* PLUGINMANAGER_H_ */
