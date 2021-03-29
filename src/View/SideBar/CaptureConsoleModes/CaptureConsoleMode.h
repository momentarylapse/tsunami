/*
 * CaptureConsoleMode.h
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODE_H_
#define SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODE_H_

#include "../../../lib/base/base.h"
#include "../../../lib/base/pointer.h"

class CaptureConsole;
class Song;
class Track;
class AudioView;
class Session;
class SignalChain;
class Module;
class AudioInput;
class MidiInput;
class AudioChannelSelector;
class PeakMeter;
class PeakMeterDisplay;
class Device;
namespace hui {
	class Panel;
}

class CaptureConsoleMode : public VirtualBase {
public:
	CaptureConsoleMode(CaptureConsole *cc);
	virtual void enter() = 0;
	virtual void leave() = 0;
	
	virtual void allow_change_device(bool allow) = 0;

	void start_sync_before();
	void start_sync_after();
	void end_sync();
	void sync();

	CaptureConsole *cc;
	Session *session;
	Song *song;
	AudioView *view;
	shared<SignalChain> chain;



	struct CaptureTrackItem {
		Track *track = nullptr;
		Device *device = nullptr;
		bool enabled = false;
		bool allowing_edit = true;
		hui::Panel *panel = nullptr;
		SignalChain *chain = nullptr;

		AudioInput *input_audio = nullptr;
		MidiInput *input_midi = nullptr;
		AudioChannelSelector *channel_selector = nullptr;
		PeakMeterDisplay *peak_meter_display = nullptr;
		PeakMeter *peak_meter = nullptr;
		Module *accumulator = nullptr;
		string id_group, id_grid, id_source, id_target, id_active, id_peaks, id_mapper;
		Array<int> channel_map();

		void set_device(Device *dev);
		void set_map(const Array<int> &map);
		void enable(bool enabled);
		void allow_edit(bool allow);
	};
	Array<CaptureTrackItem> items;
	void update_data_from_items();
};


#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODE_H_ */
