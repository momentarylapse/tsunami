/*
 * MidiFxConsole.h
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#ifndef MIDIFXCONSOLE_H_
#define MIDIFXCONSOLE_H_

#include "BottomBar.h"
#include "../../Stuff/Observer.h"
#include "../../lib/math/math.h"

class Song;
class AudioView;

class MidiFxConsole : public BottomBarConsole, public Observer
{
public:
	MidiFxConsole(AudioView *view, Song *audio);
	virtual ~MidiFxConsole();

	virtual void onUpdate(Observable *o, const string &message);
	void update();

	void clear();
	void setTrack(Track *t);

	void onAdd();


	string id_inner;

	AudioView *view;
	Track *track;
	Song *audio;
	Array<HuiPanel*> panels;
};

#endif /* MIDIFXCONSOLE_H_ */
