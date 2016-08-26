/*
 * PauseEditDialog.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_PAUSEEDITDIALOG_H_
#define SRC_VIEW_DIALOG_PAUSEEDITDIALOG_H_

#include "../../lib/hui/hui.h"

class Song;
class Range;

class PauseEditDialog : public HuiDialog
{
public:
	Song *song;
	int index;
	bool apply_to_midi;

	PauseEditDialog(HuiWindow *root, Song *song, int index, bool apply_to_midi);
	void onOk();
	void onClose();
};

#endif /* SRC_VIEW_DIALOG_PAUSEEDITDIALOG_H_ */
