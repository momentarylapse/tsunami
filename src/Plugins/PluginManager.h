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

class PluginManager : public HuiEventHandler, public Observer
{
public:
	PluginManager();
	virtual ~PluginManager();

	void LinkAppScriptData();
	void AddPluginsToMenu();
	void FindAndExecutePlugin();

	void PutFavoriteBarFixed(CHuiWindow *win, int x, int y, int w);
	void PutFavoriteBarSizable(CHuiWindow *win, const string &root_id, int x, int y);
	void PutCommandBarFixed(CHuiWindow *win, int x, int y, int w);
	void PutCommandBarSizable(CHuiWindow *win, const string &root_id, int x, int y);

	void OnMenuExecutePlugin();
	void ExecutePlugin(const string &filename);

	Plugin *GetPlugin(const string &name);

	bool LoadAndCompilePlugin(const string&);
	void InitPluginData();
	void FinishPluginData();
	void InitFavorites(CHuiWindow *win);

	void Preview(Effect &fx);

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

	virtual void OnUpdate(Observable *o);

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

	struct PluginContext
	{
		Range range;
		Track *track;
		int track_no;
		int level;
		void set(Track *t, int l, const Range &r);
	};
	PluginContext context;
};

#endif /* PLUGINMANAGER_H_ */
