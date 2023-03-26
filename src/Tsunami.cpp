/*
 * Tsunami.cpp
 *
 *  Created on: 13.08.2014
 *      Author: michi
 */

#include "Tsunami.h"
#include "lib/base/pointer.h"
#include "lib/os/CommandLineParser.h"
#include "data/base.h"
#include "data/Song.h"
#include "module/SignalChain.h"
#include "module/audio/AudioEffect.h"
#include "module/audio/SongRenderer.h"
#include "module/audio/PeakMeter.h"
#include "module/midi/MidiEffect.h"
#include "module/synthesizer/Synthesizer.h"
#include "view/TsunamiWindow.h"
#include "Session.h"
#include "EditModes.h"
#include "storage/Storage.h"
#include "stuff/Log.h"
#include "stuff/Clipboard.h"
#include "stuff/PerformanceMonitor.h"
#include "stuff/BackupManager.h"
#include "stuff/SessionManager.h"
#include "stuff/ErrorHandler.h"
#include "plugins/PluginManager.h"
#include "plugins/TsunamiPlugin.h"
#include "device/DeviceManager.h"
#include "command/song/Diff.h"
#include "command/song/Show.h"
#include "test/TestRingBuffer.h"
#ifndef NDEBUG
#include "module/ModuleFactory.h"
#include "view/module/ConfigPanel.h"
#include "view/module/ConfigurationDialog.h"
#endif

const string AppName = "Tsunami";
const string AppVersion = "0.7.111.0";
const string AppNickname = "absolute 2er0";

Tsunami *tsunami = nullptr;

bool ugly_hack_slow = false;


Tsunami::Tsunami() :
	hui::Application("tsunami", "English", hui::FLAG_NO_ERROR_HANDLER)
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
	set_property("copyright", "© 2007-2023 by Michael Ankele");
	set_property("author", "Michael Ankele <michi@lupina.de>;2er0;Benji <mail@benji.is>");
	//set_property("designer", "Michael Ankele <michi@lupina.de>;2er0;Benji <mail@benji.is>");
	set_property("documenter", "no one :P");
}

Tsunami::~Tsunami() {
	//msg_write("~Tsunami");
}

void Tsunami::on_end() {
	//msg_write("Tsunami on end");
	session_manager = nullptr;
}

bool Tsunami::on_startup(const Array<string> &arg) {
	tsunami = this;

	ErrorHandler::init();

	session_manager = new SessionManager;

	perf_mon = new PerformanceMonitor;

	log = new Log;

	Session::GLOBAL = new Session(log.get(), nullptr, nullptr, perf_mon.get());

	clipboard = new Clipboard;

	device_manager = new DeviceManager(Session::GLOBAL);
	Session::GLOBAL->device_manager = device_manager.get();

	// create (link) PluginManager after all other components are ready
	plugin_manager = new PluginManager;
	Session::GLOBAL->plugin_manager = plugin_manager.get();

	plugin_manager->link_app_data();

	if (!handle_arguments(arg))
		return false;

	return true;
}

extern bool module_config_debug;

