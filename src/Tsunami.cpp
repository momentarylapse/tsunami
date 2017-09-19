/*
 * Tsunami.cpp
 *
 *  Created on: 13.08.2014
 *      Author: michi
 */

#include "Tsunami.h"

#include "Device/DeviceManager.h"
#include "TsunamiWindow.h"
#include "Storage/Storage.h"
#include "Stuff/Log.h"
#include "Stuff/Clipboard.h"
#include "Stuff/PerformanceMonitor.h"
#include "Plugins/PluginManager.h"
#include "Plugins/TsunamiPlugin.h"


const string AppName = "Tsunami";
const string AppVersion = "0.6.100.0 alpha!";

Tsunami *tsunami = NULL;

Tsunami::Tsunami() :
	hui::Application("tsunami", "English", hui::FLAG_LOAD_RESOURCE)
{
	song = NULL;
	_view = NULL;
	device_manager = NULL;
	log = NULL;
	_win = NULL;
	clipboard = NULL;
	win = NULL;
	plugin_manager = NULL;
	storage = NULL;

	setProperty("name", AppName);
	setProperty("version", AppVersion);
	setProperty("comment", _("Editor for audio files"));
	setProperty("website", "http://michi.is-a-geek.org/software");
	setProperty("copyright", "© 2007-2017 by Michael Ankele");
	setProperty("author", "Michael Ankele <michi@lupina.de>");
}

Tsunami::~Tsunami()
{
	delete(storage);
	delete(device_manager);
	delete(plugin_manager);
	delete(clipboard);
	delete(log);
}

bool Tsunami::onStartup(const Array<string> &arg)
{
	tsunami = this;
	win = NULL;
	_win = NULL;
	_view = NULL;
	song = NULL;

	PerformanceMonitor::init();

	log = new Log;

	clipboard = new Clipboard;

	device_manager = new DeviceManager;

	storage = new Storage;

	// create (link) PluginManager after all other components are ready
	plugin_manager = new PluginManager;

	if (handleCLIArguments(arg))
		return false;

	// ok, full window mode

	log->info(AppName + " " + AppVersion);
	log->info(_("  ...don't worry. Everything will be fine!"));


	// create a window and load file
	if (!win){
		createWindow();
		win->show();

		if (arg.num >= 2)
			storage->load(win->song, arg[1]);
	}
	return true;
}

bool Tsunami::handleCLIArguments(const Array<string> &args)
{
	if (args.num < 2)
		return false;
	if (args[1] == "--help"){
		log->info(AppName + " " + AppVersion);
		log->info("--help");
		log->info("--info <FILE>");
		log->info("--export <FILE_IN> <FILE_OUT>");
		log->info("--execute <PLUGIN_NAME> [ARGUMENTS]");
		return true;
	}else if (args[1] == "--info"){
		song = new Song;
		if (args.num < 3){
			log->error(_("call: tsunami --info <File>"));
		}else if (storage->load(song, args[2])){
			msg_write(format("sample-rate: %d", song->sample_rate));
			msg_write(format("samples: %d", song->getRange().length));
			msg_write("length: " + song->get_time_str(song->getRange().length));
			msg_write(format("tracks: %d", song->tracks.num));
			int n = 0;
			for (Track *t: song->tracks)
				n += t->samples.num;
			msg_write(format("refs: %d / %d", n, song->samples.num));
			for (Tag &t: song->tags)
				msg_write("tag: " + t.key + " = " + t.value);
		}
		delete(song);
		return true;
	}else if (args[1] == "--export"){
		song = new Song;
		if (args.num < 4){
			log->error(_("call: tsunami --export <File> <Exportfile>"));
		}else if (storage->load(song, args[2])){
			storage->save(song, args[3]);
		}
		delete(song);
		return true;
	}else if (args[1] == "--execute"){
		if (args.num < 3){
			log->error(_("call: tsunami --execute <PLUGIN_NAME> [ARGUMENTS]"));
			return true;
		}
		createWindow();
		win->hide();
		win->die_on_plugin_stop = true;
		TsunamiPlugin *p = CreateTsunamiPlugin(args[2], win);
		p->args = args.sub(3, -1);
		win->plugins.add(p);
		p->subscribe3(win, std::bind(&TsunamiWindow::onPluginStopRequest, win, std::placeholders::_1), p->MESSAGE_STOP_REQUEST);
		p->start();
		return false;
	}else if (args[1].head(2) == "--"){
		log->error(_("unknown command: ") + args[1]);
		return true;
	}
	return false;
}

void Tsunami::createWindow()
{
	win = new TsunamiWindow(this);
	win->auto_delete = true;
	_win = dynamic_cast<hui::Window*>(win);
	_view = win->view;
	song = win->song;
}

void Tsunami::loadKeyCodes()
{
}

bool Tsunami::allowTermination()
{
	if (win)
		return win->allowTermination();
	return true;
}

HUI_EXECUTE(Tsunami);
