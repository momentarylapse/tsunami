/*
 * SongRenderer.cpp
 *
 *  Created on: 17.08.2015
 *      Author: michi
 */

#include "../Source/SongRenderer.h"

#include "../Synth/Synthesizer.h"
#include "../../Plugins/Effect.h"
#include "../../Plugins/MidiEffect.h"
#include "../../Plugins/PluginManager.h"
#include "../../Data/Curve.h"
#include "../../Data/SongSelection.h"
#include "../../Midi/MidiSource.h"
#include "../../Midi/MidiEventStreamer.h"
#include "../../Rhythm/BarStreamer.h"
#include "../../Tsunami.h"

#include "../../lib/math/math.h"

SongRenderer::SongRenderer(Song *s)
{
	MidiEventBuffer no_midi;
	song = s;
	beat_midifier = NULL;
	bar_streamer = NULL;

	preview_effect = NULL;
	allow_loop = false;
	loop_if_allowed = false;
	pos = 0;
	prepare(s->getRange(), false);
	song->subscribe(this, std::bind(&SongRenderer::on_song_change, this));
}

SongRenderer::~SongRenderer()
{
	song->unsubscribe(this);
	clear_data();
}

void SongRenderer::__init__(Song *s)
{
	new(this) SongRenderer(s);
}

void SongRenderer::__delete__()
{
	this->SongRenderer::~SongRenderer();
}

bool intersect_sub(SampleRef *s, const Range &r, Range &ir, int &bpos)
{
	// intersected intervall (track-coordinates)
	int i0 = max(s->pos, r.start());
	int i1 = min(s->pos + s->buf->length, r.end());

	// beginning of the intervall (relative to sub)
	ir.offset = i0 - s->pos;
	// ~ (relative to old intervall)
	bpos = i0 - r.start();
	ir.length = i1 - i0;

	return !ir.empty();
}

void SongRenderer::render_audio_track_no_fx(AudioBuffer &buf, Track *t, int ti)
{
	// track buffer
	t->readBuffersCol(buf, range_cur.offset);

	// subs
	for (SampleRef *s: t->samples){
		if (s->muted)
			continue;

		Range intersect_range;
		int bpos;
		if (!intersect_sub(s, range_cur, intersect_range, bpos))
			continue;

		bpos = s->pos - range_cur.start();
		buf.add(*s->buf, bpos, s->volume * s->origin->volume, 0);
	}
}

void SongRenderer::render_time_track_no_fx(AudioBuffer &buf, Track *t, int ti)
{
	t->synth->out->read(buf);
}

void SongRenderer::render_midi_track_no_fx(AudioBuffer &buf, Track *t, int ti)
{
	t->synth->out->read(buf);
}

void SongRenderer::render_track_no_fx(AudioBuffer &buf, Track *t, int ti)
{
	if (t->type == Track::Type::AUDIO)
		render_audio_track_no_fx(buf, t, ti);
	else if (t->type == Track::Type::TIME)
		render_time_track_no_fx(buf, t, ti);
	else if (t->type == Track::Type::MIDI)
		render_midi_track_no_fx(buf, t, ti);
}

void SongRenderer::apply_fx(AudioBuffer &buf, Track *t, Array<Effect*> &fx_list)
{
	// apply fx
	for (Effect *fx: fx_list)
		if (fx->enabled)
			fx->process(buf);
}

void SongRenderer::render_track_fx(AudioBuffer &buf, Track *t, int ti)
{
	render_track_no_fx(buf, t, ti);

	Array<Effect*> fx = t->fx;
	if (preview_effect)
		fx.add(preview_effect);
	if (fx.num > 0)
		apply_fx(buf, t, fx);
}

int get_first_usable_track(Song *s, Set<Track*> &allowed)
{
	foreachi(Track *t, s->tracks, i)
		if (!t->muted and (allowed.find(t) >= 0))
			return i;
	return -1;
}

void SongRenderer::render_song_no_fx(AudioBuffer &buf)
{
	// any un-muted track?
	int i0 = get_first_usable_track(song, allowed_tracks);
	if (i0 < 0){
		// no -> return silence
		buf.scale(0);
	}else{

		// first (un-muted) track
		render_track_fx(buf, song->tracks[i0], i0);
		buf.scale(song->tracks[i0]->volume, song->tracks[i0]->panning);

		// other tracks
		AudioBuffer tbuf;
		for (int i=i0+1;i<song->tracks.num;i++){
			if (allowed_tracks.find(song->tracks[i]) < 0)
				continue;
			if (song->tracks[i]->muted)
				continue;
			tbuf.resize(buf.length);
			render_track_fx(tbuf, song->tracks[i], i);
			buf.add(tbuf, 0, song->tracks[i]->volume, song->tracks[i]->panning);
		}

		buf.scale(song->volume);
	}
}

