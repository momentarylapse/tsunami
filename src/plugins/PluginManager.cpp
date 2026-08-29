/*
 * PluginManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PluginManager.h"
#include "Plugin.h"
#include "PresetManager.h"
#include <Tsunami.h>
#include <Session.h>
#include <module/SignalChain.h>
#include <storage/Storage.h>
#include <device/DeviceManager.h>
#include <stuff/Clipboard.h>
#include <stuff/BackupManager.h>
#include <stuff/SessionManager.h>
#include <stuff/PerformanceMonitor.h>
#include <view/TsunamiWindow.h>
#include <data/Song.h>
#include <lib/obs/Log.h>
#include <lib/hui/Menu.h>
#include <lib/hui/language.h>
#include <lib/hui/_kaba_export.h>
#include <lib/os/app.h>
#include <lib/os/filesystem.h>
#include <lib/fft/_kaba_export.h>
#include <lib/obs/_kaba_export.h>
#include <lib/os/msg.h>


namespace tsunami {

PluginManager::PluginManager() {
	presets = new PresetManager;

	kaba::init();
	kaba::config.show_compiler_stats = false;
	kaba::config.compile_silently = true;

	find_plugins();

	auto ctx = reinterpret_cast<kaba::Context*>(kaba::default_context);
	internal_module = ctx->create_empty_module("<tsunami-internal>");
	//ctx->internal_packages.add(internal_module);
	ctx->public_modules.add(internal_module.get());

	auto *type_dev = internal_module->tree->create_new_class("Device", kaba::MetaClass::NONE, nullptr, 0, 0, nullptr, {}, internal_module->tree->base_class, -1);
	internal_module->tree->get_pointer(type_dev);
	//package->tree->make_class("Device*", kaba::Class::Type::POINTER, sizeof(void*), 0, nullptr, {type_dev}, package->tree->base_class, -1);
}

PluginManager::~PluginManager() {
	//plugins.clear();
	//kaba::clean_up();
}


void PluginManager::link_app_data() {
	kaba::config.directory = Path::EMPTY;

	auto ctx = reinterpret_cast<kaba::Context*>(kaba::default_context);
	ctx->register_package_init("hui", this->plugin_dir_static() | "hui", &export_package_hui);
	ctx->register_package_init("fft", this->plugin_dir_static() | "fft", &export_package_fft);
	ctx->register_package_init("obs", this->plugin_dir_static() | "obs", &export_package_obs);
	ctx->register_package_init("tsunami", this->plugin_dir_static() | "tsunami", &PluginManager::export_kaba_package_tsunami);
}

void init_app() {

	auto t = new Tsunami();
	Tsunami::instance = t;
	t->backup_manager = new BackupManager;
	t->session_manager = new SessionManager(t->backup_manager.get());

	t->perf_mon = new PerformanceMonitor;

	t->log_hub = new obs::LogHub;

	Session::GLOBAL = new Session(t->log_hub->create_broadcaster(), nullptr, nullptr, t->session_manager.get(), t->perf_mon.get());

	t->clipboard = new Clipboard;

	t->device_manager = new DeviceManager(Session::GLOBAL->log_source.get(), Session::GLOBAL);
	Session::GLOBAL->device_manager = t->device_manager.get();

	// create (link) PluginManager after all other components are ready
	t->plugin_manager = new PluginManager;
	Session::GLOBAL->plugin_manager = t->plugin_manager.get();
}

kaba::Class* PluginManager::get_class(const string &name) {
	for (auto c: weak(internal_module->tree->base_class->classes))
		if (c->name == name)
			return (kaba::Class*)c;
	return internal_module->tree->create_new_class(name, kaba::MetaClass::NONE, nullptr, 0, 0, nullptr, {}, internal_module->tree->base_class, -1);
}

void get_plugin_file_data(PluginManager::PluginFile &pf) {
	pf.image = "";
	try {
		string content = os::fs::read_text(pf.filename);
		int p = content.find("// Image = hui:");
		if (p >= 0)
			pf.image = content.sub(p + 11, content.find("\n"));
	} catch(...) {}
}

void PluginManager::find_plugins_in_dir_absolute(const Path &_dir, const string &group, ModuleCategory type) {
	Path dir = _dir;
	if (group.num > 0)
		dir |= group;
	auto list = os::fs::search(dir, "*.kaba", "f");
	for (auto &e: list) {
		if (str(e).head(2) == "__")
			continue;
		PluginManager::PluginFile pf;
		pf.type = type;
		pf.name = e.no_ext().str();
		pf.filename = dir | e;
		pf.group = group;
		get_plugin_file_data(pf);
		plugin_files.add(pf);
	}
}

void PluginManager::find_plugins_in_dir(const Path &rel, const string &group, ModuleCategory type) {
	find_plugins_in_dir_absolute(plugin_dir_static() | rel, group, type);
	if (plugin_dir_local() != plugin_dir_static())
		find_plugins_in_dir_absolute(plugin_dir_local() | rel, group, type);
}

void PluginManager::add_plugins_in_dir(const Path &dir, hui::Menu *m, const string &name_space, TsunamiWindow *win, PluginCallback cb) {
	for (auto &f: plugin_files) {
		if (f.filename.is_in(plugin_dir_static() | dir) or f.filename.is_in(plugin_dir_local() | dir)) {
			string id = format("execute-%s--%s", name_space, f.name);
			m->add_with_image(f.name, f.image, id);
			win->event(id, [cb,f]{ cb(f.name); });
		}
	}
}

void PluginManager::find_plugins() {

	// "audiosource"
	find_plugins_in_dir("audio_source", "", ModuleCategory::AudioSource);

	// "audioeffect"
	find_plugins_in_dir("audio_effect", "channels", ModuleCategory::AudioEffect);
	find_plugins_in_dir("audio_effect", "dynamics", ModuleCategory::AudioEffect);
	find_plugins_in_dir("audio_effect", "echo", ModuleCategory::AudioEffect);
	find_plugins_in_dir("audio_effect", "filter", ModuleCategory::AudioEffect);
	find_plugins_in_dir("audio_effect", "pitch", ModuleCategory::AudioEffect);
	find_plugins_in_dir("audio_effect", "repair", ModuleCategory::AudioEffect);
	find_plugins_in_dir("audio_effect", "sound", ModuleCategory::AudioEffect);
	// hidden...
	find_plugins_in_dir("audio_effect", "special", ModuleCategory::AudioEffect);

	// "audiovisualizer"
	find_plugins_in_dir("audio_visualizer", "", ModuleCategory::AudioVisualizer);

	// "midisource"
	find_plugins_in_dir("midi_source", "", ModuleCategory::MidiSource);

	// "midieffect"
	find_plugins_in_dir("midi_effect", "", ModuleCategory::MidiEffect);

	// "beatsource"
	find_plugins_in_dir("beat_source", "", ModuleCategory::BeatSource);

	// "pitchdetector"
	find_plugins_in_dir("pitch_detector", "", ModuleCategory::PitchDetector);

	// rest
	find_plugins_in_dir("independent", "debug", ModuleCategory::TsunamiPlugin);
	find_plugins_in_dir("independent", "file_edit", ModuleCategory::TsunamiPlugin);
	find_plugins_in_dir("independent", "file_management", ModuleCategory::TsunamiPlugin);
	find_plugins_in_dir("independent", "file_visualization", ModuleCategory::TsunamiPlugin);
	find_plugins_in_dir("independent", "games", ModuleCategory::TsunamiPlugin);
	find_plugins_in_dir("independent", "live_performance", ModuleCategory::TsunamiPlugin);
	find_plugins_in_dir("independent", "practice", ModuleCategory::TsunamiPlugin);
	find_plugins_in_dir("independent", "special", ModuleCategory::TsunamiPlugin);

	// "Synthesizer"
	find_plugins_in_dir("synthesizer", "", ModuleCategory::Synthesizer);
}

void PluginManager::add_plugins_to_menu(TsunamiWindow *win) {
	hui::Menu *m = win->get_menu();

	add_plugins_in_dir("independent/debug", m->get_sub_menu_by_id("menu_plugins_debug"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("independent/file_edit", m->get_sub_menu_by_id("menu_plugins_file_edit"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("independent/file_management", m->get_sub_menu_by_id("menu_plugins_file_management"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("independent/file_visualization", m->get_sub_menu_by_id("menu_plugins_file_visualization"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("independent/games", m->get_sub_menu_by_id("menu_plugins_games"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("independent/live_performance", m->get_sub_menu_by_id("menu_plugins_live_performance"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("independent/practice", m->get_sub_menu_by_id("menu_plugins_practice"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("independent/special", m->get_sub_menu_by_id("menu_plugins_special"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
}

void PluginManager::apply_module_preset(Module *c, const string &name, bool notify) {
	presets->apply(c, name, notify);
}

void PluginManager::save_module_preset(Module *c, const string &name) {
	presets->save(c, name);
}


base::future<string> PluginManager::select_module_preset_name(hui::Window *win, Module *c, bool save) {
	return presets->select_name(win, c, save);
}

// always push the script... even if an error occurred
//   don't log error...
Plugin *PluginManager::load_and_compile_plugin(ModuleCategory type, const Path &filename) {
	for (Plugin *p: plugins)
		if (filename == p->filename)
			return p;

	//InitPluginData();

	Plugin *p = new Plugin(filename, type);
	p->index = plugins.num;

	plugins.add(p);

	return p;
}


Plugin *PluginManager::get_plugin(Session *session, ModuleCategory type, const string &name) {
	for (PluginFile &pf: plugin_files) {
		if ((pf.name.replace(" ", "") == name.replace(" ", "")) and (pf.type == type)) {
			Plugin *p = load_and_compile_plugin(type, pf.filename);
			return p;
		}
	}
	session->e(format(_("Can't find %s plugin: %s ..."), Module::category_to_str(type), name));
	return nullptr;
}

Path PluginManager::plugin_dir_static() {
	if (os::app::installed)
		return os::app::directory_static | "plugins";
	return "plugins";
}

Path PluginManager::plugin_dir_local() {
	if (os::app::installed)
		return os::app::directory_dynamic | "plugins";
	return "plugins";
}


Array<string> PluginManager::find_module_sub_types(ModuleCategory type) {
	Array<string> names;
	for (auto &pf: plugin_files)
		if (pf.type == type)
			names.add(pf.name);

	if (type == ModuleCategory::AudioSource) {
		names.add("SongRenderer");
		//names.add("BufferStreamer");
	} else if (type == ModuleCategory::MidiEffect) {
		names.add("Dummy");
	} else if (type == ModuleCategory::BeatSource) {
		//names.add("BarStreamer");
	} else if (type == ModuleCategory::AudioVisualizer) {
		names.add("PeakMeter");
	} else if (type == ModuleCategory::Synthesizer) {
		names.add("Dummy");
		//names.add("Sample");
	} else if (type == ModuleCategory::Stream) {
		names.add("AudioInput");
		names.add("AudioOutput");
		names.add("MidiInput");
	} else if (type == ModuleCategory::Plumbing) {
		names.add("AudioBackup");
		names.add("AudioChannelSelector");
		names.add("AudioJoiner");
		names.add("AudioAccumulator");
		names.add("AudioSucker");
		names.add("BeatMidifier");
		names.add("MidiJoiner");
		names.add("MidiSplitter");
		names.add("MidiAccumulator");
		names.add("MidiSucker");
	} else if (type == ModuleCategory::PitchDetector) {
		names.add("Dummy");
	}
	return names;
}

Array<string> PluginManager::find_module_sub_types_grouped(ModuleCategory type) {
	if ((type == ModuleCategory::AudioEffect) or (type == ModuleCategory::TsunamiPlugin)) {
		Array<string> names;
		for (auto &pf: plugin_files)
			if (pf.type == type)
				names.add(pf.group + "/" + pf.name);
		return names;
	}
	return find_module_sub_types(type);
}

void PluginManager::set_favorite(Session *session, ModuleCategory type, const string &name, bool favorite) {
	presets->set_favorite(session, type, name, favorite);
}

bool PluginManager::is_favorite(Session *session, ModuleCategory type, const string &name) {
	return presets->is_favorite(session, type, name);
}

}

