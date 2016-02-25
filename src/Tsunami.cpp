/*
 * Tsunami.cpp
 *
 *  Created on: 13.08.2014
 *      Author: michi
 */

#include "Tsunami.h"
#include "TsunamiWindow.h"
#include "Storage/Storage.h"
#include "Stuff/Log.h"
#include "Stuff/Clipboard.h"
#include "Audio/AudioOutput.h"
#include "Plugins/PluginManager.h"


string AppName = "Tsunami";
string AppVersion = "0.6.34.0";

Tsunami *tsunami = NULL;

Tsunami::Tsunami() :
	HuiApplication("tsunami", "English", HUI_FLAG_LOAD_RESOURCE)
{
	song = NULL;
	_view = NULL;
	output = NULL;
	log = NULL;
	_win = NULL;
	clipboard = NULL;
	win = NULL;
	plugin_manager = NULL;
	storage = NULL;

	HuiSetProperty("name", AppName);
	HuiSetProperty("version", AppVersion);
	HuiSetProperty("comment", _("Editor f&ur Audio Dateien"));
	HuiSetProperty("website", "http://michi.is-a-geek.org/software");
	HuiSetProperty("copyright", "© 2007-2016 by Michael Ankele");
	HuiSetProperty("author", "Michael Ankele <michi@lupina.de>");
}

Tsunami::~Tsunami()
{
	delete(storage);
	delete(output);
	delete(song);
	delete(plugin_manager);
}

bool Tsunami::onStartup(const Array<string> &arg)
{
	tsunami = this;
	win = NULL;
	_win = NULL;
	_view = NULL;

	log = new Log;

	log->info(AppName + " " + AppVersion);
	log->info(_("  ...keine Sorge, das wird schon!"));

	clipboard = new Clipboard;

	output = new AudioOutput;

	song = new Song;
	song->newWithOneTrack(Track::TYPE_AUDIO, DEFAULT_SAMPLE_RATE);

	storage = new Storage;

	// create (link) PluginManager after all other components are ready
	plugin_manager = new PluginManager;

	if (!HandleArguments(arg))
		return false;

	CreateWindow();
	_arg = arg;
	HuiRunLaterM(0.01f, this, &Tsunami::_HandleArguments);
	return true;
}

void Tsunami::_HandleArguments()
{
	if (_arg.num >= 2)
		storage->load(song, _arg[1]);
	song->notify(song->MESSAGE_NEW);
}

bool Tsunami::HandleArguments(const Array<string> &arg)
{
	if (arg.num < 2)
		return true;
	if (arg[1] == "--info"){
		if (arg.num < 3){
			log->error(_("Aufruf: tsunami --info <Datei>"));
		}else if (storage->load(song, arg[2])){
			msg_write(format("sample-rate: %d", song->sample_rate));
			msg_write(format("samples: %d", song->getRange().num));
			msg_write("length: " + song->get_time_str(song->getRange().num));
			msg_write(format("tracks: %d", song->tracks.num));
			foreach(Tag &t, song->tags)
				msg_write("tag: " + t.key + " = " + t.value);
		}
		return false;
	}else if (arg[1] == "--export"){
		if (arg.num < 4){
			log->error(_("Aufruf: tsunami --export <Datei> <Exportdatei>"));
		}else if (storage->load(song, arg[2])){
			storage->save(song, arg[3]);
		}
		return false;
	}
	return true;
}

void Tsunami::CreateWindow()
{
	win = new TsunamiWindow;
	_win = dynamic_cast<HuiWindow*>(win);
	_view = win->view;
	plugin_manager->AddPluginsToMenu(win);

	win->show();
	//HuiRunLaterM(0.01f, win, &TsunamiWindow::OnViewOptimal);
}

void Tsunami::LoadKeyCodes()
{
}

bool Tsunami::AllowTermination()
{
	if (win)
		return win->allowTermination();
	return true;
}

HuiExecute(Tsunami);
