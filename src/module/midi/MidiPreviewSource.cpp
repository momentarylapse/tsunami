/*
 * MidiPreviewSource.cpp
 *
 *  Created on: 22 Jan 2022
 *      Author: michi
 */


#include "MidiPreviewSource.h"
#include "../../data/midi/MidiData.h"
#include "../../Session.h"
#include "../../lib/os/msg.h"

namespace tsunami {

MidiPreviewSource::MidiPreviewSource() {
	module_class = "MidiPreviewSource";
	mode = Mode::EndOfStream;
	ttl = -1;
	volume = 1.0f;
	debug = false;
}

void MidiPreviewSource::o(const string &str) {
	if (debug)
		msg_write(str);
}

int MidiPreviewSource::read(MidiEventBuffer &midi) {
	std::lock_guard<std::mutex> lock(mutex);

	o("mps.read");
	if (mode == Mode::EndOfStream) {
		o("  - end of stream");
		return Return::EndOfStream;
	}

	if (mode == Mode::StartNotes) {
		for (int p: pitch_queued)
			midi.add(MidiEvent(0, p, volume));
		pitch_active = pitch_queued;
		set_mode(Mode::ActiveNotes);
	} else if (mode == Mode::EndNotes) {
		for (int p: pitch_active)
			midi.add(MidiEvent(0, p, 0));
		set_mode(Mode::Flush);
		ttl = session->sample_rate() * 2;
	} else if (mode == Mode::ChangeNotes) {
		for (int p: pitch_active)
			midi.add(MidiEvent(0, p, 0));
		for (int p: pitch_queued)
			midi.add(MidiEvent(0, p, volume));
		pitch_active = pitch_queued;
		set_mode(Mode::ActiveNotes);
	}

	if (ttl >= 0) {
		ttl -= midi.samples;
		if (ttl < 0) {
			if (mode == Mode::ActiveNotes)
				set_mode(Mode::EndNotes);
			else if (mode == Mode::Flush)
				set_mode(Mode::EndOfStream);
		}
	}
	return midi.samples;
}

string MidiPreviewSource::mode_str(Mode mode) {
	if (mode == Mode::Waiting)
		return "wait";
	if (mode == Mode::StartNotes)
		return "start";
	if (mode == Mode::ChangeNotes)
		return "change";
	if (mode == Mode::ActiveNotes)
		return "active";
	if (mode == Mode::EndNotes)
		return "end";
	if (mode == Mode::Flush)
		return "flush";
	if (mode == Mode::EndOfStream)
		return "eos";
	return "???";
}

void MidiPreviewSource::set_mode(Mode new_mode) {
	o("    " + mode_str(mode) + " -> " + mode_str(new_mode));
	mode = new_mode;
}

void MidiPreviewSource::start(const Array<int> &_pitch, int _ttl, float _volume) {
	std::lock_guard<std::mutex> lock(mutex);
	o("START");

	pitch_queued = _pitch;
	ttl = _ttl;
	volume = _volume;

	if ((mode == Mode::Waiting) or (mode == Mode::EndOfStream) or (mode == Mode::Flush)) {
		set_mode(Mode::StartNotes);
	} else if ((mode == Mode::EndNotes) or (mode == Mode::Flush) or (mode == Mode::ActiveNotes) or (mode == Mode::ChangeNotes)) {
		set_mode(Mode::ChangeNotes);
	}
}

void MidiPreviewSource::end() {
	o("END");
	std::lock_guard<std::mutex> lock(mutex);

	if (mode == Mode::StartNotes) {
		// skip queue
		set_mode(Mode::Waiting);
	} else if ((mode == Mode::ActiveNotes) or (mode == Mode::ChangeNotes)) {
		set_mode(Mode::EndNotes);
	}
}

}

