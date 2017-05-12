/*
 * RecordDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef RECORDDIALOG_H_
#define RECORDDIALOG_H_


#include "../../Data/Song.h"
#include "../../lib/hui/hui.h"

class RecordDialog: public hui::Window
{
public:
	RecordDialog(hui::Window *_parent, bool _allow_parent);
	virtual ~RecordDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();
};

#endif /* RECORDDIALOG_H_ */
