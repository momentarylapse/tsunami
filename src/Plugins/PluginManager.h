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

class PluginManager : public HuiEventHandler
{
public:
	PluginManager();
	virtual ~PluginManager();

	void LinkAppScriptData();
	void AddPluginsToMenu();
	void FindAndExecutePlugin();
	void DoPlugins(int message);

	void PushCurPlugin(CScript *s);
	void PopCurPlugin();

	void ImportPluginData(Effect &fx);
	void ExportPluginData(Effect &fx);

	void PutFavoriteBarFixed(CHuiWindow *win, int x, int y, int w);
	void PutFavoriteBarSizable(CHuiWindow *win, const string &root_id, int x, int y);
	void PutCommandBarFixed(CHuiWindow *win, int x, int y, int w);
	void PutCommandBarSizable(CHuiWindow *win, const string &root_id, int x, int y);

	void OnMenuExecutePlugin();
	void ExecutePlugin(const string &filename);

	bool LoadAndCompileEffect(const string &filename);
	void ApplyEffects(BufferBox &buf, Track *t, Effect *fx);


	bool LoadAndCompilePlugin(const string&);
	void PluginResetData();
	bool PluginConfigure(bool previewable);
	void InitPluginData();
	void FinishPluginData();
	void InitFavorites(CHuiWindow *win);

	void PluginDataToDialog();
	void WritePluginDataToFile(const string &name);
	void LoadPluginDataFromFile(const string &name);
	void PluginPreview();
	void PluginProcessTrack(CScript *s, Track *t, int level_no, Range r);

	void OnFavoriteName();
	void OnFavoriteList();
	void OnFavoriteSave();
	void OnFavoriteDelete();
	void OnPluginFavoriteName();
	void OnPluginFavoriteList();
	void OnPluginFavoriteSave();
	void OnPluginOk();
	void OnPluginClose();

	struct Plugin
	{
		string filename;
		CScript *s;
		hui_callback *f_reset, *f_data2dialog, *f_configure;
		int index;
	};


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

	Array<Plugin> plugin;
	Plugin *cur_plugin;
	Array<int> cur_plugin_stack;
};

#endif /* PLUGINMANAGER_H_ */
