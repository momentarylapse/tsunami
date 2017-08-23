/*
 * SongRenderer.h
 *
 *  Created on: 17.08.2015
 *      Author: michi
 */

#ifndef SRC_AUDIO_SONGRENDERER_H_
#define SRC_AUDIO_SONGRENDERER_H_

#include "AudioRenderer.h"

class MidiDataSource;

class SongRenderer : public AudioRenderer
{
public:
	SongRenderer(Song *s);
	virtual ~SongRenderer();

	void _cdecl __init__(Song *s);
	virtual void _cdecl __delete__();

	void _cdecl render(const Range &range, BufferBox &buf);
	virtual int _cdecl read(BufferBox &buf);
	virtual void _cdecl reset();
	void _cdecl prepare(const Range &range, bool alllow_loop);
	void _cdecl applySelection(SongSelection &sel);

	virtual void _cdecl seek(int pos);

	void _cdecl setRange(const Range &r){ _range = r; }
	virtual Range _cdecl range(){ return _range; }
	virtual int _cdecl getPos(){ return pos; }

	virtual int _cdecl getSampleRate();
	virtual int _cdecl getNumSamples();
	virtual Array<Tag> _cdecl getTags();

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

public:
	Effect *effect;
	bool allow_loop;
	bool loop_if_allowed;
};

#endif /* SRC_AUDIO_SONGRENDERER_H_ */
