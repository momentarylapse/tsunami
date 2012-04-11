/*
 * AudioRenderer.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioRenderer.h"
#include "../Tsunami.h"

AudioRenderer::AudioRenderer()
{
	effect = NULL;
}

AudioRenderer::~AudioRenderer()
{
}

bool intersect_sub(Track &s, const Range &r, Range &ir, int &bpos)
{
	// intersected intervall (track-coordinates)
	int i0 = max(s.pos, r.get_offset());
	int i1 = min(s.pos + s.length, r.get_end());

	// beginning of the intervall (relative to sub)
	ir.offset = i0 - s.pos;
	// ~ (relative to old intervall)
	bpos = i0 - r.get_offset();
	ir.length = i1 - i0;

	return !ir.empty();
}

void AudioRenderer::bb_render_audio_track_no_fx(BufferBox &buf, Track &t, const Range &range)
{
	msg_db_r("bb_render_audio_track_no_fx", 1);

	// track buffer
	BufferBox buf0 = t.ReadBuffers(range);
	buf.swap_ref(buf0);

	// subs
	foreach(t.sub, s){
		if (s.muted)
			continue;

		// can be repetitious!
		for (int i=0;i<s.rep_num+1;i++){
			Range rep_range = range;
			rep_range.move(-s.rep_delay * i);
			Range intersect_range;
			int bpos;
			if (!intersect_sub(s, rep_range, intersect_range, bpos))
				continue;

			BufferBox sbuf = s.ReadBuffers(intersect_range);
			buf.make_own();
			buf.add(sbuf, bpos, s.volume);
		}
	}

	msg_db_l(1);
}

void AudioRenderer::bb_render_time_track_no_fx(BufferBox &buf, Track &t, const Range &r)
{
	msg_db_r("bb_render_time_track_no_fx", 1);

	// silence... TODO...
	buf.resize(r.get_length());

	msg_db_l(1);
}

void AudioRenderer::bb_render_track_no_fx(BufferBox &buf, Track &t, const Range &range)
{
	msg_db_r("bb_render_audio_track_no_fx", 1);

	if (t.type == Track::TYPE_AUDIO)
		bb_render_audio_track_no_fx(buf, t, range);
	else if (t.type == Track::TYPE_TIME)
		bb_render_time_track_no_fx(buf, t, range);

	msg_db_l(1);
}

void AudioRenderer::make_fake_track(Track &t, AudioFile *a, BufferBox &buf, const Range &range)
{
	//msg_write("fake track");
	t.root = a;
	t.buffer.resize(1);
	t.buffer[0].set_as_ref(buf, 0, range.get_length());
}

void AudioRenderer::bb_apply_fx(BufferBox &buf, AudioFile *a, Track *t, Array<Effect> &fx_list, const Range &range)
{
	msg_db_r("bb_apply_fx", 1);

	buf.make_own();

	Track fake_track;
	make_fake_track(fake_track, a, buf, range);

	// apply preview plugin?
	if (t)
		if (effect){
			tsunami->plugins->ApplyEffects(buf, &fake_track, effect);
			//msg_write("preview  .....");
		}

	// apply fx
	foreach(fx_list, fx)
		tsunami->plugins->ApplyEffects(buf, &fake_track, &fx);

	msg_db_l(1);
}

void AudioRenderer::bb_render_track_fx(BufferBox &buf, Track &t, const Range &range)
{
	msg_db_r("bb_render_track_fx", 1);

	bb_render_track_no_fx(buf, t, range);

	if ((t.fx.num > 0) || (effect))
		bb_apply_fx(buf, t.root, &t, t.fx, range);

	msg_db_l(1);
}

int get_first_usable_track(AudioFile *a)
{
	foreachi(a->track, t, i)
		if ((!t.muted) && (t.is_selected))
			return i;
	return -1;
}

void AudioRenderer::bb_render_audio_no_fx(BufferBox &buf, AudioFile *a, const Range &range)
{
	msg_db_r("bb_render_audio_no_fx", 1);

	// any un-muted track?
	int i0 = get_first_usable_track(a);
	if (i0 < 0){
		// no -> return silence
		buf.resize(range.get_length());
	}

	// first (un-muted) track
	bb_render_track_fx(buf, a->track[i0], range);
	buf.scale(a->track[i0].volume);

	// other tracks
	for (int i=i0+1;i<a->track.num;i++){
		if ((a->track[i].muted) || (!a->track[i].is_selected))
			continue;
		BufferBox tbuf;
		bb_render_track_fx(tbuf, a->track[i], range);
		buf.make_own();
		buf.add(tbuf, 0, a->track[i].volume);
	}

	buf.scale(a->volume);

	msg_db_l(1);
}

BufferBox AudioRenderer::RenderAudioFile(AudioFile *a, const Range &range)
{
	msg_db_r("RenderAudioFile", 1);

	// render without fx
	BufferBox buf;
	bb_render_audio_no_fx(buf, a, range);

	// apply fx
	if (a->fx.num > 0)
		bb_apply_fx(buf, a, NULL, a->fx, range);

	msg_db_l(1);
	return buf;
}
