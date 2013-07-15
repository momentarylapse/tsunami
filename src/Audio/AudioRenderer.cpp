/*
 * AudioRenderer.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioRenderer.h"
#include "../Plugins/Effect.h"
#include "../Plugins/ExtendedBufferBox.h"
#include "../Tsunami.h"

AudioRenderer::AudioRenderer()
{
	effect = NULL;
}

AudioRenderer::~AudioRenderer()
{
}

bool intersect_sub(SampleRef *s, const Range &r, Range &ir, int &bpos)
{
	// intersected intervall (track-coordinates)
	int i0 = max(s->pos, r.start());
	int i1 = min(s->pos + s->buf.num, r.end());

	// beginning of the intervall (relative to sub)
	ir.offset = i0 - s->pos;
	// ~ (relative to old intervall)
	bpos = i0 - r.start();
	ir.num = i1 - i0;

	return !ir.empty();
}

void AudioRenderer::bb_render_audio_track_no_fx(BufferBox &buf, Track *t)
{
	msg_db_f("bb_render_audio_track_no_fx", 1);

	// track buffer
	BufferBox buf0 = t->ReadBuffersCol(range);
	buf.swap_ref(buf0);

	// subs
	foreach(SampleRef *s, t->sample){
		if (s->muted)
			continue;

		// can be repetitious!
		for (int i=0;i<s->rep_num+1;i++){
			Range rep_range = range;
			rep_range.move(-s->rep_delay * i);
			Range intersect_range;
			int bpos;
			if (!intersect_sub(s, rep_range, intersect_range, bpos))
				continue;

			buf.make_own();
			bpos = s->pos + s->rep_delay * i - range.start();
			buf.add(s->buf, bpos, s->volume * s->origin->volume, 0);
		}
	}
}

void AudioRenderer::bb_render_time_track_no_fx(BufferBox &buf, Track *t)
{
	msg_db_f("bb_render_time_track_no_fx", 1);

	// silence... TODO...
	buf.resize(range.length());

	Array<Beat> beats = t->bar.GetBeats(range);

	foreach(Beat &b, beats)
		((ExtendedBufferBox&)buf).add_click(b.pos - range.offset, (b.beat_no == 0) ? 0.8f : 0.3f, (b.beat_no == 0) ? 660.0f : 880.0f, t->root->sample_rate);
}

void AudioRenderer::bb_render_midi_track_no_fx(BufferBox &buf, Track *t)
{
	msg_db_f("bb_render_midi_track_no_fx", 1);

	// silence... TODO...
	buf.resize(range.length());

	Array<MidiNote> notes = t->midi.GetNotes(range);

	foreach(MidiNote &n, notes){
		Range r = Range(n.range.offset - range.offset, n.range.num);
		((ExtendedBufferBox&)buf).add_tone(r, n.volume, n.GetFrequency(), t->root->sample_rate);
	}
}

void AudioRenderer::bb_render_track_no_fx(BufferBox &buf, Track *t)
{
	msg_db_f("bb_render_track_no_fx", 1);

	if (t->type == Track::TYPE_AUDIO)
		bb_render_audio_track_no_fx(buf, t);
	else if (t->type == Track::TYPE_TIME)
		bb_render_time_track_no_fx(buf, t);
	else if (t->type == Track::TYPE_MIDI)
		bb_render_midi_track_no_fx(buf, t);
}

void AudioRenderer::make_fake_track(Track *t, BufferBox &buf)
{
	//msg_write("fake track");
	t->root = audio;
	t->level.resize(1);
	t->level[0].buffer.resize(1);
	t->level[0].buffer[0].set_as_ref(buf, 0, range.length());
}

void AudioRenderer::bb_apply_fx(BufferBox &buf, Track *t, Array<Effect> &fx_list)
{
	msg_db_f("bb_apply_fx", 1);

	buf.make_own();

	Track fake_track;
	make_fake_track(&fake_track, buf);

	// apply preview plugin?
	if (t)
		if (effect){
			effect->Apply(buf, &fake_track, false);
			//msg_write("preview  .....");
		}

	// apply fx
	foreach(Effect &fx, fx_list)
		fx.Apply(buf, &fake_track, false);
}

void AudioRenderer::bb_render_track_fx(BufferBox &buf, Track *t)
{
	msg_db_f("bb_render_track_fx", 1);

	bb_render_track_no_fx(buf, t);

	if ((t->fx.num > 0) || (effect))
		bb_apply_fx(buf, t, t->fx);
}

int get_first_usable_track(AudioFile *a)
{
	foreachi(Track *t, a->track, i)
		if ((!t->muted) && (t->is_selected))
			return i;
	return -1;
}

void AudioRenderer::bb_render_audio_no_fx(BufferBox &buf)
{
	msg_db_f("bb_render_audio_no_fx", 1);

	// any un-muted track?
	int i0 = get_first_usable_track(audio);
	if (i0 < 0){
		// no -> return silence
		buf.resize(range.length());
	}else{

		// first (un-muted) track
		bb_render_track_fx(buf, audio->track[i0]);
		buf.scale(audio->track[i0]->volume, audio->track[i0]->panning);

		// other tracks
		for (int i=i0+1;i<audio->track.num;i++){
			if ((audio->track[i]->muted) || (!audio->track[i]->is_selected))
				continue;
			BufferBox tbuf;
			bb_render_track_fx(tbuf, audio->track[i]);
			buf.make_own();
			buf.add(tbuf, 0, audio->track[i]->volume, audio->track[i]->panning);
		}

		buf.scale(audio->volume);
	}
}

BufferBox AudioRenderer::RenderAudioFilePart(AudioFile *a, const Range &_range)
{
	msg_db_f("RenderAudioFilePart", 1);
	audio = a;
	range = _range;

	// render without fx
	BufferBox buf;
	bb_render_audio_no_fx(buf);

	// apply fx
	if (a->fx.num > 0)
		bb_apply_fx(buf, NULL, audio->fx);

	return buf;
}

BufferBox AudioRenderer::RenderAudioFile(AudioFile *a, const Range &range)
{
	BufferBox buf;
	Prepare(a);
	buf = RenderAudioFilePart(a, range);
	CleanUp(a);
	return buf;
}

void AudioRenderer::Prepare(AudioFile *a)
{
	msg_db_f("Renderer.Prepare", 2);
	foreach(Effect &fx, a->fx)
		fx.Prepare();
	foreach(Track *t, a->track)
		foreach(Effect &fx, t->fx)
			fx.Prepare();
	if (effect)
		effect->Prepare();
}

void AudioRenderer::CleanUp(AudioFile *a)
{
	msg_db_f("Renderer.CleanUp", 2);
	foreach(Effect &fx, a->fx)
		fx.CleanUp();
	foreach(Track *t, a->track)
		foreach(Effect &fx, t->fx)
			fx.CleanUp();
	if (effect)
		effect->CleanUp();
}
