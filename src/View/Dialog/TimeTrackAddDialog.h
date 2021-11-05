/*
 * TimeTrackAddDialog.h
 *
 *  Created on: Nov 5, 2021
 *      Author: michi
 */

#pragma once

#include "../../lib/hui/hui.h"
#include "../../Data/Rhythm/Bar.h"

class Song;

class TimeTrackAddDialog: public hui::Dialog {
public:
	TimeTrackAddDialog(Song *song, hui::Window *_parent);

	Song *song;
	BarPattern new_bar;

	void load_data();
	void apply_data();

	void on_beats();
	void on_complex();
	void on_pattern();
	void on_divisor();
	void on_ok();
	void on_add_bars();
};
