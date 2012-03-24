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

class NewDialog: public CHuiWindow
{
public:
	NewDialog(CHuiWindow *_parent, bool _allow_parent, AudioFile *a);
	virtual ~NewDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();

	AudioFile *audio;
};

#endif /* NEWDIALOG_H_ */
