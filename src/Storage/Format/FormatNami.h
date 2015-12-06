/*
 * FormatNami.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef FORMATNAMI_H_
#define FORMATNAMI_H_

#include "Format.h"

class FormatNami : public Format
{
public:
	FormatNami();
	virtual ~FormatNami();

	virtual void loadTrack(StorageOperationData *od){}
	virtual void saveViaRenderer(StorageOperationData *od){}

	virtual void loadSong(StorageOperationData *od);
	virtual void saveSong(StorageOperationData *od);

	Array<int> ChunkPos;
	void make_consistent(Song *s);

	void BeginChunk(const string &name);
	void EndChunk();
	void WriteTag(Tag *t);
	void WriteEffect(Effect *e);
	void WriteBufferBox(BufferBox *b);
	void WriteSample(Sample *s);
	void WriteSampleRef(SampleRef *s);
	void WriteBar(BarPattern &b);
	void WriteMarker(TrackMarker &m);
	void WriteMidiEvent(MidiEvent &e);
	void WriteMidiEffect(MidiEffect *e);
	void WriteMidi(MidiNoteData &m);
	void WriteSynth(Synthesizer *s);
	void WriteTuning(Array<int> &tuning);
	void WriteTrackLevel(TrackLevel *l, int level_no);
	void WriteTrack(Track *t);
	void WriteLevelName();
	void WriteFormat();

	File *f;
	Song *song;
	StorageOperationData *od;
};

#endif /* FORMATNAMI_H_ */
