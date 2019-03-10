/*
 * ViewModeCapture.h
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODECAPTURE_H_
#define SRC_VIEW_MODE_VIEWMODECAPTURE_H_

#include "ViewModeDefault.h"
#include "../../Data/Range.h"
#include "../../lib/base/base.h"

class InputStreamAudio;
class InputStreamMidi;
class Module;
class SignalChain;
enum class SignalType;
class Track;
class AudioBuffer;
class MidiEventBuffer;

struct CaptureTrackData
{
	Track *target;
	Module *input;
	Module *recorder;
	CaptureTrackData();
	CaptureTrackData(Track *target, Module *input, Module *recorder = nullptr);
	SignalType type();
};

class ViewModeCapture : public ViewModeDefault
{
public:
	ViewModeCapture(AudioView *view);
	virtual ~ViewModeCapture();

	void on_start() override;
	void on_end() override;

	Selection get_hover() override;
	void on_left_button_down() override {}
	void on_left_double_click() override {}

	void draw_post(Painter *c) override;


	virtual Set<Track*> prevent_playback() override;

	Array<CaptureTrackData> data;
	void set_data(const Array<CaptureTrackData> &data);
	SignalChain *chain;
	
	void insert();
	void insert_midi(Track *target, const MidiEventBuffer &midi, int delay);
	void insert_audio(Track *target, const AudioBuffer &buf, int delay);
};

#endif /* SRC_VIEW_MODE_VIEWMODECAPTURE_H_ */
