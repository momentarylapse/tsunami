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

	BufferBox RenderAudioFile(AudioFile *a, int pos, int length);

private:
	void bb_render_audio_track_no_fx(BufferBox &buf, Track &t, int pos, int length);
	void bb_render_time_track_no_fx(BufferBox &buf, Track &t, int pos, int length);
	void bb_render_track_no_fx(BufferBox &buf, Track &t, int pos, int length);
	void make_fake_track(Track &t, AudioFile *a, BufferBox &buf, int pos, int length);
	void bb_apply_fx(BufferBox &buf, AudioFile *a, Track *t, Array<Effect> &fx_list, int pos, int length);
	void bb_render_track_fx(BufferBox &buf, Track &t, int pos, int length);
	void bb_render_audio_no_fx(BufferBox &buf, AudioFile *a, int pos, int length);

public:
	Effect *effect;
};

#endif /* AUDIORENDERER_H_ */
