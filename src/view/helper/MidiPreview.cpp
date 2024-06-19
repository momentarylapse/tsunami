/*
 * MidiPreview.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#include "MidiPreview.h"
#include "../../module/synthesizer/Synthesizer.h"
#include "../../module/midi/MidiPreviewSource.h"
#include "../../module/SignalChain.h"
#include "../../module/stream/MidiInput.h"
#include "../../Session.h"

namespace tsunami {

MidiPreview::MidiPreview(Session *s, Synthesizer *_synth) {
	session = s;
	source = new MidiPreviewSource;
	chain = session->create_signal_chain_system("midi-preview");
	chain->_add(source.get());
	joiner = chain->add(ModuleCategory::PLUMBING, "MidiJoiner");
	accumulator = chain->add(ModuleCategory::PLUMBING, "MidiAccumulator");
//	synth->setInstrument(view->cur_track->instrument);
	synth = chain->_add(_synth);
	out = chain->add(ModuleCategory::STREAM, "AudioOutput");

	input = nullptr;
	input_device = nullptr;

	chain->connect(source.get(), 0, joiner.get(), 0);
	chain->connect(joiner.get(), 0, synth.get(), 0);
	chain->connect(synth.get(), 0, out.get(), 0);
	chain->connect(accumulator.get(), 0, joiner.get(), 1);

	chain->mark_all_modules_as_system();
}

MidiPreview::~MidiPreview() {
	session->remove_signal_chain(chain.get());
}

void MidiPreview::start(const Array<int> &pitch, float volume, float ttl) {
	//kill_preview();

	source->start(pitch, session->sample_rate() * ttl, volume);
	chain->start();
}

void MidiPreview::end() {
	source->end();
}

void MidiPreview::_start_input() {
	input = chain->addx<MidiInput>(ModuleCategory::STREAM, "MidiInput");
	chain->connect(input.get(), 0, accumulator.get(), 0);
	//chain->subscribe(this, [=]{ on_midi_input(this); }, Module::MESSAGE_TICK);
	chain->start();
	chain->command(ModuleCommand::ACCUMULATION_START, 0);
}

void MidiPreview::_stop_input() {
	//chain->unsubscribe(this);
	chain->command(ModuleCommand::ACCUMULATION_STOP, 0);
	chain->stop();
	chain->delete_module(input.get());
	input = nullptr;
}

void MidiPreview::set_input_device(Device *d) {
	input_device = d;
	if (input)
		input->set_device(d);
}

}
