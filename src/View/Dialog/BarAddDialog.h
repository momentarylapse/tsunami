/*
 * BarAddDialog.h
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_BARADDDIALOG_H_
#define SRC_VIEW_DIALOG_BARADDDIALOG_H_

#include "../../lib/hui/hui.h"

class Song;

class BarAddDialog : public HuiDialog
{
public:
	Song *song;
	int index;
	bool apply_to_midi;

	BarAddDialog(HuiWindow *root, Song *_s, int _index, bool _apply_to_midi);
	void onOk();
	void onClose();
};

#endif /* SRC_VIEW_DIALOG_BARADDDIALOG_H_ */
