/*
 * SongRenderer.h
 *
 *  Created on: 17.08.2015
 *      Author: michi
 */

#ifndef SRC_AUDIO_SOURCE_SONGRENDERER_H_
#define SRC_AUDIO_SOURCE_SONGRENDERER_H_

#include "../Source/AudioSource.h"

class MidiDataStreamer;

class SongRenderer : public AudioSource
{
public:
	SongRenderer(Song *s);
	virtual ~SongRenderer();

	void _cdecl __init__(Song *s);
	virtual void _cdecl __delete__();

	// from AudioSource
	virtual int _cdecl read(AudioBuffer &buf);
	virtual int _cdecl getSampleRate();

	void _cdecl render(const Range &range, AudioBuffer &buf);
	void _cdecl prepare(const Range &range, bool alllow_loop);
	void _cdecl allowTracks(const Set<Track*> &allowed_tracks);

	void _cdecl seek(int pos);

	void _cdecl setRange(const Range &r){ _range = r; }
	Range _cdecl range(){ return _range; }
	int _cdecl getPos(){ return pos; }

	int _cdecl getNumSamples();

private:
	void _cdecl reset();
	void read_basic(AudioBuffer &buf, int pos);
	void render_audio_track_no_fx(AudioBuffer &buf, Track *t);
	void render_time_track_no_fx(AudioBuffer &buf, Track *t);
	void render_midi_track_no_fx(AudioBuffer &buf, Track *t, int ti);
	void render_track_no_fx(AudioBuffer &buf, Track *t, int ti);
	void apply_fx(AudioBuffer &buf, Track *t, Array<Effect*> &fx_list);
	void render_track_fx(AudioBuffer &buf, Track *t, int ti);
	void render_song_no_fx(AudioBuffer &buf);

	Song *song;
	Range _range;
	Range range_cur;
	int pos;
	Array<MidiData> midi;
	Set<Track*> allowed_tracks;

	MidiDataStreamer *midi_streamer;

public:
	Effect *preview_effect;
	bool allow_loop;
	bool loop_if_allowed;
};

#endif /* SRC_AUDIO_SOURCE_SONGRENDERER_H_ */
