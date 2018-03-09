/*
 * Session.cpp
 *
 *  Created on: 09.03.2018
 *      Author: michi
 */

#include "Session.h"
#include "TsunamiWindow.h"
#include "Tsunami.h"
#include "Stuff/Log.h"
#include "Storage/Storage.h"
#include "lib/hui/hui.h"

int Session::next_id = 0;
Session *Session::GLOBAL = NULL;

Session::Session()
{
	win = NULL;
	view = NULL;
	_kaba_win = NULL;
	song = NULL;
	storage = new Storage(this);

	log = tsunami->log;
	device_manager = tsunami->device_manager;
	plugin_manager = tsunami->plugin_manager;

	id = next_id ++;
	die_on_plugin_stop = false;
}

Session::~Session()
{
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

