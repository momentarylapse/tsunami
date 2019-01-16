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
class Range;
class BarPattern;

class BarEditDialog : public hui::Dialog
{
public:
	Song *song;
	Array<int> sel;
	BarPattern *new_bar;

	int duration;

	BarEditDialog(hui::Window *root, Song *song, const Range &bars);
	void on_ok();
	void on_close();
	void on_beats();
	void on_sub_beats();
	void on_bpm();
	void on_number();
	void on_complex();
	void on_pattern();
	void on_mode();
	void update_result_bpm();
	void on_shift_data();
};

#endif /* SRC_VIEW_DIALOG_BAREDITDIALOG_H_ */
