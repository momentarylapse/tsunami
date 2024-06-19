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
	mode = Mode::END_OF_STREAM;
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
	if (mode == Mode::END_OF_STREAM) {
		o("  - end of stream");
		return END_OF_STREAM;
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

string MidiPreviewSource::mode_str(Mode mode) {
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

	if ((mode == Mode::WAITING) or (mode == Mode::END_OF_STREAM) or (mode == Mode::FLUSH)) {
		set_mode(Mode::START_NOTES);
	} else if ((mode == Mode::END_NOTES) or (mode == Mode::FLUSH) or (mode == Mode::ACTIVE_NOTES) or (mode == Mode::CHANGE_NOTES)) {
		set_mode(Mode::CHANGE_NOTES);
	}
}

void MidiPreviewSource::end() {
	o("END");
	std::lock_guard<std::mutex> lock(mutex);

	if (mode == Mode::START_NOTES) {
		// skip queue
		set_mode(Mode::WAITING);
	} else if ((mode == Mode::ACTIVE_NOTES) or (mode == Mode::CHANGE_NOTES)) {
		set_mode(Mode::END_NOTES);
	}
}

}

