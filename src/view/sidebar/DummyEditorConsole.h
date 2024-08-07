/*
 * DummyEditorConsole.h
 *
 *  Created on: May 2, 2020
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_DUMMYEDITORCONSOLE_H_
#define SRC_VIEW_SIDEBAR_DUMMYEDITORCONSOLE_H_

#include "SideBar.h"

namespace tsunami {

class TrackLayer;

class DummyEditorConsole : public SideBarConsole {
public:
	DummyEditorConsole(Session *session, SideBar *bar);
};

}

#endif /* SRC_VIEW_SIDEBAR_DUMMYEDITORCONSOLE_H_ */