void apply_curves(Song *audio, int pos)
{
	for (Curve *c: audio->curves)
		c->apply(pos);
}

void unapply_curves(Song *audio)
{
	for (Curve *c: audio->curves)
		c->unapply();
}

void SongRenderer::read_basic(AudioBuffer &buf, int pos)
{
	range_cur = Range(pos, buf.length);

	apply_curves(song, pos);

	// render without fx
	render_song_no_fx(buf);

	// apply global fx
	if (song->fx.num > 0)
		apply_fx(buf, NULL, song->fx);

	unapply_curves(song);
}

int SongRenderer::read(AudioBuffer &buf)
{
	int size = min(buf.length, _range.end() - pos);
	if (size <= 0)
		return END_OF_STREAM;

	if (song->curves.num >= 0){
		int chunk = 128;
		for (int d=0; d<size; d+=chunk){
			AudioBuffer tbuf;
			tbuf.set_as_ref(buf, d, min(size - d, chunk));
			read_basic(tbuf, pos + d);
			buf.set(tbuf, d, 1.0f);
		}
	}else
		read_basic(buf, pos);

	buf.offset = pos;
	pos += size;
	if ((pos >= _range.end()) and allow_loop and loop_if_allowed)
		seek(_range.offset);
	return size;
}

void SongRenderer::render(const Range &range, AudioBuffer &buf)
{
	prepare(range, false);
	buf.resize(range.length);
	read(buf);
}

void SongRenderer::allowTracks(const Set<Track*> &_allowed_tracks)
{
	for (Track *t: _allowed_tracks)
		if (!allowed_tracks.contains(t))
			reset_track_state(t);
	allowed_tracks = _allowed_tracks;
	//reset_state();
	_seek(pos);
}

void SongRenderer::clear_data()
{
	for (auto m: midi_streamer)
		delete m;
	midi_streamer.clear();

	if (beat_midifier){
		delete beat_midifier;
		beat_midifier = NULL;
	}

	if (bar_streamer){
		delete bar_streamer;
		bar_streamer = NULL;
	}

	allowed_tracks.clear();
}

void SongRenderer::prepare(const Range &__range, bool _allow_loop)
{
	clear_data();
	_range = __range;
	allow_loop = _allow_loop;
	pos = _range.offset;

	for (Track* t: song->tracks)
		allowed_tracks.add(t);

	reset_state();
	build_data();
}

void SongRenderer::reset_state()
{
	for (Effect *fx: song->fx)
		fx->resetState();
	if (preview_effect)
		preview_effect->resetState();

	for (Track *t: song->tracks)
		reset_track_state(t);
}

void SongRenderer::reset_track_state(Track *t)
{
	for (Effect *fx: t->fx)
		fx->resetState();
	t->synth->reset();
}

void SongRenderer::build_data()
{
	bar_streamer = new BarStreamer(song->bars);
	beat_midifier = new BeatMidifier;
	beat_midifier->setBeatSource(bar_streamer);

	foreachi(Track *t, song->tracks, i){
		//midi.add(t, t->midi);
		if (t->type == t->Type::MIDI){
			MidiNoteBuffer _midi = t->midi;
			for (auto c: t->samples)
				if (c->type() == t->Type::MIDI)
					_midi.append(*c->midi, c->pos);
			for (MidiEffect *fx: t->midi.fx){
				fx->prepare();
				fx->process(&_midi);
			}

			MidiEventBuffer raw = midi_notes_to_events(_midi);
			MidiEventStreamer *m = new MidiEventStreamer(raw);
			m->ignore_end = true;
			m->seek(pos);
			midi_streamer.add(m);

			t->synth->setSampleRate(song->sample_rate);
			t->synth->setInstrument(t->instrument);
			t->synth->out->setSource(m);
		}else if (t->type == t->Type::TIME){

			t->synth->setSampleRate(song->sample_rate);
			t->synth->setInstrument(t->instrument);
			t->synth->out->setSource(beat_midifier);
		}
	}
}

int SongRenderer::getSampleRate()
{
	return song->sample_rate;
}

int SongRenderer::getNumSamples()
{
	if (allow_loop and loop_if_allowed)
		return -1;
	return _range.length;
}

void SongRenderer::seek(int _pos)
{
	reset_state();
	_seek(_pos);
}

void SongRenderer::_seek(int _pos)
{
	pos = _pos;
	for (auto m: midi_streamer)
		m->seek(pos);
	if (bar_streamer)
		bar_streamer->seek(pos);
}

void SongRenderer::on_song_change()
{
	//reset_state();
	_seek(pos);
}

