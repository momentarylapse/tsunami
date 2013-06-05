/*
 * RecordDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef RECORDDIALOG_H_
#define RECORDDIALOG_H_


#include "../../lib/hui/hui.h"
#include "../../Data/AudioFile.h"

class RecordDialog: public HuiWindow
{
public:
	RecordDialog(HuiWindow *_parent, bool _allow_parent);
	virtual ~RecordDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();
};

#endif /* RECORDDIALOG_H_ */
