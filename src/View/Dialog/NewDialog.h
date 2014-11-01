/*
 * NewDialog.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef NEWDIALOG_H_
#define NEWDIALOG_H_


#include "../../lib/hui/hui.h"
#include "../../Data/AudioFile.h"

class NewDialog: public HuiWindow
{
public:
	NewDialog(HuiWindow *_parent, bool _allow_parent, AudioFile *a);
	virtual ~NewDialog();

	void loadData();
	void applyData();

	void onOk();
	void onClose();
	void onMetronome();

	AudioFile *audio;
};

#endif /* NEWDIALOG_H_ */
