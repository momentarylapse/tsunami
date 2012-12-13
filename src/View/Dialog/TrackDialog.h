/*
 * TrackDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef TRACKDIALOG_H_
#define TRACKDIALOG_H_


#include "EmbeddedDialog.h"
class Track;
class Slider;
class FxList;
class BarList;

class TrackDialog: public EmbeddedDialog
{
public:
	TrackDialog(CHuiWindow *win);
	virtual ~TrackDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();
	void OnName();
	void OnVolume();
	void OnMute();

	void SetTrack(Track *t);

	Track *track;
	Slider *volume_slider;
	FxList *fx_list;
	BarList *bar_list;
};

#endif /* TRACKDIALOG_H_ */
