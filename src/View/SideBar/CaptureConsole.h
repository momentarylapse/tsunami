/*
 * CaptureDialog.h
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_CAPTURECONSOLE_H_
#define SRC_VIEW_SIDEBAR_CAPTURECONSOLE_H_


#include "SideBar.h"
#include "../../Data/Song.h"
#include "../Helper/PeakMeterDisplay.h"

class CaptureConsoleMode;
class Session;

class InputStreamAudio;
class AudioSucker;
class MidiEventBuffer;

class CaptureConsole : public SideBarConsole
{
public:
	CaptureConsole(Session *session);
	virtual ~CaptureConsole();



	void on_enter() override;
	void on_leave() override;

	void on_start();
	void on_dump();
	void on_pause();
	void on_ok();
	void on_cancel();
	void on_new_version();

	void update_time();

	void on_putput_update();
	void on_output_end_of_stream();

	bool is_capturing();

	PeakMeterDisplay *peak_meter;
	CaptureConsoleMode *mode;

	CaptureConsoleMode *mode_audio;
	CaptureConsoleMode *mode_midi;
	CaptureConsoleMode *mode_multi;


	bool insert_audio(Track *target, AudioBuffer &buf, int delay);
	bool insert_midi(Track *target, const MidiEventBuffer &midi, int delay);
};

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLE_H_ */