bool Tsunami::handle_arguments(const Array<string> &args) {
	Session *session = Session::GLOBAL;

	string chain_file;
	string plugin_file;
	bool allow_window = false;
	auto flags = Storage::Flags::NONE;

	CommandLineParser p;
	p.info("tsunami", AppName + " - the ultimate audio editor");//AppName + " " + AppVersion);
	p.option("--slow", "", [] {
		ugly_hack_slow = true;
	});
	p.option("--mcd", "", [] {
		module_config_debug = true;
	});
	p.option("--force", "", [&flags] {
		flags = flags | Storage::Flags::FORCE;
	});
	p.option("--plugin", "FILE", "add a plugin to run", [&plugin_file] (const string &a) {
		plugin_file = a;
	});
	p.option("--chain", "FILE", "add a signal chain", [&chain_file] (const string &a) {
		chain_file = a;
	});
	p.option("--params", "PARAMS", "set loading parameters", [] (const string &a) {
		Storage::options_in = a;
	});


	p.cmd("", "[FILE]", "open a window and (optionally) load a file", [this, &session, &allow_window] (const Array<string> &a) {
		Session::GLOBAL->i(format("%s %s \"%s\"", AppName, AppVersion, AppNickname));
		Session::GLOBAL->i(_("  ...don't worry. Everything will be fine!"));

		device_manager->init();
		session = session_manager->create_session();

		session->win->show();
		if (a.num > 0) {
			session->storage->load(session->song.get(), a[0]);
		} else {
			// new file
			session->song->add_track(SignalType::AUDIO_MONO);

			// default tags
			session->song->add_tag("title", _("New Audio File"));
			session->song->add_tag("album", AppName);
			session->song->add_tag("artist", hui::config.get_str("DefaultArtist", AppName));
			session->song->reset_history();

			session->song->notify(session->song->MESSAGE_FINISHED_LOADING);
		}
		allow_window = true;
	});
	p.cmd("help", "", "show this help page", [&p] (const Array<string> &) {
		p.show();
	});
	p.cmd("info", "FILE1 ...", "show information about the file", [session, &flags] (const Array<string> &a) {
		session->storage->allow_gui = false;
		//session->log->allow_console_output = false;
		Song* song = new Song(session, DEFAULT_SAMPLE_RATE);
		session->song = song;
		flags = flags | Storage::Flags::ONLY_METADATA;
		for (string &filename: a)
			if (session->storage->load_ex(song, filename, flags))
				show_song(song);
		delete song;
	});
	p.cmd("diff", "FILE1 FILE2", "compare 2 files", [session, &flags] (const Array<string> &a) {
		session->storage->allow_gui = false;
		session->log->allow_console_output = false;
		Song* song1 = new Song(session, DEFAULT_SAMPLE_RATE);
		Song* song2 = new Song(session, DEFAULT_SAMPLE_RATE);
		session->song = song1;
		flags = flags | Storage::Flags::ONLY_METADATA;
		if (session->storage->load_ex(song1, a[0], flags)) {
			if (session->storage->load_ex(song2, a[1], flags)) {
				auto r = diff_song(song1, song2);
				if (r.num > 0)
					msg_error("diffs: " + str(r));
			}
		}
		delete song1;
		delete song2;
	});
	p.cmd("export", "FILE_IN FILE_OUT", "convert a file", [session, &flags] (const Array<string> &a) {
		session->storage->allow_gui = false;
		Song* song = new Song(session, DEFAULT_SAMPLE_RATE);
		session->song = song;
		if (session->storage->load(song, a[0]) or (flags & Storage::Flags::FORCE))
			session->storage->_export(song, a[1]);
		delete song;
	});
	p.cmd("execute", "PLUGIN ...", "just run a plugin", [this, &session] (const Array<string> &a) {
		device_manager->init();
		session = session_manager->create_session();
		session->win->hide();
		session->die_on_plugin_stop = true;
		session->execute_tsunami_plugin(a[0], a.sub_ref(1));
	});
	p.cmd("session", "SESSION ...", "restore a saved session", [this, &session] (const Array<string> &a) {
		session_manager->load_session(a[0]);
	});
#ifndef NDEBUG
	p.cmd("test list", "", "debug: list internal unit tests", [] (const Array<string> &) {
		UnitTest::print_all_names();
	});
	p.cmd("test run", "FILTER", "debug: run internal unit tests", [] (const Array<string> &a) {
		UnitTest::run_all(a[0]);
	});
	p.cmd("previewgui", "TYPE NAME", "debug: show the config gui of a plugin", [this, &session] (const Array<string> &a) {
		session = session_manager->create_session();
		session->win->hide();
		Module *m = nullptr;
		if (a[0] == "fx") {
			m = ModuleFactory::create(session, ModuleCategory::AUDIO_EFFECT, a[1]);
			configure_module(session->win.get(), m);
		} else if (a[0] == "mfx") {
			m = ModuleFactory::create(session, ModuleCategory::MIDI_EFFECT, a[1]);
			configure_module(session->win.get(), m);
		} else if (a[0] == "synth") {
			m = ModuleFactory::create(session, ModuleCategory::SYNTHESIZER, a[1]);
			configure_module(session->win.get(), m);
		} else if (a[0] == "vis") {
			m = CreateAudioVisualizer(session, a[1]);
			auto *dlg = new hui::Window("", 800, 600);
			dlg->add_grid("", 0, 0, "root");
			ConfigPanel::_hidden_parent_ = dlg;
			auto *p = m->create_panel();
			dlg->embed(p, "root", 0,0);
			hui::fly(dlg);
		} else {
			msg_error("unknown type: " + a[0] + " (try fx|mfx|synth|vis)");
		}
	});
	p.cmd("listview", "", "bla", [] (const Array<string> &a) {
		auto dlg = new hui::Window("ListView", 800, 600);
		dlg->from_source("ListView list 'a'");
		dlg->add_string("list", "0");
		dlg->add_string("list", "1");
		dlg->add_string("list", "2");
		if (a == Array<string>({"long"}))
			for (int i=3; i<1000; i++)
				dlg->add_string("list", i2s(i));
		hui::run(dlg);
	});
	p.cmd("columnview", "", "bla", [] (const Array<string> &) {
		auto dlg = new hui::Window("ColumnView", 800, 600);
		dlg->from_source("Grid ? ''\n\tListView list 'a\\b\\c' format=CmT\n\tButton button 'x'");
		dlg->add_string("list", "true\\<b>test</b>\\x");
		dlg->add_string("list", "false\\<i>more test</i>\\y");
		dlg->event_x("list", "hui:change", [dlg] {
			auto e = hui::get_event();
			msg_write(format("edit: %d %d  %s", e->row, e->column, dlg->get_cell("list", e->row, e->column)));
		});
		dlg->event("button", [dlg] {
			dlg->change_string("list", 0, "false\\bla\\z");
		});
		hui::run(dlg);
	});
	p.cmd("treeview", "", "bla", [] (const Array<string> &a) {
		auto dlg = new hui::Window("TreeView", 400, 600);
		dlg->from_source("TreeView list 'a'");
		dlg->add_string("list", "0");
		dlg->add_string("list", "1");
		dlg->add_string("list", "2");
		for (int i=3; i<30; i++)
			dlg->add_child_string("list", i % 3, i2s(i));
		hui::run(dlg);
	});
#endif
	p.parse(args);

	Storage::options_in = "";
	Storage::options_out = "";



	if (plugin_file != "")
		session->execute_tsunami_plugin(plugin_file);


	if (chain_file != "") {
		session->add_signal_chain(SignalChain::load(session, chain_file));
		session->set_mode(EditMode::XSignalEditor);
	}


	return allow_window;
}

void Tsunami::load_key_codes() {
}


void Tsunami::test_allow_termination(hui::Callback cb_yes, hui::Callback cb_no) {
	bool allowed = true;

	for (auto *s: weak(session_manager->sessions)) {
		s->win->test_allow_termination([] {}, [&allowed] { allowed = false; });
		if (!allowed)
			break;
	}

	if (allowed)
		cb_yes();
	else
		cb_no();
}

HUI_EXECUTE(Tsunami);
