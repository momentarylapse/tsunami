/*
 * BarsConsole.h
 *
 *  Created on: 21.10.2015
 *      Author: michi
 */

#ifndef BARSCONSOLE_H_
#define BARSCONSOLE_H_


#include "SideBar.h"
#include "../../Stuff/Observer.h"
class BarList;
class Song;
class AudioView;

class BarsConsole: public SideBarConsole, public Observer
{
public:
	BarsConsole(Song *song, AudioView *view);
	virtual ~BarsConsole();

	void onEditSong();

	virtual void onUpdate(Observable *o, const string &message);

	BarList *bar_list;
};

#endif /* BARSCONSOLE_H_ */
