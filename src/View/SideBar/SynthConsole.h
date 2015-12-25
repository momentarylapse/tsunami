/*
 * SynthConsole.h
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#ifndef SYNTHCONSOLE_H_
#define SYNTHCONSOLE_H_

#include "SideBar.h"
#include "../../Stuff/Observer.h"

class Track;
class AudioView;

class SynthConsole : public SideBarConsole, public Observer
{
public:
	SynthConsole(AudioView *view);
	virtual ~SynthConsole();

	void clear();
	void setTrack(Track *t);

	void onSelect();
	void onDetune();

	void onEditSong();
	void onEditTrack();

	virtual void onUpdate(Observable *o, const string &message);

	string id_inner;

	HuiPanel *panel;

	AudioView *view;
	Track *track;
};

#endif /* SYNTHCONSOLE_H_ */
