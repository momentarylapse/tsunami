/*
 * NewDialog.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef NEWDIALOG_H_
#define NEWDIALOG_H_


#include "../../lib/hui/hui.h"
#include "../../data/rhythm/Bar.h"

class Song;
enum class SignalType;

class NewDialog: public hui::Dialog {
public:
	NewDialog(hui::Window *_parent);

	BarPattern new_bar;
	SignalType type;

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

#endif /* NEWDIALOG_H_ */
