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
#include "Audio/AudioInput.h"
#include "Audio/AudioRenderer.h"
#include "View/Helper/Progress.h"
#include "Plugins/PluginManager.h"


string AppName = "Tsunami";
string AppVersion = "0.6.4.1";

Tsunami *tsunami = NULL;

Tsunami::Tsunami() :
	HuiApplication("tsunami", "Deutsch", HUI_FLAG_LOAD_RESOURCE)
{
	HuiSetProperty("name", AppName);
	HuiSetProperty("version", AppVersion);
	HuiSetProperty("comment", _("Editor f&ur Audio Dateien"));
	HuiSetProperty("website", "http://michi.is-a-geek.org/software");
	HuiSetProperty("copyright", "© 2007-2014 by MichiSoft TM");
	HuiSetProperty("author", "Michael Ankele <michi@lupina.de>");
}

Tsunami::~Tsunami()
{
	delete(storage);
	delete(output);
	delete(input);
	delete(audio);
	delete(renderer);
	delete(plugin_manager);
}

bool Tsunami::onStartup(const Array<string> &arg)
{
	tsunami = this;
	win = NULL;
	_win = NULL;
	_view = NULL;

	progress = new Progress;
	log = new Log;

	clipboard = new Clipboard;

	output = new AudioOutput;
	input = new AudioInput;
	renderer = new AudioRenderer;

	audio = new AudioFile;
	audio->NewWithOneTrack(Track::TYPE_AUDIO, DEFAULT_SAMPLE_RATE);

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
		storage->Load(audio, _arg[1]);
}

bool Tsunami::HandleArguments(const Array<string> &arg)
{
	if (arg.num < 2)
		return true;
	if (arg[1] == "--info"){
		if (arg.num < 3){
			log->Error(_("Aufruf: tsunami --info <Datei.nami>"));
		}else if (storage->Load(audio, arg[2])){
			msg_write(format("sample-rate: %d", audio->sample_rate));
			msg_write(format("samples: %d", audio->GetRange().num));
			msg_write("length: " + audio->get_time_str(audio->GetRange().num));
			foreach(Tag &t, audio->tag)
				msg_write("tag: " + t.key + " = " + t.value);
		}
		return false;
	}else if (arg[1] == "--export"){
		if (arg.num < 4){
			log->Error(_("Aufruf: tsunami --export <Datei.nami> <Exportdatei>"));
		}else if (storage->Load(audio, arg[2])){
			storage->Export(audio, audio->GetRange(), arg[3]);
		}
		return false;
	}
	return true;
}

void Tsunami::CreateWindow()
{
	log->Info(AppName + " " + AppVersion);
	log->Info(_("  ...keine Sorge, das wird schon!"));

	win = new TsunamiWindow;
	_win = dynamic_cast<HuiWindow*>(win);
	_view = win->view;
	plugin_manager->AddPluginsToMenu(win);

	win->Show();
	//HuiRunLaterM(0.01f, win, &TsunamiWindow::OnViewOptimal);
}

void Tsunami::LoadKeyCodes()
{
}

bool Tsunami::AllowTermination()
{
	if (win)
		return win->AllowTermination();
	return true;
}

HuiExecute(Tsunami);
