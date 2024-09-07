/*
 * NewDialog.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef NEWDIALOG_H_
#define NEWDIALOG_H_


#include "../../lib/hui/Window.h"
#include "../../data/rhythm/Bar.h"

namespace tsunami {

class Song;
enum class SignalType;

class NewSongDialog: public hui::Dialog {
public:
	NewSongDialog(hui::Window *_parent);

	BarPattern new_bar;
	SignalType type;
	bool manually_changed_metronome_flag = false;

	void load_data();
	void apply_data();

	void on_type(SignalType t);
	void on_beats();
	void on_complex();
	void on_pattern();
	void on_divisor();
	void on_ok();
	void on_metronome();
};

}

#endif /* NEWDIALOG_H_ */
