/*
 * TrackDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef TRACKDIALOG_H_
#define TRACKDIALOG_H_


#include "SideBar.h"
#include "../../Stuff/Observer.h"
class Track;
class Slider;
class BarList;
class AudioView;

class TrackDialog: public SideBarConsole, public Observer
{
public:
	TrackDialog(AudioView *view);
	virtual ~TrackDialog();

	void loadData();
	void applyData();

	void onName();
	void onVolume();
	void onPanning();
	void onSynthesizer();
	void onConfigSynthesizer();
	void onEditMidi();

	void setTrack(Track *t);

	virtual void onUpdate(Observable *o, const string &message);

	AudioView *view;
	Track *track;
	BarList *bar_list;
};

#endif /* TRACKDIALOG_H_ */
