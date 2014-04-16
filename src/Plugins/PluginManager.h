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
class Synthesizer;
class Configurable;
class FavoriteManager;

class PluginManager : public HuiEventHandler, public Observer
{
public:
	PluginManager();
	virtual ~PluginManager();

	void LinkAppScriptData();
	void AddPluginsToMenu();
	void FindAndExecutePlugin();

	void PutFavoriteBarSizable(HuiPanel *panel, const string &root_id, int x, int y);
	void PutCommandBarSizable(HuiPanel *panel, const string &root_id, int x, int y);

	void OnMenuExecutePlugin();
	void ExecutePlugin(const string &filename);

	Plugin *GetPlugin(const string &name);

	bool LoadAndCompilePlugin(const string&);
	void InitPluginData();
	void FinishPluginData();
	void InitFavorites(HuiPanel *panel);

	void PreviewStart(Effect *fx);
	void PreviewEnd();

	Array<string> GetFavoriteList(Configurable *c);
	void ApplyFavorite(Configurable *c, const string &name);
	void SaveFavorite(Configurable *c, const string &name);
	string SelectFavoriteName(HuiWindow *win, Configurable *c, bool save);

	void OnFavoriteName();
	void OnFavoriteList();
	void OnFavoriteSave();
	void OnFavoriteDelete();
	void OnPluginFavoriteName();
	void OnPluginFavoriteList();
	void OnPluginFavoriteSave();
	void OnPluginPreview();
	void OnPluginOk();
	void OnPluginClose();

	virtual void OnUpdate(Observable *o, const string &message);

	Effect *LoadEffect(const string &name);

	Array<string> FindSynthesizers();
	Synthesizer *LoadSynthesizer(const string &name);

	bool ConfigureSynthesizer(Synthesizer *s);

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

	bool PluginCancelled;
	bool PluginAddPreview;

	Array<Plugin*> plugin;
	Plugin *cur_plugin;
	Effect *cur_effect;
	Synthesizer *cur_synth;
	Configurable *get_configurable();

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
