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
class AudioEffect;
class MidiEffect;
class Synthesizer;
class Module;
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

	void _ExecutePlugin(Session *session, const string &filename);

	Plugin *LoadAndCompilePlugin(int type, const string &filename);
	Plugin *GetPlugin(Session *session, int type, const string &name);

	void ApplyFavorite(Module *c, const string &name);
	void SaveFavorite(Module *c, const string &name);
	string SelectFavoriteName(hui::Window *win, Module *c, bool save);

	Array<string> FindSynthesizers();
	Array<string> FindAudioSources();
	Array<string> FindAudioEffects();
	Array<string> FindMidiSources();
	Array<string> FindMidiEffects();
	Array<string> FindBeatSources();
	Array<string> FindConfigurable(int type);
	Synthesizer *__LoadSynthesizer(Session *session, const string &name);
	Synthesizer *CreateSynthesizer(Session *session, const string &name);

	AudioEffect *ChooseEffect(hui::Panel *parent, Session *session);
	MidiEffect *ChooseMidiEffect(hui::Panel *parent, Session *session);
	Synthesizer *ChooseSynthesizer(hui::Window *parent, Session *session, const string &old_name = "");


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
