/*
 * BarAddDialog.h
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_BARADDDIALOG_H_
#define SRC_VIEW_DIALOG_BARADDDIALOG_H_

#include "../../lib/hui/Window.h"
#include "../../data/rhythm/Bar.h"

namespace tsunami {

class Song;

class BarAddDialog : public hui::Dialog {
public:
	Song *song;
	int index;

	BarPattern new_bar;

	BarAddDialog(hui::Window *root, Song *s, int index);

	void on_beats();
	void on_complex();
	void on_pattern();
	void on_divisor();
	void on_ok();
	void on_close();
};

}

#endif /* SRC_VIEW_DIALOG_BARADDDIALOG_H_ */
