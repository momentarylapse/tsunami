/*
 * PauseAddDialog.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_PAUSEADDDIALOG_H_
#define SRC_VIEW_DIALOG_PAUSEADDDIALOG_H_

#include "../../lib/hui/hui.h"
#include "../../Data/Range.h"

class Song;

class PauseAddDialog : public HuiDialog
{
public:
	Song *song;
	Range bars;
	bool apply_to_midi;

	PauseAddDialog(HuiWindow *root, Song *s, const Range &bars, bool apply_to_midi);
	void onOk();
	void onClose();
};

#endif /* SRC_VIEW_DIALOG_PAUSEADDDIALOG_H_ */
