//
// Created by michi on 13.05.23.
//
#ifndef SRC_VIEW_DIALOG_NEWTRACKDIALOG_H
#define SRC_VIEW_DIALOG_NEWTRACKDIALOG_H


#include "../../lib/hui/hui.h"
#include "../../data/base.h"
#include "../../data/midi/Instrument.h"
#include "../../data/rhythm/Bar.h"

class Song;
class Session;
class Synthesizer;
enum class SignalType;

class NewTrackDialog : public hui::Dialog {
public:
	NewTrackDialog(hui::Window *parent, Session *session);

	SignalType type;
	Session *session;
	Instrument instrument;
	Array<Instrument> instrument_list;
	BarPattern new_bar;
	Array<string> presets;

	shared<Synthesizer> synth;
	shared<hui::Panel> synth_panel;
	void set_synthesizer(Synthesizer *synth);

	void load_data();
	void apply_data();
	void update_strings();

	void on_type(SignalType t);
	void on_instrument();
	void on_beats();
	void on_complex();
	void on_pattern();
	void on_divisor();
	void on_ok();
	void on_metronome();
	void on_edit_tuning();

	void on_save_preset();
	void on_preset_select();
	void on_preset();
};

#endif //SRC_VIEW_DIALOG_NEWTRACKDIALOG_H
