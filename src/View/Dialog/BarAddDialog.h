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

class BarAddDialog : public HuiDialog
{
public:
	Song *song;
	Range bars;
	bool apply_to_midi;

	BarAddDialog(HuiWindow *root, Song *s, const Range &bars, bool apply_to_midi);
	void onOk();
	void onClose();
};

#endif /* SRC_VIEW_DIALOG_BARADDDIALOG_H_ */
