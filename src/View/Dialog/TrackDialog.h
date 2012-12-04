/*
 * TrackDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef TRACKDIALOG_H_
#define TRACKDIALOG_H_


#include "../../lib/hui/hui.h"
class Track;
class Slider;
class FxList;
class BarList;

class TrackDialog: public CHuiWindow
{
public:
	TrackDialog(CHuiWindow *_parent, bool _allow_parent, Track *t);
	virtual ~TrackDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();
	void OnName();
	void OnVolume();
	void OnMute();

	Track *track;
	Slider *volume_slider;
	FxList *fx_list;
	BarList *bar_list;
};

#endif /* TRACKDIALOG_H_ */
