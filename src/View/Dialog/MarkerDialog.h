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

class MarkerDialog: public hui::ResourceWindow
{
public:
	MarkerDialog(hui::Window *_parent, Track *t, const Range &range, int index);
	virtual ~MarkerDialog();

	void onEdit();
	void onOk();
	void onClose();

	Track *track;
	Range range;
	int index;
};

#endif /* SRC_VIEW_DIALOG_MARKERDIALOG_H_ */
