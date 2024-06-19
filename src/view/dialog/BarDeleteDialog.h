/*
 * BarDeleteDialog.h
 *
 *  Created on: 06.09.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_BARDELETEDIALOG_H_
#define SRC_VIEW_DIALOG_BARDELETEDIALOG_H_

#include "../../lib/hui/hui.h"

namespace tsunami {

class Song;
class Range;

class BarDeleteDialog : public hui::Dialog {
public:
	Song *song;
	Array<int> sel;

	BarDeleteDialog(hui::Window *parent, Song *s, const Array<int> &bars);
	void on_ok();
	void on_replace_by_pause();
};

}

#endif /* SRC_VIEW_DIALOG_BARDELETEDIALOG_H_ */
