/*
 * PluginManager.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#include "../lib/hui/hui.h"
#include "../lib/script/script.h"
#include "../Data/AudioFile.h"
#include "../Stuff/Observer.h"

class Plugin;
class Effect;
class MidiEffect;
class Synthesizer;
class Configurable;
class FavoriteManager;

class PluginManager : public HuiEventHandler, public Observer
{
public:
	PluginManager();
	virtual ~PluginManager();

	void LinkAppScriptData();
	void FindPlugins();
	void AddPluginsToMenu(HuiWindow *win);
	void FindAndExecutePlugin();

	void OnMenuExecutePlugin();
	void ExecutePlugin(const string &filename);

	Plugin *GetPlugin(const string &name);

	bool LoadAndCompilePlugin(const string&);

	void PreviewStart(Effect *fx);
	void PreviewEnd();

	void ApplyFavorite(Configurable *c, const string &name);
	void SaveFavorite(Configurable *c, const string &name);
	string SelectFavoriteName(HuiWindow *win, Configurable *c, bool save);

	virtual void onUpdate(Observable *o, const string &message);

	Effect *LoadEffect(const string &name);
	MidiEffect *LoadMidiEffect(const string &name);

	Array<string> FindSynthesizers();
	Synthesizer *LoadSynthesizer(const string &name);

	Effect *ChooseEffect(HuiPanel *parent);
	MidiEffect *ChooseMidiEffect(HuiPanel *parent);
	//Synthesizer *ChooseSynthesizer(HuiPanel *parent);


	// not compiled yet
	struct PluginFile
	{
		string name;
		string filename;
		string image;
		Array<string> title;
	};

	Array<PluginFile> plugin_file;
	bool ErrorApplyingEffect;

	Array<string> PluginFavoriteName;

	Array<Plugin*> plugin;
	Plugin *cur_plugin;
	Effect *cur_effect;

	struct PluginContext
	{
		Range range;
		Track *track;
		int track_no;
		int level;
		void set(Track *t, int l, const Range &r);
	};
	PluginContext context;

	FavoriteManager *favorites;
};

#endif /* PLUGINMANAGER_H_ */
