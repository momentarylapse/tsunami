/*
 * ViewModeCapture.cpp
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#include "ViewModeCapture.h"
#include "../AudioView.h"
#include "../AudioViewLayer.h"
#include "../../Data/base.h"
#include "../../Data/Track.h"
#include "../../Device/InputStreamAudio.h"
#include "../../Device/InputStreamMidi.h"
#include "../../Module/Audio/AudioSucker.h"


SignalType CaptureTrackData::type()
{
	return target->type;
}


ViewModeCapture::ViewModeCapture(AudioView *view) :
	ViewModeDefault(view)
{
	//console_mode = NULL;
}

ViewModeCapture::~ViewModeCapture()
{
	set_data({});
}

Selection ViewModeCapture::get_hover()
{
	return get_hover_basic(false);
}

void ViewModeCapture::draw_post(Painter *c)
{
	// capturing preview

	for (auto &d: data){
		if (d.type() == SignalType::AUDIO){
			InputStreamAudio *input_audio = (InputStreamAudio*)d.input;
			if (input_audio and input_audio->is_capturing()){
				AudioBuffer &buf = ((AudioSucker*)d.sucker)->buf;
				view->update_peaks_now(buf);
				auto *l = view->get_layer(d.target->layers[0]);
				l->drawBuffer(c, buf, view->cam.pos - view->sel.range.offset, view->colors.capture_marker, l->area.x1, l->area.x2);
				view->draw_time_line(c, view->sel.range.start() + buf.length, (int)Selection::Type::PLAYBACK, view->colors.capture_marker, true);
			}
		}else if (d.type() == SignalType::MIDI){
			InputStreamMidi *input_midi = (InputStreamMidi*)d.input;
			if (input_midi and input_midi->is_capturing()){
				draw_midi(c, view->get_layer(d.target->layers[0]), midi_events_to_notes(input_midi->midi), true, view->sel.range.start());
				view->draw_time_line(c, view->sel.range.start() + input_midi->get_sample_count(), (int)Selection::Type::PLAYBACK, view->colors.capture_marker, true);
			}
		}
	}
}

Set<Track*> ViewModeCapture::prevent_playback()
{
	Set<Track*> prev;
	for (auto &d: data)
		prev.add(d.target);
	return prev;
}

void ViewModeCapture::set_data(const Array<CaptureTrackData> &_data)
{
	for (auto &d: data)
		d.input->unsubscribe(this);

	data = _data;
	view->notify(view->MESSAGE_INPUT_CHANGE);

	for (auto &d: data)
		d.input->subscribe(this, [&]{ on_input_update(); });
}

void ViewModeCapture::on_input_update()
{
	/*for (auto &d: data){
		if (d.type() == SignalType::AUDIO){
			AudioBuffer &buf = export_view_sucker->buf;
			if (input_audio->is_capturing())
				view->cam.make_sample_visible(view->sel.range.start() + buf.length);
			//view->forceRedraw();
		}else if (d.type() == SignalType::MIDI){
			if (input_midi->is_capturing())
				view->cam.make_sample_visible(view->sel.range.start() + input_midi->get_sample_count());
			//view->forceRedraw();
	}*/
	// no need to redraw... already triggered by the OutputStream
}
