/*
 * CaptureDialog.h
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_CAPTURECONSOLE_H_
#define SRC_VIEW_SIDEBAR_CAPTURECONSOLE_H_


#include "SideBar.h"
#include "../../data/Song.h"
#include "../helper/PeakMeterDisplay.h"

class CaptureConsoleMode;
class Session;

class AudioInput;
class AudioSucker;
class SignalChain;
class MidiEventBuffer;

class CaptureConsole : public SideBarConsole {
public:
	CaptureConsole(Session *session, SideBar *bar);


	void on_enter() override;
	void on_leave() override;

	void test_allow_close(hui::Callback cb_yes, hui::Callback cb_no) override;

	void on_start();
	void on_dump();
	void on_pause();
	void on_ok();
	void on_cancel();
	void on_new_version();

	void update_time();

	void on_putput_tick();
	void on_output_end_of_stream();

	bool has_data();

	enum class State {
		EMPTY,
		CAPTURING,
		PAUSED
	} state;

	owned<PeakMeterDisplay> peak_meter_display;
	CaptureConsoleMode *mode;

	owned<CaptureConsoleMode> mode_audio;
	owned<CaptureConsoleMode> mode_midi;
	owned<CaptureConsoleMode> mode_multi;
	shared<SignalChain> chain;
	int n_sync = 0;


	bool insert_audio(Track *target, AudioBuffer &buf, int delay);
	bool insert_midi(Track *target, const MidiEventBuffer &midi, int delay);
};

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLE_H_ */
