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


// compiled script
class Plugin
{
public:
	string filename;
	CScript *s;
	hui_callback *f_reset;
	hui_callback *f_data2dialog;
	hui_callback *f_configure;
	hui_callback *f_reset_state;
	int index;
	sType *state_type;
	void *state;
	sType *data_type;
	void *data;


	void ResetData();
	void ResetState();
	bool Configure(bool previewable);
	void DataToDialog();
};

class PluginManager : public HuiEventHandler
{
public:
	PluginManager();
	virtual ~PluginManager();

	void LinkAppScriptData();
	void AddPluginsToMenu();
	void FindAndExecutePlugin();

	void ImportPluginData(Effect &fx);
	void ExportPluginData(Effect &fx);
	void ImportPluginState(Effect &fx);
	void ExportPluginState(Effect &fx);
	void ResetPluginState(Effect &fx);

	void PrepareEffect(Effect &fx);
	void CleanUpEffect(Effect &fx);

	void PutFavoriteBarFixed(CHuiWindow *win, int x, int y, int w);
	void PutFavoriteBarSizable(CHuiWindow *win, const string &root_id, int x, int y);
	void PutCommandBarFixed(CHuiWindow *win, int x, int y, int w);
	void PutCommandBarSizable(CHuiWindow *win, const string &root_id, int x, int y);

	void OnMenuExecutePlugin();
	void ExecutePlugin(const string &filename);

	bool LoadAndCompileEffect(Effect &fx);
	void ApplyEffects(BufferBox &buf, Track *t, Effect &fx);


	bool LoadAndCompilePlugin(const string&);
	void InitPluginData();
	void FinishPluginData();
	void InitFavorites(CHuiWindow *win);

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
};

#endif /* PLUGINMANAGER_H_ */
