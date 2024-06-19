/*
 * PauseAddDialog.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_PAUSEADDDIALOG_H_
#define SRC_VIEW_DIALOG_PAUSEADDDIALOG_H_

#include "../../lib/hui/hui.h"
#include "../../data/Range.h"

namespace tsunami {

class Song;

class PauseAddDialog : public hui::Dialog {
public:
	Song *song;
	int index;

	PauseAddDialog(hui::Window *parent, Song *s, int index);
	void on_ok();
	void on_close();
};

}

#endif /* SRC_VIEW_DIALOG_PAUSEADDDIALOG_H_ */
