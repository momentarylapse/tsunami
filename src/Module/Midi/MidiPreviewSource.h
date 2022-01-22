/*
 * MidiPreviewSource.h
 *
 *  Created on: 22 Jan 2022
 *      Author: michi
 */

#ifndef SRC_MODULE_MIDI_MIDIPREVIEWSOURCE_H_
#define SRC_MODULE_MIDI_MIDIPREVIEWSOURCE_H_


#include "MidiSource.h"
#include <mutex>


class MidiPreviewSource : public MidiSource {
public:
	MidiPreviewSource();

	bool debug;
	void o(const string &str);
	int read(MidiEventBuffer &midi) override;
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
	string mode_str(Mode mode);
	void set_mode(Mode new_mode);
	int ttl;

	Array<int> pitch_active;
	Array<int> pitch_queued;
	float volume;


	void start(const Array<int> &_pitch, int _ttl, float _volume);
	void end();

private:
	std::mutex mutex;
};




#endif /* SRC_MODULE_MIDI_MIDIPREVIEWSOURCE_H_ */
