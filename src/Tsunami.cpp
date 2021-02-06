/*
 * Tsunami.cpp
 *
 *  Created on: 13.08.2014
 *      Author: michi
 */

#include "Tsunami.h"
#include "lib/base/pointer.h"

#include "Data/base.h"
#include "Data/Song.h"
#include "Data/Track.h"
#include "Data/TrackLayer.h"
#include "Module/SignalChain.h"
#include "Module/Audio/AudioEffect.h"
#include "Module/Audio/SongRenderer.h"
#include "Module/Audio/PeakMeter.h"
#include "Module/Midi/MidiEffect.h"
#include "Module/Synth/Synthesizer.h"
#include "TsunamiWindow.h"
#include "Session.h"
#include "Storage/Storage.h"
#include "Stuff/Log.h"
#include "Stuff/Clipboard.h"
#include "Stuff/CLIParser.h"
#include "Stuff/PerformanceMonitor.h"
#include "Stuff/BackupManager.h"
#include "Stuff/Diff.h"
#include "Plugins/PluginManager.h"
#include "Plugins/TsunamiPlugin.h"
#include "Device/DeviceManager.h"
#include "Device/Stream/AudioOutput.h"
#include "Test/TestRingBuffer.h"
#ifndef NDEBUG
#include "Module/ModuleFactory.h"
#include "Module/ConfigPanel.h"
#endif


#include "Data/Sample.h"
#include "Data/SampleRef.h"

const string AppName = "Tsunami";
const string AppVersion = "0.7.101.5";
const string AppNickname = "absolute 2er0";

Tsunami *tsunami = nullptr;

bool ugly_hack_slow = false;


Tsunami::Tsunami() :
	hui::Application("tsunami", "English", hui::FLAG_LOAD_RESOURCE)
{
	device_manager = nullptr;
	log = nullptr;
	clipboard = nullptr;
	plugin_manager = nullptr;
	perf_mon = nullptr;

	set_property("name", AppName);
	set_property("version", AppVersion + " \"" + AppNickname + "\"");
	set_property("comment", _("Editor for audio files"));
	set_property("website", "http://michi.is-a-geek.org/software");
	set_property("copyright", "© 2007-2021 by Michael Ankele");
	set_property("author", "Michael Ankele <michi@lupina.de>");
}

Tsunami::~Tsunami() {
	//msg_write("~Tsunami");
}

void Tsunami::on_end() {
	//msg_write("Tsunami on end");
	sessions.clear();
}

bool Tsunami::on_startup(const Array<string> &arg) {
	tsunami = this;

	perf_mon = new PerformanceMonitor;

	log = new Log;

	Session::GLOBAL = new Session(log.get(), nullptr, nullptr, perf_mon.get());

	clipboard = new Clipboard;

	device_manager = new DeviceManager(Session::GLOBAL);
	Session::GLOBAL->device_manager = device_manager.get();

	// create (link) PluginManager after all other components are ready
	plugin_manager = new PluginManager;
	Session::GLOBAL->plugin_manager = plugin_manager.get();

	plugin_manager->link_app_script_data();

	if (!handle_arguments(arg))
		return false;

	return true;
}

void show_song(Song *song) {
	msg_write(format("  sample-rate: %d", song->sample_rate));
	msg_write(format("  samples: %d", song->range().length));
	msg_write("  length: " + song->get_time_str(song->range().length));
	msg_write(format("  bars: %d", song->bars.num));
	msg_write(format("  tracks: %d", song->tracks.num));
	int n = 0;
	for (Track *t: weak(song->tracks)) {
		msg_write(format("  track '%s'", t->nice_name()));
		msg_write("    type: " + signal_type_name(t->type));
		if (t->type == SignalType::MIDI) {
			msg_write(format("    synth: %s v%d", t->synth->module_subtype, t->synth->version()));
		}
		for (TrackLayer *l: weak(t->layers)) {
			msg_write("    layer");
			if (l->buffers.num > 0)
				msg_write(format("      buffers: %d", l->buffers.num));
			if (l->midi.num > 0)
				msg_write(format("      notes: %d", l->midi.num));
			if (l->samples.num > 0)
				msg_write(format("      sample-refs: %d", l->samples.num));
			if (l->markers.num > 0)
				msg_write(format("      markers: %d", l->markers.num));
			n += l->samples.num;
		}
		for (auto *fx: weak(t->fx))
			msg_write(format("    fx: %s v%d", fx->module_subtype, fx->version()));
		for (auto *fx: weak(t->midi_fx))
			msg_write(format("    midifx: %s v%d", fx->module_subtype, fx->version()));
	}
	msg_write(format("  refs: %d / %d", n, song->samples.num));
	for (Tag &t: song->tags)
		msg_write(format("  tag: %s = %s", t.key, t.value));
}

extern bool module_config_debug;

