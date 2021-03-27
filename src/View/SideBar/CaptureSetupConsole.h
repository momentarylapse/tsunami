/*
 * CaptureSetupConsole.h
 *
 *  Created on: Mar 27, 2021
 *      Author: michi
 */

#pragma once

#include "SideBar.h"
#include "../../Module/SignalChain.h"

class SignalChain;
class Device;
class Track;
class AudioInput;
class MidiInput;
class PeakMeter;
class PeakMeterDisplay;

class CaptureSetupConsole : public SideBarConsole {
public:
	CaptureSetupConsole(Session *session);

	void on_enter() override;
	void on_leave() override;

	void on_source();

	Array<Device*> sources_audio;
	Array<Device*> sources_midi;

	struct CaptureItem {
		Track *track;
		AudioInput *input_audio;
		MidiInput *input_midi;
		PeakMeterDisplay *peak_meter_display;
		PeakMeter *peak_meter;
		Device *device;
		string id_source, id_target, id_type, id_peaks, id_group, id_grid, id_mapper;
		Array<int> channel_map;
	};
	Array<CaptureItem> items;


	shared<SignalChain> chain;
};
