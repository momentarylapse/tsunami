/*
 * MidiFxConsole.h
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#ifndef MIDIFXCONSOLE_H_
#define MIDIFXCONSOLE_H_

#include "SideBar.h"

class Track;

class MidiFxConsole : public SideBarConsole
{
public:
	MidiFxConsole(Session *session);
	virtual ~MidiFxConsole();

	void onViewCurTrackChange();
	void onTrackDelete();
	void onUpdate();
	void update();

	void clear();
	void setTrack(Track *t);

	void onAdd();

	void onEditSong();
	void onEditTrack();
	void onEditMidi();

	string id_inner;

	Track *track;
	Array<hui::Panel*> panels;
};

#endif /* MIDIFXCONSOLE_H_ */
