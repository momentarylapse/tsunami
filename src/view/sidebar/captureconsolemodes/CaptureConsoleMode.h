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
#include "../../../lib/pattern/Observable.h"

namespace hui {
	class Panel;
}

namespace tsunami {

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
struct CaptureTrackData;
enum class SignalType;

class CaptureConsoleMode : public obs::Node<VirtualBase> {
public:
	explicit CaptureConsoleMode(CaptureConsole *cc);
	virtual void enter() = 0;
	virtual void leave();
	
	virtual void allow_change_device(bool allow) = 0;

	void start_sync_before();
	void start_sync_after();
	void end_sync();
	void sync();

	void accumulation_start();
	void accumulation_stop();
	void accumulation_clear();

	CaptureConsole* console;
	Session* session;
	Song* song;
	AudioView* view;
	shared<SignalChain> chain;


	Array<CaptureTrackData>& items() const;
	void update_data_from_items();
	Array<int> event_ids;


	Array<Device*> sources_audio;
	Array<Device*> sources_midi;
	Device* get_source(SignalType type, int i) const;
	void update_device_list();
};

}

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODE_H_ */
