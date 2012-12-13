/*
 * AudioRenderer.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioRenderer.h"
#include "../Plugins/Effect.h"
#include "../Tsunami.h"

AudioRenderer::AudioRenderer()
{
	effect = NULL;
}

AudioRenderer::~AudioRenderer()
{
}

bool intersect_sub(Track *s, const Range &r, Range &ir, int &bpos)
{
	// intersected intervall (track-coordinates)
	int i0 = max(s->pos, r.start());
	int i1 = min(s->pos + s->length, r.end());

	// beginning of the intervall (relative to sub)
	ir.offset = i0 - s->pos;
	// ~ (relative to old intervall)
	bpos = i0 - r.start();
	ir.num = i1 - i0;

	return !ir.empty();
}

void AudioRenderer::bb_render_audio_track_no_fx(BufferBox &buf, Track *t)
{
	msg_db_r("bb_render_audio_track_no_fx", 1);

	// track buffer
	BufferBox buf0 = t->ReadBuffersCol(range);
	buf.swap_ref(buf0);

	// subs
	foreach(Track *s, t->sub){
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

			BufferBox sbuf = s->ReadBuffers(0, intersect_range);
			buf.make_own();
			buf.add(sbuf, bpos, s->volume);
		}
	}

	msg_db_l(1);
}

void AudioRenderer::bb_render_time_track_no_fx(BufferBox &buf, Track *t)
{
	msg_db_r("bb_render_time_track_no_fx", 1);

	// silence... TODO...
	buf.resize(range.length());

	int pos0 = 0;
	foreach(Bar &b, t->bar)
		if (b.type == b.TYPE_BAR){
			for (int j=0;j<b.count;j++){
				for (int i=0;i<b.num_beats;i++){
					int pos = pos0 + i * b.length / b.num_beats;
					buf.add_click(pos - range.offset, (i == 0) ? 0.7f : 0.35f, (i == 0) ? 660.0f : 880.0f, t->root->sample_rate);
				}
				pos0 += b.length;
			}
		}else if (b.type == b.TYPE_PAUSE){
			pos0 += b.length;
		}

	msg_db_l(1);
}

void AudioRenderer::bb_render_track_no_fx(BufferBox &buf, Track *t)
{
	msg_db_r("bb_render_track_no_fx", 1);

	if (t->type == Track::TYPE_AUDIO)
		bb_render_audio_track_no_fx(buf, t);
	else if (t->type == Track::TYPE_TIME)
		bb_render_time_track_no_fx(buf, t);

	msg_db_l(1);
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
	msg_db_r("bb_apply_fx", 1);

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

	msg_db_l(1);
}

void AudioRenderer::bb_render_track_fx(BufferBox &buf, Track *t)
{
	msg_db_r("bb_render_track_fx", 1);

	bb_render_track_no_fx(buf, t);

	if ((t->fx.num > 0) || (effect))
		bb_apply_fx(buf, t, t->fx);

	msg_db_l(1);
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
	msg_db_r("bb_render_audio_no_fx", 1);

	// any un-muted track?
	int i0 = get_first_usable_track(audio);
	if (i0 < 0){
		// no -> return silence
		buf.resize(range.length());
	}else{

		// first (un-muted) track
		bb_render_track_fx(buf, audio->track[i0]);
		buf.scale(audio->track[i0]->volume);

		// other tracks
		for (int i=i0+1;i<audio->track.num;i++){
			if ((audio->track[i]->muted) || (!audio->track[i]->is_selected))
				continue;
			BufferBox tbuf;
			bb_render_track_fx(tbuf, audio->track[i]);
			buf.make_own();
			buf.add(tbuf, 0, audio->track[i]->volume);
		}

		buf.scale(audio->volume);
	}

	msg_db_l(1);
}

BufferBox AudioRenderer::RenderAudioFilePart(AudioFile *a, const Range &_range)
{
	msg_db_r("RenderAudioFilePart", 1);
	audio = a;
	range = _range;

	// render without fx
	BufferBox buf;
	bb_render_audio_no_fx(buf);

	// apply fx
	if (a->fx.num > 0)
		bb_apply_fx(buf, NULL, audio->fx);

	msg_db_l(1);
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
	msg_db_r("Renderer.Prepare", 2);
	foreach(Track *t, a->track)
		foreach(Effect &fx, t->fx)
			fx.Prepare();
	if (effect)
		effect->Prepare();
	msg_db_l(2);
}

void AudioRenderer::CleanUp(AudioFile *a)
{
	msg_db_r("Renderer.CleanUp", 2);
	foreach(Track *t, a->track)
		foreach(Effect &fx, t->fx)
			fx.CleanUp();
	if (effect)
		effect->CleanUp();
	msg_db_l(2);
}
