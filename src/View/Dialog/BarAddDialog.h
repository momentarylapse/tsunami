/*
 * BarAddDialog.h
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_BARADDDIALOG_H_
#define SRC_VIEW_DIALOG_BARADDDIALOG_H_

#include "../../lib/hui/hui.h"
#include "../../Data/Range.h"

class Song;

class BarAddDialog : public hui::Dialog
{
public:
	Song *song;
	int index;

	BarAddDialog(hui::Window *root, Song *s, int index);
	void onOk();
	void onClose();
};

#endif /* SRC_VIEW_DIALOG_BARADDDIALOG_H_ */
