/*
 * Session.cpp
 *
 *  Created on: 09.03.2018
 *      Author: michi
 */

#include "Session.h"
#include "EditModes.h"
#include "Playback.h"
#include "stuff/Log.h"
#include "stuff/SessionManager.h"
#include "storage/Storage.h"
#include "plugins/TsunamiPlugin.h"
#include "data/base.h"
#include "data/Song.h"
#include "lib/hui/hui.h"
#include "module/SignalChain.h"
#include "view/audioview/AudioView.h"
#include "view/bottombar/BottomBar.h"
#include "view/bottombar/MixingConsole.h"
#include "view/TsunamiWindow.h"
#include "view/mainview/MainView.h"

namespace tsunami {

int Session::next_id = 0;
Session *Session::GLOBAL = nullptr;


const string EditMode::Default = "default";
const string EditMode::DefaultSong = "default/song";
const string EditMode::DefaultTrack = "default/track";
const string EditMode::DefaultTrackFx = "default/track/fx";
const string EditMode::DefaultTrackMidiFx = "default/track/midi-fx";
const string EditMode::DefaultTrackSynth = "default/track/synth";
const string EditMode::DefaultFx = "default/fx";
const string EditMode::DefaultMidiFx = "default/midi-fx";
const string EditMode::DefaultSamples = "default/samples";
const string EditMode::DefaultMixing = "default/mixing";
const string EditMode::DefaultMastering = "default/mastering";
const string EditMode::DefaultSampleRef = "default/sample-ref";
const string EditMode::XSignalEditor = ".../signal-editor";
const string EditMode::EditTrack = "edit-track";
const string EditMode::Capture = "capture";
const string EditMode::ScaleBars = "scale-bars";
const string EditMode::ScaleMarker = "scale-marker";
const string EditMode::Curves = "curves";


Session::Session(Log *_log, DeviceManager *_device_manager, PluginManager *_plugin_manager, SessionManager *_session_manager, PerformanceMonitor *_perf_mon) {
	view = nullptr;
	main_view = nullptr;
	_kaba_win = nullptr;
	storage = new Storage(this);

	log = _log;
	device_manager = _device_manager;
	plugin_manager = _plugin_manager;
	session_manager = _session_manager;
	perf_mon = _perf_mon;
	auto_delete = true;

	id = next_id ++;
	die_on_plugin_stop = false;
}

Session::~Session() = default;

void Session::prepare_end() {
	if (persistence_data)
		session_manager->save_session(this);

	for (auto p: weak(plugins))
		p->on_stop();
	plugins.clear();
}

int Session::sample_rate() {
	if (song)
		return song->sample_rate;
	return DEFAULT_SAMPLE_RATE;
}

void Session::set_win(TsunamiWindow *_win) {
	win = _win;
	view = win->view;
	_kaba_win = dynamic_cast<hui::Window*>(win.get());
}

Session *Session::create_child() {
	auto *child = new Session(log, device_manager, plugin_manager, session_manager, perf_mon);
	return child;
}

void Session::i(const string &message) {
	log->info(this, message);
}

void Session::debug(const string &cat, const string &message) {
	log->debug(this, cat + ": " + message);
}

void Session::w(const string &message) {
	log->warn(this, message);
}

void Session::e(const string &message) {
	log->error(this, message);
}

void Session::q(const string &message, const Array<string> &responses) {
	log->question(this, message, responses);
}

void Session::status(const string &message) {
	log->status(this, message);
}

shared<TsunamiPlugin> Session::execute_tsunami_plugin(const string &name, const Array<string> &args) {
	auto *p = CreateTsunamiPlugin(this, name);
	if (!p)
		return nullptr;

	plugins.add(p);
	p->out_stop_request >> create_sink([this, p] {
		on_plugin_stop_request(p);
	});

	p->args = args;
	p->on_start();

	out_add_plugin(p);
	return p;
}

void session_clean_up_unused_signal_chains(Session* s) {
	auto chains = weak(s->all_signal_chains);
	for (auto c: chains)
		if (c->_pointer_ref_counter == 1)
			s->remove_signal_chain(c);
}

void session_destroy_plugin(Session* s, TsunamiPlugin *p) {
	s->out_remove_plugin(p);
	p->on_stop();
	foreachi (auto *pp, weak(s->plugins), i)
		if (p == pp)
			s->plugins.erase(i);

	session_clean_up_unused_signal_chains(s);
}

void Session::on_plugin_stop_request(TsunamiPlugin *p) {
	hui::run_later(0.001f, [this,p] {
		session_destroy_plugin(this, p);
	});

	/*tpl->stop();

	if (die_on_plugin_stop)
		//tsunami->end();//
		hui::RunLater(0.01f, std::bind(&TsunamiWindow::destroy, win));*/
}

void Session::set_mode(const string &_mode) {
	hui::run_later(0.01f, [this, mode = _mode] {
		debug("mode", ">> " + mode);
		if (mode == EditMode::Default) {
			main_view->activate_view(view);
			view->set_mode(mode);
		} else if (mode == EditMode::Capture) {
			main_view->activate_view(view);
			view->set_mode(mode);
		} else if (mode == EditMode::EditTrack) {
			main_view->activate_view(view);
			view->set_mode(mode);
		} else if (mode == EditMode::ScaleBars) {
			main_view->activate_view(view);
			view->set_mode(mode);
		} else if (mode == EditMode::ScaleMarker) {
			main_view->activate_view(view);
			view->set_mode(mode);
		} else if (mode == EditMode::Curves) {
			main_view->activate_view(view);
			view->set_mode(mode);
		} else if (mode == EditMode::DefaultTrack) {
			main_view->activate_view(view);
			view->set_mode(mode);
		} else if (mode == EditMode::DefaultTrackFx) {
			main_view->activate_view(view);
			view->set_mode(mode);
		} else if (mode == EditMode::DefaultTrackMidiFx) {
			main_view->activate_view(view);
			view->set_mode(mode);
		} else if (mode == EditMode::DefaultTrackSynth) {
			main_view->activate_view(view);
			view->set_mode(mode);
		} else if (mode == EditMode::DefaultSong) {
			main_view->activate_view(view);
			view->set_mode(mode);
		} else if (mode == EditMode::DefaultSamples) {
			main_view->activate_view(view);
			view->set_mode(mode);
		} else if (mode == EditMode::DefaultMixing) {
			win->bottom_bar->open(BottomBar::Index::MixingConsole);
		} else if (mode == EditMode::DefaultFx) {
			main_view->activate_view(view);
			view->set_mode(mode);
			/*win->bottom_bar->open(BottomBar::Index::MixingConsole);
			win->bottom_bar->mixing_console->show_fx(view->cur_track());*/
		} else if (mode == EditMode::DefaultMidiFx) {
			main_view->activate_view(view);
			view->set_mode(mode);
			/*view->set_mode(view->mode_default);
			win->bottom_bar->open(BottomBar::Index::MixingConsole);
			win->bottom_bar->mixing_console->show_fx(view->cur_track());*/
		} else if (mode == EditMode::DefaultSampleRef) {
			main_view->activate_view(view);
			view->set_mode(mode);
		} else if (mode == EditMode::XSignalEditor) {
			//view->set_mode(view->mode_default);
			win->bottom_bar->open(BottomBar::Index::SignalChainConsole);
		} else {
			e("unknown mode: " + mode);
			return;
		}
		out_mode_changed();
	});
}

SideBar* Session::side_bar() {
	return win->side_bar.get();
}

BottomBar* Session::bottom_bar() {
	return win->bottom_bar.get();
}

void Session::add_signal_chain(xfer<SignalChain> chain) {
	all_signal_chains.add(chain);
	out_add_signal_chain(chain);
	/*chain->subscribe(this, [this] {
		_remove_signal_chain(chain);
	}, chain->MESSAGE_DELETE);*/
}

shared<SignalChain> Session::create_signal_chain(const string &name) {
	auto chain = new SignalChain(this, name);
	add_signal_chain(chain);
	return chain;
}

shared<SignalChain> Session::create_signal_chain_system(const string &name) {
	auto *chain = new SignalChain(this, name);
	chain->belongs_to_system = true;
	add_signal_chain(chain);
	return chain;
}

shared<SignalChain> Session::load_signal_chain(const Path &filename) {
	auto *chain = SignalChain::load(this, filename);
	add_signal_chain(chain);
	return chain;
}

void Session::remove_signal_chain(SignalChain* chain) {
	shared<SignalChain> temp = chain; // keep alive until after the event
	for (int i=0; i<all_signal_chains.num; i++)
		if (chain == all_signal_chains[i])
			all_signal_chains.erase(i);
	out_remove_signal_chain(chain);
}

}
