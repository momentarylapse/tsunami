/*
 * BarEditDialog.h
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_BAREDITDIALOG_H_
#define SRC_VIEW_DIALOG_BAREDITDIALOG_H_

#include "../../lib/hui/hui.h"

class Song;

class BarEditDialog : public HuiDialog
{
public:
	Song *song;
	Array<int> sel;
	bool apply_to_midi;

	BarEditDialog(HuiWindow *root, Song *song, Array<int> &s, bool apply_to_midi);
	void onOk();
	void onClose();
};

#endif /* SRC_VIEW_DIALOG_BAREDITDIALOG_H_ */
