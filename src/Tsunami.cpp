/*
 * Tsunami.cpp
 *
 *  Created on: 13.08.2014
 *      Author: michi
 */

#include "Tsunami.h"

#include "Data/base.h"
#include "Data/Song.h"
#include "Data/Track.h"
#include "Data/TrackLayer.h"
#include "Module/SignalChain.h"
#include "Module/Audio/SongRenderer.h"
#include "Module/Audio/PeakMeter.h"
#include "TsunamiWindow.h"
#include "Session.h"
#include "Storage/Storage.h"
#include "Stuff/Log.h"
#include "Stuff/Clipboard.h"
#include "Stuff/CLIParser.h"
#include "Stuff/PerformanceMonitor.h"
#include "Stuff/BackupManager.h"
#include "Plugins/PluginManager.h"
#include "Plugins/TsunamiPlugin.h"
#include "Device/DeviceManager.h"
#include "Device/Stream/AudioOutput.h"
#include "Test/TestRingBuffer.h"
#ifndef NDEBUG
#include "Module/Audio/AudioEffect.h"
#include "Module/ConfigPanel.h"
#endif


const string AppName = "Tsunami";
const string AppVersion = "0.7.96.2";
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
	set_property("copyright", "© 2007-2020 by Michael Ankele");
	set_property("author", "Michael Ankele <michi@lupina.de>");
}

Tsunami::~Tsunami() {
	delete device_manager;
	delete plugin_manager;
	delete clipboard;
	delete log;
	delete perf_mon;
}

void Tsunami::on_end() {
	while (sessions.num > 0) {
		Session *s = sessions.pop();
		delete s;
	}
}

bool Tsunami::on_startup(const Array<string> &_arg) {
	Array<string> arg = _arg;
	tsunami = this;

	perf_mon = new PerformanceMonitor;

	log = new Log;

	Session::GLOBAL = new Session(log, nullptr, nullptr, perf_mon);

	clipboard = new Clipboard;

	device_manager = new DeviceManager(Session::GLOBAL);
	Session::GLOBAL->device_manager = device_manager;

	// create (link) PluginManager after all other components are ready
	plugin_manager = new PluginManager;
	Session::GLOBAL->plugin_manager = plugin_manager;

	plugin_manager->link_app_script_data();

	if (!handle_arguments(arg))
		return false;

	return true;
}

bool Tsunami::handle_arguments(Array<string> &args) {
	Session *session = Session::GLOBAL;

	string chain_file;
	string plugin_file;
	bool allow_window = false;

	CLIParser p;
	p.info("tsunami", AppName + " - the ultimate audio editor");//AppName + " " + AppVersion);
	p.option("--slow", [&]{ ugly_hack_slow = true; });
	p.option("--plugin", "FILE", [&](const string &a){ plugin_file = a; });
	p.option("--chain", "FILE", [&](const string &a){ chain_file = a; });


	p.mode("", {"[FILE]"}, [&](const Array<string> &a){
		Session::GLOBAL->i(AppName + " " + AppVersion + " \"" + AppNickname + "\"");
		Session::GLOBAL->i(_("  ...don't worry. Everything will be fine!"));

		device_manager->init();
		session = create_session();

		session->win->show();
		if (a.num > 0) {
			session->storage->load(session->song, a[0]);
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
	p.mode("--help", {}, [&](const Array<string> &){
		p.show();
	});
	p.mode("--info", {"FILE1", "..."}, [&](const Array<string> &a){
		Song* song = new Song(session, DEFAULT_SAMPLE_RATE);
		session->song = song;
		for (string &filename: a) {
			if (session->storage->load_ex(song, filename, true)) {
				msg_write(format("sample-rate: %d", song->sample_rate));
				msg_write(format("samples: %d", song->range().length));
				msg_write("length: " + song->get_time_str(song->range().length));
				msg_write(format("tracks: %d", song->tracks.num));
				int n = 0;
				for (Track *t: song->tracks)
					for (TrackLayer *l: t->layers)
						n += l->samples.num;
				msg_write(format("refs: %d / %d", n, song->samples.num));
				for (Tag &t: song->tags)
					msg_write("tag: " + t.key + " = " + t.value);
			}
		}
		delete song;
	});
	p.mode("--export", {"FILE_IN", "FILE_OUT"}, [&](const Array<string> &a){
		Song* song = new Song(session, DEFAULT_SAMPLE_RATE);
		session->song = song;
		if (session->storage->load(song, a[0])) {
			session->storage->save(song, a[1]);
		}
		delete song;
	});
	p.mode("--execute", {"PLUGIN"}, [&](const Array<string> &a){
		device_manager->init();
		session = create_session();
		session->win->hide();
		session->die_on_plugin_stop = true;
		session->execute_tsunami_plugin(a[0]);
	});
#ifndef NDEBUG
	p.mode("--list-tests", {}, [&](const Array<string> &){
		UnitTest::print_all_names();
	});
	p.mode("--run-tests", {"FILTER"}, [&](const Array<string> &a){
		UnitTest::run_all(a[0]);
	});
	p.mode("--preview-gui", {"TYPE", "NAME"}, [&](const Array<string> &a){
		session = create_session();
		session->win->hide();
		Module *m = nullptr;
		if (a[0] == "fx") {
			m = CreateAudioEffect(session, a[1]);
			configure_module(session->win, m);
		} else if (a[0] == "vis") {
			m = CreateAudioVisualizer(session, a[1]);
			auto *p = m->create_panel();
			auto *dlg = new hui::Window("", 800, 600);
			dlg->add_grid("", 0, 0, "root");
			dlg->embed(p, "root", 0,0);
			dlg->run();
		}
	});
#endif
	p.parse(args);



	if (plugin_file != "")
		session->execute_tsunami_plugin(plugin_file);


	if (chain_file != "")
		session->add_signal_chain(SignalChain::load(session, chain_file));


	return allow_window;
}

Session* Tsunami::create_session() {
	Session *session = new Session(log, device_manager, plugin_manager, perf_mon);

	session->song = new Song(session, DEFAULT_SAMPLE_RATE);

	session->set_win(new TsunamiWindow(session));
	session->win->auto_delete = true;
	session->win->show();

	sessions.add(session);
	return session;
}

void Tsunami::load_key_codes() {
}

bool Tsunami::allow_termination() {
	for (auto *s: sessions)
		if (!s->win->allow_termination())
			return false;
	return true;
}

HUI_EXECUTE(Tsunami);
