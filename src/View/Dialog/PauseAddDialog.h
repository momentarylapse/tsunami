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
class AudioView;

class PauseAddDialog : public hui::Dialog
{
public:
	Song *song;
	AudioView *view;
	int index;

	PauseAddDialog(hui::Window *root, Song *s, AudioView *v, int index);
	void onOk();
	void onClose();
};

#endif /* SRC_VIEW_DIALOG_PAUSEADDDIALOG_H_ */
