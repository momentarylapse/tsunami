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

class PauseEditDialog : public hui::Dialog
{
public:
	Song *song;
	int index;

	PauseEditDialog(hui::Window *root, Song *song, int index);
	void onOk();
};

#endif /* SRC_VIEW_DIALOG_PAUSEEDITDIALOG_H_ */
