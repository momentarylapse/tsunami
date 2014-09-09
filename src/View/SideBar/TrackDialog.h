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

	void LoadData();
	void ApplyData();

	void OnName();
	void OnVolume();
	void OnPanning();
	void OnSynthesizer();
	void OnConfigSynthesizer();
	void OnEditMidi();

	void SetTrack(Track *t);

	virtual void OnUpdate(Observable *o, const string &message);

	AudioView *view;
	Track *track;
	BarList *bar_list;
};

#endif /* TRACKDIALOG_H_ */
