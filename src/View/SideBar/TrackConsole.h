/*
 * TrackConsole.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef TRACKCONSOLE_H_
#define TRACKCONSOLE_H_


#include "SideBar.h"
class Track;
class Slider;
class ModulePanel;
class FxListEditor;

class TrackConsole: public SideBarConsole {
public:
	TrackConsole(Session *session);

	void on_enter() override;
	void on_leave() override;

	void load_data();
	void update_strings();

	void on_name();
	void on_volume();
	void on_panning();
	void on_instrument();
	void on_edit_tuning();

	void set_track(Track *t);

	void on_view_cur_track_change();
	void on_update();

	Track *track;
	hui::Panel *panel;
	bool editing;
	owned<FxListEditor> fx_editor;

	enum class Mode {
		FX,
		MIDI_FX,
		SYNTH
	};
	Mode mode;
	void set_mode(Mode m);
};

#endif /* TRACKCONSOLE_H_ */
