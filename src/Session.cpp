/*
 * Session.cpp
 *
 *  Created on: 09.03.2018
 *      Author: michi
 */

#include "Session.h"
#include "TsunamiWindow.h"
#include "Stuff/Log.h"
#include "Storage/Storage.h"
#include "Plugins/TsunamiPlugin.h"
#include "Data/base.h"
#include "Data/Song.h"
#include "lib/hui/hui.h"
#include "Module/SignalChain.h"
#include "View/AudioView.h"
#include "View/Mode/ViewModeDefault.h"
#include "View/Mode/ViewModeCapture.h"
#include "View/Mode/ViewModeCurve.h"
#include "View/Mode/ViewModeMidi.h"
#include "View/Mode/ViewModeScaleBars.h"
#include "View/Mode/ViewModeScaleMarker.h"
#include "View/SideBar/SideBar.h"
#include "View/BottomBar/BottomBar.h"
#include "View/BottomBar/MixingConsole.h"

int Session::next_id = 0;
Session *Session::GLOBAL = nullptr;

const string Session::MESSAGE_ADD_PLUGIN = "AddPlugin";
const string Session::MESSAGE_REMOVE_PLUGIN = "RemovePlugin";

Session::Session(Log *_log, DeviceManager *_device_manager, PluginManager *_plugin_manager, PerformanceMonitor *_perf_mon)
{
	win = nullptr;
	view = nullptr;
	_kaba_win = nullptr;
	song = nullptr;
	storage = new Storage(this);

	signal_chain = nullptr;
	song_renderer = nullptr;
	peak_meter = nullptr;
	output_stream = nullptr;

	log = _log;
	device_manager = _device_manager;
	plugin_manager = _plugin_manager;
	perf_mon = _perf_mon;

	last_plugin = nullptr;

	id = next_id ++;
	die_on_plugin_stop = false;
}

Session::~Session()
{
	if (signal_chain)
		delete(signal_chain);
	if (song)
		delete(song);
	delete(storage);
}

int Session::sample_rate()
{
	if (song)
		return song->sample_rate;
	return DEFAULT_SAMPLE_RATE;
}

void Session::set_win(TsunamiWindow *_win)
{
	win = _win;
	view = win->view;
	_kaba_win = dynamic_cast<hui::Window*>(win);
}

Session *Session::create_child()
{
	Session *child = new Session(log, device_manager, plugin_manager, perf_mon);
	return child;
}

void Session::i(const string &message)
{
	log->info(this, message);
}

void Session::debug(const string &cat, const string &message)
{
	log->debug(this, cat + ": " + message);
}

void Session::w(const string &message)
{
	log->warn(this, message);
}

void Session::e(const string &message)
{
	log->error(this, message);
}

void Session::q(const string &message, const Array<string> &responses)
{
	log->question(this, message, responses);
}

void Session::execute_tsunami_plugin(const string& name)
{
	TsunamiPlugin *p = CreateTsunamiPlugin(this, name);
	if (!p)
		return;

	plugins.add(p);
	p->subscribe3(this, [=](VirtualBase *o){ on_plugin_stop_request((TsunamiPlugin*)o); }, p->MESSAGE_STOP_REQUEST);

	p->on_start();

	last_plugin = p;
	notify(MESSAGE_ADD_PLUGIN);
}


void Session::on_plugin_stop_request(TsunamiPlugin *p)
{
	hui::RunLater(0.001f, [this,p]{
		last_plugin = p;
		notify(MESSAGE_REMOVE_PLUGIN);
		p->on_stop();
		foreachi (auto *pp, plugins, i)
			if (p == pp)
				plugins.erase(i);
		delete p;
	});

	/*tpl->stop();

	if (die_on_plugin_stop)
		//tsunami->end();//
		hui::RunLater(0.01f, std::bind(&TsunamiWindow::destroy, win));*/
}

void Session::set_mode(const string &mode)
{
	debug("mode", ">> " + mode);
	if (mode == "default"){
		view->set_mode(view->mode_default);
		win->side_bar->_hide();
	}else if (mode == "capture"){
		view->set_mode(view->mode_capture);
		win->side_bar->open(SideBar::CAPTURE_CONSOLE);
	}else if (mode == "midi"){
		view->set_mode(view->mode_midi);
		win->side_bar->open(SideBar::MIDI_EDITOR_CONSOLE);
	}else if (mode == "scale-bars"){
		view->set_mode(view->mode_scale_bars);
		win->side_bar->_hide();
	}else if (mode == "scale-marker"){
		view->set_mode(view->mode_scale_marker);
		win->side_bar->_hide();
	}else if (mode == "curves"){
		view->set_mode(view->mode_curve);
		win->side_bar->open(SideBar::CURVE_CONSOLE);
	}else if (mode == "default/track"){
		view->set_mode(view->mode_default);
		win->side_bar->open(SideBar::TRACK_CONSOLE);
	}else if (mode == "default/song"){
		view->set_mode(view->mode_default);
		win->side_bar->open(SideBar::SONG_CONSOLE);
	}else if (mode == "default/samples"){
		view->set_mode(view->mode_default);
		win->side_bar->open(SideBar::SAMPLE_CONSOLE);
	}else if (mode == "default/mixing"){
		view->set_mode(view->mode_default);
		win->bottom_bar->open(BottomBar::MIXING_CONSOLE);
		win->bottom_bar->mixing_console->set_mode(MixerMode::VOLUME);
	}else if (mode == "default/fx"){
		view->set_mode(view->mode_default);
		win->bottom_bar->open(BottomBar::MIXING_CONSOLE);
		win->bottom_bar->mixing_console->set_mode(MixerMode::EFFECTS);
	}else if (mode == "default/midi-fx"){
		view->set_mode(view->mode_default);
		win->bottom_bar->open(BottomBar::MIXING_CONSOLE);
		win->bottom_bar->mixing_console->set_mode(MixerMode::MIDI_EFFECTS);
	}else if (mode == "default/synth"){
		view->set_mode(view->mode_default);
		win->side_bar->open(SideBar::SYNTH_CONSOLE);
	}else if (mode == "default/sample-ref"){
		view->set_mode(view->mode_default);
		win->side_bar->open(SideBar::SAMPLEREF_CONSOLE);
	}else{
		e("unknown mode: " + mode);
		return;
	}
	hui::RunLater(0.1f, [=]{ win->update_menu(); });
	this->mode = mode;
}

bool Session::in_mode(const string &m)
{
	return mode == m;
}
