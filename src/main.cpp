/*
 * main.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"
#include "Tsunami.h"

string AppName = "Tsunami";
string AppVersion = "0.5.4.0";


int hui_main(Array<string> arg)
{
	HuiInit("tsunami", true, "Deutsch");
	HuiSetProperty("name", AppName);
	HuiSetProperty("version", AppVersion);
	HuiSetProperty("comment", _("Editor f&ur Audio Dateien"));
	HuiSetProperty("website", "http://michi.is-a-geek.org/software");
	HuiSetProperty("copyright", "© 2007-2014 by MichiSoft TM");
	HuiSetProperty("author", "Michael Ankele <michi@lupina.de>");

	msg_db_r("main",1);

	tsunami = new Tsunami(arg);
	return tsunami->Run();
}

