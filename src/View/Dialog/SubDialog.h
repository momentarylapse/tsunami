/*
 * SubDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SUBDIALOG_H_
#define SUBDIALOG_H_

#include "../../lib/hui/hui.h"
#include "../../Data/Track.h"

class SubDialog: public CHuiWindow
{
public:
	SubDialog(CHuiWindow *_parent, bool _allow_parent, Track *s);
	virtual ~SubDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();

	Track *sub;
};

#endif /* SUBDIALOG_H_ */
