/*
 * DummyEditorConsole.cpp
 *
 *  Created on: May 2, 2020
 *      Author: michi
 */

#include "DummyEditorConsole.h"
#include "../../Session.h"
#include "../../EditModes.h"


DummyEditorConsole::DummyEditorConsole(Session *session, SideBar *bar) :
	SideBarConsole(_("Editor"), "dummy-editor-console", session, bar)
{
	from_resource("dummy-editor");

	event("edit_track", [session] { session->set_mode(EditMode::DefaultTrack); });
	event("edit_song", [session] { session->set_mode(EditMode::DefaultSong); });
}
