/*
 * MidiPreview.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#include "MidiPreview.h"
#include "../../Session.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../Module/Midi/MidiSource.h"
#include "../../Module/SignalChain.h"
#include <mutex>
#include "../../Device/Stream/MidiInput.h"
#include "../../lib/file/msg.h"


class MidiPreviewSource : public MidiSource {
public:
	MidiPreviewSource() {
		module_class = "MidiPreviewSource";
		mode = Mode::END_OF_STREAM;
		ttl = -1;
		volume = 1.0f;
		debug = false;
	}

	bool debug;
	void o(const string &str) {
		if (debug)
			msg_write(str);
	}

	int read(MidiEventBuffer &midi) override {
		std::lock_guard<std::mutex> lock(mutex);

		o("mps.read");
		if (mode == Mode::END_OF_STREAM) {
			o("  - end of stream");
			return Port::END_OF_STREAM;
		}

		if (mode == Mode::START_NOTES) {
			for (int p: pitch_queued)
				midi.add(MidiEvent(0, p, volume));
			pitch_active = pitch_queued;
			set_mode(Mode::ACTIVE_NOTES);
		} else if (mode == Mode::END_NOTES) {
			for (int p: pitch_active)
				midi.add(MidiEvent(0, p, 0));
			set_mode(Mode::FLUSH);
			ttl = session->sample_rate() * 2;
		} else if (mode == Mode::CHANGE_NOTES) {
			for (int p: pitch_active)
				midi.add(MidiEvent(0, p, 0));
			for (int p: pitch_queued)
				midi.add(MidiEvent(0, p, volume));
			pitch_active = pitch_queued;
			set_mode(Mode::ACTIVE_NOTES);
		}

		if (ttl >= 0) {
			ttl -= midi.samples;
			if (ttl < 0) {
				if (mode == Mode::ACTIVE_NOTES)
					set_mode(Mode::END_NOTES);
				else if (mode == Mode::FLUSH)
					set_mode(Mode::END_OF_STREAM);
			}
		}
		return midi.samples;
	}
	enum class Mode {
		WAITING,
		START_NOTES,
		CHANGE_NOTES,
		ACTIVE_NOTES,
		END_NOTES,
		FLUSH,
		END_OF_STREAM
	};
	Mode mode;
	string mode_str(Mode mode) {
		if (mode == Mode::WAITING)
			return "wait";
		if (mode == Mode::START_NOTES)
			return "start";
		if (mode == Mode::CHANGE_NOTES)
			return "change";
		if (mode == Mode::ACTIVE_NOTES)
			return "active";
		if (mode == Mode::END_NOTES)
			return "end";
		if (mode == Mode::FLUSH)
			return "flush";
		if (mode == Mode::END_OF_STREAM)
			return "eos";
		return "???";
	}
	void set_mode(Mode new_mode) {
		o("    " + mode_str(mode) + " -> " + mode_str(new_mode));
		mode = new_mode;
	}
	int ttl;

	Array<int> pitch_active;
	Array<int> pitch_queued;
	float volume;


	void start(const Array<int> &_pitch, int _ttl, float _volume) {
		std::lock_guard<std::mutex> lock(mutex);
		o("START");

		pitch_queued = _pitch;
		ttl = _ttl;
		volume = _volume;

		if ((mode == Mode::WAITING) or (mode == Mode::END_OF_STREAM) or (mode == Mode::FLUSH)) {
			set_mode(Mode::START_NOTES);
		} else if ((mode == Mode::END_NOTES) or (mode == Mode::FLUSH) or (mode == Mode::ACTIVE_NOTES) or (mode == Mode::CHANGE_NOTES)) {
			set_mode(Mode::CHANGE_NOTES);
		}
	}
	void end() {
		o("END");
		std::lock_guard<std::mutex> lock(mutex);

		if (mode == Mode::START_NOTES) {
			// skip queue
			set_mode(Mode::WAITING);
		} else if ((mode == Mode::ACTIVE_NOTES) or (mode == Mode::CHANGE_NOTES)) {
			set_mode(Mode::END_NOTES);
		}
	}

private:
	std::mutex mutex;
};

MidiPreview::MidiPreview(Session *s, Synthesizer *_synth) {
	session = s;
	source = new MidiPreviewSource;
	chain = session->create_signal_chain_system("midi-preview");
	chain->_add(source);
	joiner = chain->add(ModuleCategory::PLUMBING, "MidiJoiner");
	accumulator = chain->add(ModuleCategory::PLUMBING, "MidiAccumulator");
//	synth->setInstrument(view->cur_track->instrument);
	synth = chain->_add(_synth);
	out = chain->add(ModuleCategory::STREAM, "AudioOutput");

	input = nullptr;
	input_device = nullptr;

	chain->connect(source, 0, joiner, 0);
	chain->connect(joiner, 0, synth, 0);
	chain->connect(synth, 0, out, 0);
	chain->connect(accumulator, 0, joiner, 1);

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
	input = (MidiInput*)chain->add(ModuleCategory::STREAM, "MidiInput");
	chain->connect(input, 0, accumulator, 0);
	//chain->subscribe(this, [=]{ on_midi_input(this); }, Module::MESSAGE_TICK);
	chain->start();
	chain->command(ModuleCommand::ACCUMULATION_START, 0);
}

void MidiPreview::_stop_input() {
	//chain->unsubscribe(this);
	chain->command(ModuleCommand::ACCUMULATION_STOP, 0);
	chain->stop();
	chain->delete_module(input);
	input = nullptr;
}

void MidiPreview::set_input_device(Device *d) {
	input_device = d;
	if (input)
		input->set_device(d);
}

