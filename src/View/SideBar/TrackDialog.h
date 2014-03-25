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

class TrackDialog: public SideBarConsole, public Observer
{
public:
	TrackDialog();
	virtual ~TrackDialog();

	void LoadData();
	void ApplyData();

	void OnName();
	void OnSynthesizer();
	void OnConfigSynthesizer();
	void OnEditMidiTrack();

	void SetTrack(Track *t);

	virtual void OnUpdate(Observable *o, const string &message);

	Track *track;
	BarList *bar_list;
};

#endif /* TRACKDIALOG_H_ */
