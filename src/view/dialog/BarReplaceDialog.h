/*
 * BarReplaceDialog.h
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_BARREPLACEDIALOG_H_
#define SRC_VIEW_DIALOG_BARREPLACEDIALOG_H_

#include "../../lib/hui/hui.h"
#include "../../data/rhythm/Bar.h"

namespace tsunami {

class Song;
class Range;

class BarReplaceDialog : public hui::Dialog {
public:
	Song *song;
	Array<int> sel;
	BarPattern new_bar;

	int duration;

	BarReplaceDialog(hui::Window *root, Song *song, const Array<int> &bars);
	void on_ok();
	void on_close();
	void on_beats();
	void on_divisor();
	void on_number();
	void on_complex();
	void on_pattern();
	void update_result_bpm();
	void on_shift_data();
};

}

#endif /* SRC_VIEW_DIALOG_BARREPLACEDIALOG_H_ */
