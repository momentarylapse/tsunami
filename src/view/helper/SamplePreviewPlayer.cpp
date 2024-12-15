//
// Created by Michael Ankele on 2024-12-16.
//

#include "SamplePreviewPlayer.h"
#include "../helper/Progress.h"
#include "../../data/base.h"
#include "../../module/stream/AudioOutput.h"
#include "../../module/SignalChain.h"
#include "../../module/audio/BufferStreamer.h"
#include "../../module/audio/SongRenderer.h"
#include "../../module/midi/MidiEventStreamer.h"
#include "../../Session.h"
#include "../../lib/hui/hui.h"

namespace tsunami {



SamplePreviewPlayer::SamplePreviewPlayer(hui::Window* win, Session* session, Sample *s) {
	sample = s;
	chain = session->create_signal_chain_system("sample-preview");
	if (sample->type == SignalType::Audio) {
		renderer = new BufferStreamer(sample->buf);
		chain->_add(renderer);
		stream = chain->addx<AudioOutput>(ModuleCategory::Stream, "AudioOutput").get();
		chain->connect(renderer, 0, stream, 0);
	} else { // MIDI
		midi_streamer = new MidiEventStreamer();
		midi_streamer->set_data(midi_notes_to_events(sample->midi));
		chain->_add(midi_streamer);
		auto synth = chain->add(ModuleCategory::Synthesizer, "").get();
		stream = chain->addx<AudioOutput>(ModuleCategory::Stream, "AudioOutput").get();
		chain->connect(midi_streamer, 0, synth, 0);
		chain->connect(synth, 0, stream, 0);
	}

	progress = new ProgressCancelable(_("Preview"), win);
	progress->out_cancel >> create_sink([this] { on_progress_cancel(); });
	chain->out_tick >> create_sink([this] { on_preview_tick(); });
	chain->out_play_end_of_stream >> create_sink([this] { on_preview_stream_end(); });
	chain->start();
}

SamplePreviewPlayer::~SamplePreviewPlayer() {
	if (progress) {
		progress->unsubscribe(this);
		progress = nullptr;
	}
	chain->unsubscribe(this);
	stream->unsubscribe(this);
	chain->stop();
	chain->unregister();
	sample = nullptr;
}

void SamplePreviewPlayer::on_preview_tick() {
	int pos = 0;
	if (sample->type == SignalType::Audio) {
		pos = renderer->get_pos();
	} else {
		pos = midi_streamer->get_pos();
	}
	const Range r = sample->range();
	progress->set(_("Preview"), (float)(pos - r.offset) / r.length);
}

void SamplePreviewPlayer::on_preview_stream_end() {
	hui::run_later(0.1f, [this] {
		end_preview();
	});
}


void SamplePreviewPlayer::end_preview() {
	// :D
	delete this;
}

void SamplePreviewPlayer::on_progress_cancel() {
	if (progress)
		end_preview();
}


void SamplePreviewPlayer::play(hui::Window* win, Session* session, Sample *sample) {
	// who needs memory management?!?
	new SamplePreviewPlayer(win, session, sample);
}


} // tsunami
