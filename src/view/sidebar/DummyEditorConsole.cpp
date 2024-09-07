/*
 * DummyEditorConsole.cpp
 *
 *  Created on: May 2, 2020
 *      Author: michi
 */

#include "DummyEditorConsole.h"
#include "../../lib/hui/language.h"

namespace tsunami {

DummyEditorConsole::DummyEditorConsole(Session *session, SideBar *bar) :
	SideBarConsole(_("Editor"), "dummy-editor-console", session, bar)
{
	from_resource("dummy-editor");
}

}
