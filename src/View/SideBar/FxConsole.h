/*
 * FxConsole.h
 *
 *  Created on: 20.03.2014
 *      Author: michi
 */

#ifndef FXCONSOLE_H_
#define FXCONSOLE_H_

#include "SideBar.h"

class Track;
class Song;
class AudioView;
class Session;

class FxConsole : public SideBarConsole
{
public:
	FxConsole(Session *session);
	virtual ~FxConsole();

	void clear();
	void setTrack(Track *t);

	void onAdd();

	void onEditSong();
	void onEditTrack();

	void onTrackDelete();
	void onViewCurTrackChange();
	void onUpdate();

	string id_inner;

	Track *track;
	Array<hui::Panel*> panels;
};

#endif /* FXCONSOLE_H_ */
