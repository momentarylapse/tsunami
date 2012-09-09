/*
 * main.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"
#include "Tsunami.h"

string AppName = "Tsunami";
string AppVersion = "0.3.6.0 alpha";

#define iforeach(_array_, _it_) \
	for(typeof((_array_).begin()) _it_ = (_array_).begin(); _it_.valid(); _it_ ++)
#define iforeachi(_array_, _it_, _i_) \
	for(typeof((_array_).begin()) _it_ = (_array_).begin(); _it_.valid(); _it_ ++) \
		for (int _i_ = _it_.get_index(); _i_ >= 0; _i_ = -1)


int hui_main(Array<string> arg)
{
	HuiInitExtended("tsunami", AppName + " " + AppVersion, NULL, true, "Deutsch");
	HuiSetProperty("name", AppName);
	HuiSetProperty("version", AppVersion);
	HuiSetProperty("comment", _("Editor f&ur Audio Dateien"));
	HuiSetProperty("website", "http://michi.is-a-geek.org/michisoft");
	HuiSetProperty("copyright", "© 2007-2012 by MichiSoft TM");
	HuiSetProperty("author", "Michael Ankele <michi@lupina.de>");

	msg_db_r("main",1);
	msg_write(AppName + " " + AppVersion);
	msg_write("");


	tsunami = new Tsunami(arg);
	return tsunami->Run();
}

