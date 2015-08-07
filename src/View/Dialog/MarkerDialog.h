/*
 * MarkerDialog.h
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#ifndef SRC_VIEW_DIALOG_MARKERDIALOG_H_
#define SRC_VIEW_DIALOG_MARKERDIALOG_H_


#include "../../lib/hui/hui.h"
#include "../../Data/Track.h"

class MarkerDialog: public HuiWindow
{
public:
	MarkerDialog(HuiWindow *_parent, bool _allow_parent, Track *t, int pos, int index);
	virtual ~MarkerDialog();

	void onEdit();
	void onOk();
	void onClose();

	Track *track;
	int pos;
	int index;
};

#endif /* SRC_VIEW_DIALOG_MARKERDIALOG_H_ */
