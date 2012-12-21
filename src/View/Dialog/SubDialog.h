/*
 * SubDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SUBDIALOG_H_
#define SUBDIALOG_H_

#include "../../lib/hui/hui.h"
class Slider;
class Track;

class SubDialog: public CHuiWindow
{
public:
	SubDialog(CHuiWindow *_parent, bool _allow_parent, Track *s);
	virtual ~SubDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();
	void OnName();
	void OnMute();
	void OnLevelTrack();
	void OnVolume();
	void OnRepNum();
	void OnRepDelay();

	Track *sub;
	Slider *volume_slider;
};

#endif /* SUBDIALOG_H_ */
