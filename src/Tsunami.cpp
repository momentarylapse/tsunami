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
#include "Plugins/PluginManager.h"

#include "lib/image/ImagePainter.h"


string AppName = "Tsunami";
string AppVersion = "0.6.39.2";

Tsunami *tsunami = NULL;

Tsunami::Tsunami() :
	HuiApplication("tsunami", "English", HUI_FLAG_LOAD_RESOURCE)
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

	HuiSetProperty("name", AppName);
	HuiSetProperty("version", AppVersion);
	HuiSetProperty("comment", _("Editor for audio files"));
	HuiSetProperty("website", "http://michi.is-a-geek.org/software");
	HuiSetProperty("copyright", "© 2007-2016 by Michael Ankele");
	HuiSetProperty("author", "Michael Ankele <michi@lupina.de>");
}

Tsunami::~Tsunami()
{
	delete(storage);
	delete(device_manager);
	delete(plugin_manager);
}

bool Tsunami::onStartup(const Array<string> &arg)
{
	tsunami = this;
	win = NULL;
	_win = NULL;
	_view = NULL;
	song = NULL;

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

	createWindow(arg);
	return true;
}

bool Tsunami::handleCLIArguments(const Array<string> &arg)
{
	if (arg.num < 2)
		return false;
	if (arg[1] == "--help"){
		log->info(AppName + " " + AppVersion);
		log->info("--help");
		log->info("--info <FILE>");
		log->info("--export <FILE_IN> <FILE_OUT>");
		return true;
	}else if (arg[1] == "--info"){
		song = new Song;
		if (arg.num < 3){
			log->error(_("Call: tsunami --info <File>"));
		}else if (storage->load(song, arg[2])){
			msg_write(format("sample-rate: %d", song->sample_rate));
			msg_write(format("samples: %d", song->getRange().length));
			msg_write("length: " + song->get_time_str(song->getRange().length));
			msg_write(format("tracks: %d", song->tracks.num));
			for (Tag &t : song->tags)
				msg_write("tag: " + t.key + " = " + t.value);
		}
		delete(song);
		return true;
	}else if (arg[1] == "--export"){
		song = new Song;
		if (arg.num < 4){
			log->error(_("Call: tsunami --export <File> <Exportfile>"));
		}else if (storage->load(song, arg[2])){
			storage->save(song, arg[3]);
		}
		delete(song);
		return true;
	}
	return false;
}

void Tsunami::createWindow(const Array<string> &arg)
{
	win = new TsunamiWindow;
	_win = dynamic_cast<HuiWindow*>(win);
	_view = win->view;
	song = win->song;

	win->show();


	if (arg.num >= 2)
		tsunami->storage->load(win->song, arg[1]);
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

HuiExecute(Tsunami);
