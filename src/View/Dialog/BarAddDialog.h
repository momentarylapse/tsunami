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
class AudioView;

class BarAddDialog : public HuiDialog
{
public:
	Song *song;
	AudioView *view;
	Range bars;

	BarAddDialog(HuiWindow *root, Song *s, AudioView *v);
	void onOk();
	void onClose();
};

#endif /* SRC_VIEW_DIALOG_BARADDDIALOG_H_ */
