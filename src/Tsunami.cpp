/*
 * Tsunami.cpp
 *
 *  Created on: 13.08.2014
 *      Author: michi
 */

#include "Tsunami.h"

#include "Device/DeviceManager.h"
#include "TsunamiWindow.h"
#include "Session.h"
#include "Storage/Storage.h"
#include "Stuff/Log.h"
#include "Stuff/Clipboard.h"
#include "Stuff/PerformanceMonitor.h"
#include "Stuff/BackupManager.h"
#include "Plugins/PluginManager.h"
#include "Plugins/TsunamiPlugin.h"


const string AppName = "Tsunami";
const string AppVersion = "0.7.3.0";
const string AppNickname = "absolute 2er0";

Tsunami *tsunami = NULL;

Tsunami::Tsunami() :
	hui::Application("tsunami", "English", hui::FLAG_LOAD_RESOURCE)
{
	device_manager = NULL;
	log = NULL;
	clipboard = NULL;
	plugin_manager = NULL;

	setProperty("name", AppName);
	setProperty("version", AppVersion + " \"" + AppNickname + "\"");
	setProperty("comment", _("Editor for audio files"));
	setProperty("website", "http://michi.is-a-geek.org/software");
	setProperty("copyright", "© 2007-2018 by Michael Ankele");
	setProperty("author", "Michael Ankele <michi@lupina.de>");
}

Tsunami::~Tsunami()
{
	delete(device_manager);
	delete(plugin_manager);
	delete(clipboard);
	delete(log);
}

bool Tsunami::onStartup(const Array<string> &arg)
{
	tsunami = this;

	PerformanceMonitor::init();

	log = new Log;

	clipboard = new Clipboard;

	device_manager = new DeviceManager;

	// create (link) PluginManager after all other components are ready
	plugin_manager = new PluginManager;

	Session::GLOBAL = new Session();

	plugin_manager->LinkAppScriptData();

	if (handleCLIArguments(arg))
		return false;

	// ok, full window mode

	BackupManager::check_old_files(Session::GLOBAL);


	// create a window and load file
	//if (!win){
		Session *session = createSession();
		session->win->show();

		if (arg.num >= 2)
			session->storage->load(session->song, arg[1]);
	//}
	return true;
}

bool Tsunami::handleCLIArguments(const Array<string> &args)
{
	if (args.num < 2)
		return false;
	Session *session = Session::GLOBAL;

	if (args[1] == "--help"){
		session->i(AppName + " " + AppVersion);
		session->i("--help");
		session->i("--info <FILE>");
		session->i("--export <FILE_IN> <FILE_OUT>");
		session->i("--execute <PLUGIN_NAME> [ARGUMENTS]");
		return true;
	}else if (args[1] == "--info"){
		Song* song = new Song(session);
		session->song = song;
		if (args.num < 3){
			session->e(_("call: tsunami --info <File>"));
		}else if (session->storage->load(song, args[2])){
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
		Song* song = new Song(session);
		session->song = song;
		if (args.num < 4){
			session->e(_("call: tsunami --export <File> <Exportfile>"));
		}else if (session->storage->load(song, args[2])){
			session->storage->save(song, args[3]);
		}
		delete(song);
		return true;
	/*}else if (args[1] == "--execute"){
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
		return false;*/
	}else if (args[1].head(2) == "--"){
		session->e(_("unknown command: ") + args[1]);
		return true;
	}
	return false;
}

Session* Tsunami::createSession()
{
	Session *session = new Session();

	session->i(AppName + " " + AppVersion + " \"" + AppNickname + "\"");
	session->i(_("  ...don't worry. Everything will be fine!"));

	session->song = new Song(session);

	session->setWin(new TsunamiWindow(session));
	session->win->auto_delete = true;
	session->win->show();

	sessions.add(session);
	return session;
}

void Tsunami::loadKeyCodes()
{
}

bool Tsunami::allowTermination()
{
	for (auto *s: sessions)
		if (!s->win->allowTermination())
			return false;
	return true;
}

HUI_EXECUTE(Tsunami);
