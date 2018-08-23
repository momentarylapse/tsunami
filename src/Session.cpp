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

int Session::next_id = 0;
Session *Session::GLOBAL = nullptr;

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

void Session::setWin(TsunamiWindow *_win)
{
	win = _win;
	view = win->view;
	_kaba_win = dynamic_cast<hui::Window*>(win);
}

void Session::i(const string &message)
{
	log->info(this, message);
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

void Session::executeTsunamiPlugin(const string& name)
{
	/*for (TsunamiPlugin *p: plugins)
		if (p->name == name){
			if (p->active)
				p->stop();
			else
				p->start();
			return;
		}*/

	TsunamiPlugin *p = CreateTsunamiPlugin(this, name);

	plugins.add(p);
	p->subscribe3(this, std::bind(&Session::onPluginStopRequest, this, std::placeholders::_1), p->MESSAGE_STOP_REQUEST);

	p->on_start();
}


void Session::onPluginStopRequest(VirtualBase *o)
{
	TsunamiPlugin *p = (TsunamiPlugin*)o;
	msg_write("stop request..." + p2s(p));

	hui::RunLater(0.001f, [this,p]{
		msg_write("stop " + p2s(p));
		p->on_stop();
		foreachi (auto *pp, plugins, i)
			if (p == pp)
				plugins.erase(i);
		msg_write("del");
		delete p;
	});

	/*tpl->stop();

	if (die_on_plugin_stop)
		//tsunami->end();//
		hui::RunLater(0.01f, std::bind(&TsunamiWindow::destroy, win));*/
}
