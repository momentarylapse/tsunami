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

class BarEditDialog : public hui::Dialog
{
public:
	Song *song;
	Array<int> sel;
	bool apply_to_midi;

	BarEditDialog(hui::Window *root, Song *song, const Range &bars, bool apply_to_midi);
	void onOk();
	void onClose();
	void onBeats();
	void onSubBeats();
	void onBpm();
};

#endif /* SRC_VIEW_DIALOG_BAREDITDIALOG_H_ */
