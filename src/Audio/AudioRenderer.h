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

private:
	void bb_render_audio_track_no_fx(BufferBox &buf, Track &t, const Range &range);
	void bb_render_time_track_no_fx(BufferBox &buf, Track &t, const Range &range);
	void bb_render_track_no_fx(BufferBox &buf, Track &t, const Range &range);
	void make_fake_track(Track &t, AudioFile *a, BufferBox &buf, const Range &range);
	void bb_apply_fx(BufferBox &buf, AudioFile *a, Track *t, Array<Effect> &fx_list, const Range &range);
	void bb_render_track_fx(BufferBox &buf, Track &t, const Range &range);
	void bb_render_audio_no_fx(BufferBox &buf, AudioFile *a, const Range &range);

public:
	Effect *effect;
};

#endif /* AUDIORENDERER_H_ */
