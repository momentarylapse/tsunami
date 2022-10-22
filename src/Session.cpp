/*
 * Session.cpp
 *
 *  Created on: 09.03.2018
 *      Author: michi
 */

#include "Session.h"
#include "EditModes.h"
#include "TsunamiWindow.h"
#include "stuff/Log.h"
#include "storage/Storage.h"
#include "plugins/TsunamiPlugin.h"
#include "data/base.h"
#include "data/Song.h"
#include "lib/hui/hui.h"
#include "module/SignalChain.h"
#include "view/audioview/AudioView.h"
#include "view/mode/ViewModeDefault.h"
#include "view/mode/ViewModeCapture.h"
#include "view/mode/ViewModeCurve.h"
#include "view/mode/ViewModeEdit.h"
#include "view/mode/ViewModeEditAudio.h"
#include "view/mode/ViewModeEditMidi.h"
#include "view/mode/ViewModeScaleBars.h"
#include "view/mode/ViewModeScaleMarker.h"
#include "view/sidebar/SideBar.h"
#include "view/sidebar/TrackConsole.h"
#include "view/bottombar/BottomBar.h"
#include "view/bottombar/MixingConsole.h"

int Session::next_id = 0;
Session *Session::GLOBAL = nullptr;

const string Session::MESSAGE_ADD_PLUGIN = "AddPlugin";
const string Session::MESSAGE_REMOVE_PLUGIN = "RemovePlugin";
const string Session::MESSAGE_ADD_SIGNAL_CHAIN = "AddSignalChain";


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


Session::Session(Log *_log, DeviceManager *_device_manager, PluginManager *_plugin_manager, PerformanceMonitor *_perf_mon) {
	view = nullptr;
	_kaba_win = nullptr;
	storage = new Storage(this);

	log = _log;
	device_manager = _device_manager;
	plugin_manager = _plugin_manager;
	perf_mon = _perf_mon;
	auto_delete = true;

	last_plugin = nullptr;

	id = next_id ++;
	die_on_plugin_stop = false;
}

Session::~Session() {
	//msg_write("~Session");
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
	auto *child = new Session(log, device_manager, plugin_manager, perf_mon);
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

TsunamiPlugin *Session::execute_tsunami_plugin(const string &name, const Array<string> &args) {
	auto *p = CreateTsunamiPlugin(this, name);
	if (!p)
		return nullptr;

	plugins.add(p);
	p->subscribe3(this, [this] (VirtualBase *o) {
		on_plugin_stop_request((TsunamiPlugin*)o);
	}, p->MESSAGE_STOP_REQUEST);

	p->args = args;
	p->on_start();

	last_plugin = p;
	notify(MESSAGE_ADD_PLUGIN);
	return p;
}


void Session::on_plugin_stop_request(TsunamiPlugin *p) {
	hui::run_later(0.001f, [this,p]{
		last_plugin = p;
		notify(MESSAGE_REMOVE_PLUGIN);
		p->on_stop();
		foreachi (auto *pp, weak(plugins), i)
			if (p == pp)
				plugins.erase(i);
	});

	/*tpl->stop();

	if (die_on_plugin_stop)
		//tsunami->end();//
		hui::RunLater(0.01f, std::bind(&TsunamiWindow::destroy, win));*/
}

void Session::set_mode(const string &mode) {
	debug("mode", ">> " + mode);
	if (mode == EditMode::Default) {
		view->set_mode(view->mode_default);
		win->side_bar->_hide();
	} else if (mode == EditMode::Capture) {
		view->set_mode(view->mode_capture);
	} else if (mode == EditMode::EditTrack) {
		view->set_mode(view->mode_edit);
	} else if (mode == EditMode::ScaleBars) {
		view->set_mode(view->mode_scale_bars);
	} else if (mode == EditMode::ScaleMarker) {
		view->set_mode(view->mode_scale_marker);
	} else if (mode == EditMode::Curves) {
		view->set_mode(view->mode_curve);
	} else if (mode == EditMode::DefaultTrack) {
		view->set_mode(view->mode_default);
		win->side_bar->open(SideBar::TRACK_CONSOLE);
	} else if (mode == EditMode::DefaultTrackFx) {
		view->set_mode(view->mode_default);
		win->side_bar->open(SideBar::TRACK_CONSOLE);
		win->side_bar->track_console->set_mode(TrackConsole::Mode::FX);
	} else if (mode == EditMode::DefaultTrackMidiFx) {
		view->set_mode(view->mode_default);
		win->side_bar->open(SideBar::TRACK_CONSOLE);
		win->side_bar->track_console->set_mode(TrackConsole::Mode::MIDI_FX);
	} else if (mode == EditMode::DefaultTrackSynth) {
		view->set_mode(view->mode_default);
		win->side_bar->open(SideBar::TRACK_CONSOLE);
		win->side_bar->track_console->set_mode(TrackConsole::Mode::SYNTH);
	} else if (mode == EditMode::DefaultSong) {
		view->set_mode(view->mode_default);
		win->side_bar->open(SideBar::SONG_CONSOLE);
	} else if (mode == EditMode::DefaultSamples) {
		view->set_mode(view->mode_default);
		win->side_bar->open(SideBar::SAMPLE_CONSOLE);
	} else if (mode == EditMode::DefaultMixing) {
		view->set_mode(view->mode_default);
		win->bottom_bar->open(BottomBar::MIXING_CONSOLE);
	} else if (mode == EditMode::DefaultFx) {
		view->set_mode(view->mode_default);
		win->bottom_bar->open(BottomBar::MIXING_CONSOLE);
		win->bottom_bar->mixing_console->show_fx(view->cur_track());
	} else if (mode == EditMode::DefaultMidiFx) {
		view->set_mode(view->mode_default);
		win->bottom_bar->open(BottomBar::MIXING_CONSOLE);
		win->bottom_bar->mixing_console->show_fx(view->cur_track());
	} else if (mode == EditMode::DefaultSampleRef) {
		view->set_mode(view->mode_default);
		win->side_bar->open(SideBar::SAMPLEREF_CONSOLE);
	} else if (mode == EditMode::XSignalEditor) {
		//view->set_mode(view->mode_default);
		win->bottom_bar->open(BottomBar::SIGNAL_EDITOR);
	} else {
		e("unknown mode: " + mode);
		return;
	}
	hui::run_later(0.1f, [=]{ win->update_menu(); });
	this->mode = mode;
}

bool Session::in_mode(const string &m) {
	return mode == m;
}

void Session::add_signal_chain(SignalChain *chain) {
	all_signal_chains.add(chain);
	notify(MESSAGE_ADD_SIGNAL_CHAIN);
	/*chain->subscribe(this, [=]{
		_remove_signal_chain(chain);
	}, chain->MESSAGE_DELETE);*/
}

SignalChain* Session::create_signal_chain(const string &name) {
	auto *chain = new SignalChain(this, name);
	add_signal_chain(chain);
	return chain;
}

SignalChain* Session::create_signal_chain_system(const string &name) {
	auto *chain = new SignalChain(this, name);
	chain->belongs_to_system = true;
	add_signal_chain(chain);
	return chain;
}

SignalChain* Session::load_signal_chain(const Path &filename) {
	auto *chain = SignalChain::load(this, filename);
	add_signal_chain(chain);
	return chain;
}

void Session::remove_signal_chain(SignalChain* chain) {
	for (int i=0; i<all_signal_chains.num; i++)
		if (chain == all_signal_chains[i])
			all_signal_chains.erase(i);
}
