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
#include "Stuff/PerformanceMonitor.h"
#include "Stuff/BackupManager.h"
#include "Plugins/PluginManager.h"
#include "Plugins/TsunamiPlugin.h"
#include "Device/DeviceManager.h"
#include "Stream/AudioOutput.h"
#include "Test/TestRingBuffer.h"


const string AppName = "Tsunami";
const string AppVersion = "0.7.91.0";
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
	set_property("copyright", "© 2007-2019 by Michael Ankele");
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

	Session::GLOBAL->i(AppName + " " + AppVersion + " \"" + AppNickname + "\"");
	Session::GLOBAL->i(_("  ...don't worry. Everything will be fine!"));

	device_manager = new DeviceManager;
	Session::GLOBAL->device_manager = device_manager;

	// create (link) PluginManager after all other components are ready
	plugin_manager = new PluginManager;
	Session::GLOBAL->plugin_manager = plugin_manager;

	plugin_manager->link_app_script_data();

	if (handle_arguments(arg))
		return false;

	// ok, full window mode

	BackupManager::check_old_files(Session::GLOBAL);


	// create a window and load file
	if (sessions.num == 0) {
		Session *session = create_session();
		session->song->add_track(SignalType::AUDIO_MONO);

		// default tags
		session->song->add_tag("title", _("New Audio File"));
		session->song->add_tag("album", AppName);
		session->song->add_tag("artist", hui::Config.get_str("DefaultArtist", AppName));
		session->song->reset_history();

		session->song->notify(session->song->MESSAGE_FINISHED_LOADING);
		session->win->show();
	}

	return true;
}

bool Tsunami::handle_arguments(Array<string> &args) {
	if (args.num < 2)
		return false;
	Session *session = Session::GLOBAL;

	for (int i=1; i<args.num; i++) {

	if (args[i] == "--help") {
		session->i(AppName + " " + AppVersion);
		session->i("--help");
		session->i("--info <FILE>");
		session->i("--export <FILE-IN> <FILE-OUT>");
		session->i("--execute <PLUGIN-NAME> [ARGUMENTS]");
		session->i("--chain <SIGNAL-CHAIN-FILE>");
#ifndef NDEBUG
		session->i("--run-tests <FILTER>");
#endif
		return true;
	} else if (args[i] == "--info") {
		Song* song = new Song(session, DEFAULT_SAMPLE_RATE);
		session->song = song;
		if (args.num < i+2){
			session->e(_("call: tsunami --info <FILE>"));
		} else if (session->storage->load_ex(song, args[i+1], true)) {
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
		delete song;
		return true;
	} else if (args[i] == "--export") {
		Song* song = new Song(session, DEFAULT_SAMPLE_RATE);
		session->song = song;
		if (args.num < i + 3) {
			session->e(_("call: tsunami --export <FILE-IN> <FILE-OUT>"));
		} else if (session->storage->load(song, args[i+1])) {
			session->storage->save(song, args[i+2]);
		}
		delete song;
		return true;
	} else if (args[1] == "--execute") {
		if (args.num < i + 2) {
			session->e(_("call: tsunami --execute <PLUGIN-NAME> [ARGUMENTS]"));
			return true;
		}
		if (session == Session::GLOBAL)
			session = create_session();
		session->win->hide();
		session->die_on_plugin_stop = true;
		session->execute_tsunami_plugin(args[i+1]);
		i ++;
		//return false;
	} else if (args[i] == "--chain") {
		if (args.num < i + 2) {
			session->e(_("call: tsunami --chain <SIGNAL-CHAIN>"));
			return true;
		}
		if (session == Session::GLOBAL)
			session = create_session(args[i+1]);
		session->win->show();
		i ++;
	} else if (args[i] == "--slow") {
		ugly_hack_slow = true;
#ifndef NDEBUG
	} else if (args[i] == "--run-tests") {
		if (args.num > i + 1)
			UnitTest::run_all(args[i+1]);
		else
			UnitTest::print_all_names();
		return true;
#endif
	} else if (args[i].head(2) == "--") {
		session->e(_("unknown command: ") + args[i]);
		return true;
	} else {
		if (session == Session::GLOBAL)
			session = create_session();
		session->win->show();
		session->storage->load(session->song, args[i]);
	}
	}
	return false;
}

Session* Tsunami::create_session(const string &chain_filename) {
	Session *session = new Session(log, device_manager, plugin_manager, perf_mon);

	session->song = new Song(session, DEFAULT_SAMPLE_RATE);

	if (chain_filename.num > 0) {
		session->signal_chain = SignalChain::load(session, chain_filename);
		session->all_signal_chains.add(session->signal_chain);
	} else {
		session->signal_chain = session->add_signal_chain_system("playback");
	}
	session->signal_chain->create_default_modules();
	session->song_renderer = (SongRenderer*)session->signal_chain->get_by_type(ModuleType::AUDIO_SOURCE, "SongRenderer");
	session->peak_meter = (PeakMeter*)session->signal_chain->get_by_type(ModuleType::AUDIO_VISUALIZER, "PeakMeter");
	session->output_stream = (AudioOutput*)session->signal_chain->get_by_type(ModuleType::STREAM, "AudioOutput");
	session->signal_chain->mark_all_modules_as_system();

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
