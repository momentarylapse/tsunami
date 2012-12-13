/*
 * AudioRenderer.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef AUDIORENDERER_H_
#define AUDIORENDERER_H_

#include "../Data/AudioFile.h"

class AudioRenderer
{
public:
	AudioRenderer();
	virtual ~AudioRenderer();

	BufferBox RenderAudioFile(AudioFile *a, const Range &range);
	BufferBox RenderAudioFilePart(AudioFile *a, const Range &range);
	void Prepare(AudioFile *a);
	void CleanUp(AudioFile *a);

private:
	void bb_render_audio_track_no_fx(BufferBox &buf, Track *t);
	void bb_render_time_track_no_fx(BufferBox &buf, Track *t);
	void bb_render_track_no_fx(BufferBox &buf, Track *t);
	void make_fake_track(Track *t, BufferBox &buf);
	void bb_apply_fx(BufferBox &buf, Track *t, Array<Effect> &fx_list);
	void bb_render_track_fx(BufferBox &buf, Track *t);
	void bb_render_audio_no_fx(BufferBox &buf);

	AudioFile *audio;
	Range range;

public:
	Effect *effect;
};

#endif /* AUDIORENDERER_H_ */
