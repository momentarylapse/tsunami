/*
 * DummyEditorConsole.cpp
 *
 *  Created on: May 2, 2020
 *      Author: michi
 */

#include "DummyEditorConsole.h"
#include "../../Session.h"


DummyEditorConsole::DummyEditorConsole(Session *session) :
	SideBarConsole(_("Edit"), session)
{
	from_resource("dummy-editor");

	event("edit_track", [=]{ session->set_mode("default/track"); });
	event("edit_song", [=]{ session->set_mode("default/song"); });
}
