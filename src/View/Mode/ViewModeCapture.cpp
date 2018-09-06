/*
 * ViewModeCapture.cpp
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#include "ViewModeCapture.h"
#include "../AudioView.h"
#include "../AudioViewLayer.h"
#include "../../Data/Track.h"
#include "../../Device/InputStreamAudio.h"
#include "../../Device/InputStreamMidi.h"
#include "../../Module/Audio/AudioSucker.h"

InputStreamAudio *export_view_input = nullptr;
AudioSucker *export_view_sucker = nullptr;

ViewModeCapture::ViewModeCapture(AudioView *view) :
	ViewModeDefault(view)
{
	input_audio = nullptr;
	input_midi = nullptr;
	export_view_input = nullptr;
	capturing_track = nullptr;
	//console_mode = NULL;
}

ViewModeCapture::~ViewModeCapture()
{
	set_input_audio(nullptr);
	set_input_midi(nullptr);
}

Selection ViewModeCapture::get_hover()
{
	return get_hover_basic(false);
}

void ViewModeCapture::draw_post(Painter *c)
{
	// capturing preview
	if (input_audio and input_audio->is_capturing()){
		AudioBuffer &buf = export_view_sucker->buf;
		view->update_peaks_now(buf);
		if (capturing_track)
			view->get_layer(capturing_track->layers[0])->drawBuffer(c, buf, view->cam.pos - view->sel.range.offset, view->colors.capture_marker);
		view->draw_time_line(c, view->sel.range.start() + buf.length, (int)Selection::Type::PLAYBACK, view->colors.capture_marker, true);
	}


	if (input_midi and input_midi->is_capturing()){
		if (capturing_track)
			draw_midi(c, view->get_layer(capturing_track->layers[0]), midi_events_to_notes(input_midi->midi), true, view->sel.range.start());
		view->draw_time_line(c, view->sel.range.start() + input_midi->get_sample_count(), (int)Selection::Type::PLAYBACK, view->colors.capture_marker, true);
	}
}

void ViewModeCapture::set_input_audio(InputStreamAudio *_input)
{
	if (input_audio)
		input_audio->unsubscribe(this);

	input_audio = _input;
	export_view_input = input_audio;
	view->notify(view->MESSAGE_INPUT_CHANGE);

	if (input_audio)
		input_audio->subscribe(this, std::bind(&ViewModeCapture::on_input_update, this));
}

void ViewModeCapture::set_input_midi(InputStreamMidi *_input)
{
	if (input_midi)
		input_midi->unsubscribe(this);

	input_midi = _input;
	view->notify(view->MESSAGE_INPUT_CHANGE);

	if (input_midi)
		input_midi->subscribe(this, std::bind(&ViewModeCapture::on_input_update, this));
}

void ViewModeCapture::on_input_update()
{
	if (input_audio){
		AudioBuffer &buf = export_view_sucker->buf;
		if (input_audio->is_capturing())
			view->cam.make_sample_visible(view->sel.range.start() + buf.length);
		//view->forceRedraw();
	}
	if (input_midi){
		if (input_midi->is_capturing())
			view->cam.make_sample_visible(view->sel.range.start() + input_midi->get_sample_count());
		//view->forceRedraw();
	}
	// no need to redraw... already triggered by the OutputStream
}
