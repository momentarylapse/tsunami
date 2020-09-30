/*
 * DummyEditorConsole.cpp
 *
 *  Created on: May 2, 2020
 *      Author: michi
 */

#include "DummyEditorConsole.h"
#include "../../Session.h"
#include "../../EditModes.h"


DummyEditorConsole::DummyEditorConsole(Session *session) :
	SideBarConsole(_("Editor"), session)
{
	from_resource("dummy-editor");

	event("edit_track", [=]{ session->set_mode(EditMode::DefaultTrack); });
	event("edit_song", [=]{ session->set_mode(EditMode::DefaultSong); });
}