bool Tsunami::handle_arguments(const Array<string> &args) {
	Session *session = Session::GLOBAL;

	string chain_file;
	string plugin_file;
	bool allow_window = false;
	bool force = false;

	CLIParser p;
	p.info("tsunami", AppName + " - the ultimate audio editor");//AppName + " " + AppVersion);
	p.flag("--slow", "", [&]{ ugly_hack_slow = true; });
	p.flag("--mcd", "", [&]{ module_config_debug = true; });
	p.flag("--force", "", [&]{ force = true; });
	p.option("--plugin", "FILE", "add a plugin to run", [&](const string &a){ plugin_file = a; });
	p.option("--chain", "FILE", "add a signal chain", [&](const string &a){ chain_file = a; });
	p.option("--params", "PARAMS", "set loading parameters", [&](const string &a){ Storage::options_in = a; });


	p.mode("", {"[FILE]"}, "open a window and (optionally) load a file", [&](const Array<string> &a) {
		Session::GLOBAL->i(format("%s %s \"%s\"", AppName, AppVersion, AppNickname));
		Session::GLOBAL->i(_("  ...don't worry. Everything will be fine!"));

		device_manager->init();
		session = create_session();

		session->win->show();
		if (a.num > 0) {
			session->storage->load(session->song.get(), a[0]);
		} else {
			// new file
			session->song->add_track(SignalType::AUDIO_MONO);

			// default tags
			session->song->add_tag("title", _("New Audio File"));
			session->song->add_tag("album", AppName);
			session->song->add_tag("artist", hui::Config.get_str("DefaultArtist", AppName));
			session->song->reset_history();

			session->song->notify(session->song->MESSAGE_FINISHED_LOADING);
		}
		BackupManager::check_old_files(Session::GLOBAL);
		allow_window = true;
	});
	p.mode("--help", {}, "show this info", [&](const Array<string> &) {
		p.show_info();
	});
	p.mode("--info", {"FILE1", "..."}, "show information about the file", [&](const Array<string> &a) {
		//session->log->allow_console_output = false;
		Song* song = new Song(session, DEFAULT_SAMPLE_RATE);
		session->song = song;
		for (string &filename: a)
			if (session->storage->load_ex(song, filename, true) or force)
				show_song(song);
		delete song;
	});
	p.mode("--diff", {"FILE1", "FILE2"}, "compare 2 files", [&](const Array<string> &a) {
		session->log->allow_console_output = false;
		Song* song1 = new Song(session, DEFAULT_SAMPLE_RATE);
		Song* song2 = new Song(session, DEFAULT_SAMPLE_RATE);
		session->song = song1;
		if (session->storage->load_ex(song1, a[0], true) or force)
			if (session->storage->load_ex(song2, a[1], true) or force) {
				auto r = diff_song(song1, song2);
				if (r.num > 0)
					msg_error("diffs: " + sa2s(r));
			}

		delete song1;
		delete song2;
	});
	p.mode("--export", {"FILE_IN", "FILE_OUT"}, "convert a file", [&](const Array<string> &a) {
		Song* song = new Song(session, DEFAULT_SAMPLE_RATE);
		session->song = song;
		if (session->storage->load(song, a[0]) or force)
			session->storage->save(song, a[1]);
		delete song;
	});
	p.mode("--execute", {"PLUGIN", "..."}, "just run a plugin", [&](const Array<string> &a) {
		device_manager->init();
		session = create_session();
		session->win->hide();
		session->die_on_plugin_stop = true;
		session->execute_tsunami_plugin(a[0], a.sub(1, -1));
	});
#ifndef NDEBUG
	p.mode("--list-tests", {}, "debug: list internal unit tests", [&](const Array<string> &) {
		UnitTest::print_all_names();
	});
	p.mode("--run-tests", {"FILTER"}, "debug: run internal unit tests", [&](const Array<string> &a) {
		UnitTest::run_all(a[0]);
	});
	p.mode("--preview-gui", {"TYPE", "NAME"}, "debug: show the config gui of a plugin", [&](const Array<string> &a) {
		session = create_session();
		session->win->hide();
		Module *m = nullptr;
		if (a[0] == "fx") {
			m = ModuleFactory::create(session, ModuleType::AUDIO_EFFECT, a[1]);
			configure_module(session->win.get(), m);
		} else if (a[0] == "mfx") {
			m = ModuleFactory::create(session, ModuleType::MIDI_EFFECT, a[1]);
			configure_module(session->win.get(), m);
		} else if (a[0] == "synth") {
			m = ModuleFactory::create(session, ModuleType::SYNTHESIZER, a[1]);
			configure_module(session->win.get(), m);
		} else if (a[0] == "vis") {
			m = CreateAudioVisualizer(session, a[1]);
			auto *p = m->create_panel();
			auto *dlg = new hui::Window("", 800, 600);
			dlg->add_grid("", 0, 0, "root");
			dlg->embed(p, "root", 0,0);
			dlg->run();
		} else {
			msg_error("unknown type: " + a[0] + " (try fx|mfx|synth|vis)");
		}
	});
#endif
	p.parse(args);

	Storage::options_in = "";
	Storage::options_out = "";



	if (plugin_file != "")
		session->execute_tsunami_plugin(plugin_file);


	if (chain_file != "")
		session->add_signal_chain(SignalChain::load(session, chain_file));


	return allow_window;
}

Session* Tsunami::create_session() {
	Session *session = new Session(log.get(), device_manager.get(), plugin_manager.get(), perf_mon.get());

	session->song = new Song(session, DEFAULT_SAMPLE_RATE);

	session->set_win(new TsunamiWindow(session));
	session->win->show();

	sessions.add(session);
	return session;
}

void Tsunami::load_key_codes() {
}

bool Tsunami::allow_termination() {
	for (auto *s: weak(sessions))
		if (!s->win->allow_termination())
			return false;
	return true;
}

HUI_EXECUTE(Tsunami);
