/*
 * TrackDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef TRACKDIALOG_H_
#define TRACKDIALOG_H_


#include "../Helper/EmbeddedDialog.h"
#include "../../Stuff/Observer.h"
class Track;
class Slider;
class FxList;
class BarList;

class TrackDialog: public EmbeddedDialog, public Observer
{
public:
	TrackDialog(HuiWindow *win);
	virtual ~TrackDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();
	void OnName();
	void OnVolume();
	void OnMute();
	void OnPanning();

	void SetTrack(Track *t);

	virtual void OnUpdate(Observable *o);

	Track *track;
	Slider *volume_slider;
	Slider *panning_slider;
	FxList *fx_list;
	BarList *bar_list;
};

#endif /* TRACKDIALOG_H_ */
