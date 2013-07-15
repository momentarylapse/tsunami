/*
 * Synthesizer.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SYNTHESIZER_H_
#define SYNTHESIZER_H_

class AudioFile;
class Range;
class BufferBox;
class MidiNote;

class Synthesizer
{
public:
	Synthesizer();
	virtual ~Synthesizer();
	virtual void __delete__();

	virtual void AddTone(BufferBox &buf, const Range &range, int pitch, float volume);
	virtual void AddToneFreq(BufferBox &buf, const Range &range, float freq, float volume){}
	virtual void AddClick(BufferBox &buf, int pos, int pitch, float volume);
	//void AddTones(BufferBox &buf, Array<MidiNote> &notes);
	void AddMetronomeClick(BufferBox &buf, int pos, int level, float volume);

	AudioFile *audio;
};

float pitch_to_freq(int pitch);

#endif /* SYNTHESIZER_H_ */
