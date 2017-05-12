/*
 * MidiFxConsole.h
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#ifndef MIDIFXCONSOLE_H_
#define MIDIFXCONSOLE_H_

#include "SideBar.h"
#include "../../Stuff/Observer.h"
#include "../../lib/math/math.h"

class Song;
class Track;
class AudioView;

class MidiFxConsole : public SideBarConsole, public Observer
{
public:
	MidiFxConsole(AudioView *view, Song *audio);
	virtual ~MidiFxConsole();

	virtual void onUpdate(Observable *o, const string &message);
	void update();

	void clear();
	void setTrack(Track *t);

	void onAdd();

	void onEditSong();
	void onEditTrack();
	void onEditMidi();

	string id_inner;

	AudioView *view;
	Track *track;
	Song *song;
	Array<hui::Panel*> panels;
};

#endif /* MIDIFXCONSOLE_H_ */
