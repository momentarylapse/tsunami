/*
 * SongRenderer.h
 *
 *  Created on: 17.08.2015
 *      Author: michi
 */

#ifndef SRC_AUDIO_SONGRENDERER_H_
#define SRC_AUDIO_SONGRENDERER_H_

#include "AudioRenderer.h"

class SongSelection;
class MidiDataSource;

class SongRenderer : public AudioRenderer
{
public:
	SongRenderer(Song *a, SongSelection *sel);
	virtual ~SongRenderer();

	void _cdecl __init__(Song *s, SongSelection *sel);
	virtual void _cdecl __delete__();

	void render(const Range &range, BufferBox &buf);
	virtual int read(BufferBox &buf);
	virtual void reset();
	void prepare(const Range &range, bool alllow_loop);
	void applySelection(SongSelection &sel);

	virtual void seek(int pos);

	void setRange(const Range &r){ _range = r; }
	virtual Range range(){ return _range; }
	virtual int getPos(){ return pos; }

	virtual int getSampleRate();
	virtual int getNumSamples();
	virtual Array<Tag> getTags();

private:
	void read_basic(BufferBox &buf, int pos, int size);
	void bb_render_audio_track_no_fx(BufferBox &buf, Track *t);
	void bb_render_time_track_no_fx(BufferBox &buf, Track *t);
	void bb_render_midi_track_no_fx(BufferBox &buf, Track *t, int ti);
	void bb_render_track_no_fx(BufferBox &buf, Track *t, int ti);
	void make_fake_track(Track *t, BufferBox &buf);
	void bb_apply_fx(BufferBox &buf, Track *t, Array<Effect*> &fx_list);
	void bb_render_track_fx(BufferBox &buf, Track *t, int ti);
	void bb_render_song_no_fx(BufferBox &buf);

	Song *song;
	Range _range;
	Range range_cur;
	int pos;
	Array<MidiData> midi;

	MidiDataSource *midi_streamer;

	SongSelection *sel;
	bool sel_own;

public:
	Effect *effect;
	bool allow_loop;
	bool loop_if_allowed;
};

#endif /* SRC_AUDIO_SONGRENDERER_H_ */
