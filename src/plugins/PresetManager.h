/*
 * PresetManager.h
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#pragma once

#include "../lib/base/base.h"
#include "../data/midi/Instrument.h"

class Path;
class Module;
enum class ModuleCategory;
enum class SignalType;
namespace hui {
	class Window;
}
class Session;

class PresetManager {
public:
	PresetManager();

	static const string DEFAULT_NAME;

	struct ModulePreset {
		string name;
		ModuleCategory category;
		string _class;
		string options;
		int version;
		bool read_only;
	};

	bool loaded;
	Array<ModulePreset> module_presets;

	void make_usable(Session *session);
	void load(Session *session);
	void load_from_file(const Path &filename, bool read_only, Session *session);
	void load_from_file_old(const Path &filename, bool read_only, Session *session);
	void save(Session *session);

	void set(const ModulePreset &f);

	Array<string> get_list(Module *c);
	void apply(Module *c, const string &name, bool notify);
	void save(Module *c, const string &name);

	void select_name(hui::Window *win, Module *c, bool save, std::function<void(const string&)> cb);

	struct TrackPreset {
		string name;
		SignalType type;
		int channels;
		Instrument instrument;
		string synth_class;
		string synth_options;
		int synth_version;
	};
	void save_track_preset(Session *session, const TrackPreset& p);
	Array<TrackPreset> track_presets;
	TrackPreset dummy_track_preset;
	Array<string> enumerate_track_presets(Session *session);
	const TrackPreset& get_track_preset(Session *session, const string& name);


	void set_favorite(Session *session, ModuleCategory type, const string &name, bool favorite);
	bool is_favorite(Session *session, ModuleCategory type, const string &name);

	struct Favorite {
		ModuleCategory type;
		string name;
		bool operator==(const Favorite&) const;
	};
	Array<Favorite> favorites;
};
