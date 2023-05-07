/*
 * Playback.cpp
 *
 *  Created on: 7 May 2023
 *      Author: michi
 */

#include "Playback.h"
#include "Session.h"
#include "module/SignalChain.h"
#include "module/audio/SongRenderer.h"
#include "module/audio/PeakMeter.h"
#include "device/stream/AudioOutput.h"
#include "view/audioview/AudioView.h"
#include "lib/hui/hui.h"

const string Playback::MESSAGE_TICK = "Tick";
const string Playback::MESSAGE_STATE_CHANGE = "StateChange";

Playback::Playback(Session *s) {
	session = s;

	signal_chain = session->create_signal_chain_system("playback");
	renderer = signal_chain->addx<SongRenderer>(ModuleCategory::AUDIO_SOURCE, "SongRenderer");
	peak_meter = signal_chain->addx<PeakMeter>(ModuleCategory::AUDIO_VISUALIZER, "PeakMeter");
	output_stream = signal_chain->addx<AudioOutput>(ModuleCategory::STREAM, "AudioOutput");
	output_stream->set_volume(hui::config.get_float("Output.Volume", 1.0f));
	signal_chain->connect(renderer.get(), 0, peak_meter.get(), 0);
	signal_chain->connect(peak_meter.get(), 0, output_stream.get(), 0);
	signal_chain->mark_all_modules_as_system();

	// propagate messages
	signal_chain->subscribe(this, [this] {
		notify(MESSAGE_TICK);
	}, Module::MESSAGE_TICK);
	signal_chain->subscribe(this, [this] {
		notify(MESSAGE_STATE_CHANGE);
	}, Module::MESSAGE_STATE_CHANGE);

}

Playback::~Playback() {
	signal_chain->unsubscribe(this);
	hui::config.set_float("Output.Volume", output_stream->get_volume());
}

AudioView *Playback::view() {
	return session->view;
}

void Playback::update_range(const Range &r) {

	renderer->change_range(r);

	// TODO ...check....
	if (is_active()) {
		if (renderer->range().is_inside(get_pos())) {
			renderer->change_range(r);
		} else {
			stop();
		}
	}
}

void Playback::play() {
	if (signal_chain->is_prepared() and !signal_chain->is_active()) {
		signal_chain->start();
		return;
	}

	prepare(view()->get_playback_selection(false), true);
	signal_chain->start();
}

void Playback::prepare(const Range &range, bool allow_loop) {
	if (signal_chain->is_active() or signal_chain->is_prepared())
		stop();

	renderer->allow_loop = allow_loop;
	renderer->set_range(range);
	renderer->allow_layers(view()->get_playable_layers());
	auto p0 = output_stream->samples_played();
	if (p0.has_value())
		_stream_offset = range.offset - *p0;
	else
		_stream_offset = range.offset;

	signal_chain->command(ModuleCommand::PREPARE_START, 0);
}

void Playback::set_loop(bool loop) {
	renderer->set_loop(loop);
	notify(MESSAGE_STATE_CHANGE);
}

void Playback::stop() {
	signal_chain->stop_hard();
}

void Playback::pause(bool _pause) {
	if (_pause)
		signal_chain->stop();
	else
		signal_chain->start();
}

// playing or paused?
bool Playback::is_active() {
	return signal_chain->is_active() or signal_chain->is_prepared();
}

bool Playback::is_paused() {
	return signal_chain->is_prepared() and !signal_chain->is_active();
}

int loop_in_range(int pos, const Range &r) {
	return loop(pos, r.start(), r.end());
}

bool Playback::looping() {
	return renderer->loop;
}

int Playback::get_pos() {
	// crappy syncing....
	_sync_counter ++;
	if (_sync_counter > 100)
		_sync_pos();

	int pos = _stream_offset;
	auto p0 = output_stream->samples_played();
	if (p0.has_value())
		pos += *p0;
	if (looping() and renderer->allow_loop)
		return loop_in_range(pos, renderer->range());
	return pos;
}

// crappy syncing....
void Playback::_sync_pos() {
	auto spos = output_stream->samples_played();
    auto lat = output_stream->get_latency();
    if (lat.has_value() and spos.has_value()) {
        int xpos = renderer->get_pos(-output_stream->get_available() - *lat);
        _stream_offset = xpos - *spos;
    }
	_sync_counter = 0;
}

void Playback::set_pos(int pos) {
	renderer->set_pos(pos);
	auto p0 = output_stream->samples_played();
	if (p0.has_value())
		_stream_offset = pos - *p0;
	else
		_stream_offset = pos;
}

void Playback::seek_relative(float dt) {
	int pos = get_pos();
	pos += dt * session->sample_rate();
	pos = max(pos, renderer->range().offset);
	set_pos(pos);
}

