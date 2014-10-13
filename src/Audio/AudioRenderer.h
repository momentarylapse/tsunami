/*
 * AudioRenderer.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef AUDIORENDERER_H_
#define AUDIORENDERER_H_

#include "../Data/AudioFile.h"

class AudioRendererInterface : public VirtualBase
{
public:
	AudioRendererInterface(){}
	virtual ~AudioRendererInterface(){}
	virtual int read(BufferBox &buf){ return 0; }
};

class AudioRenderer : public AudioRendererInterface
{
public:
	AudioRenderer();
	virtual ~AudioRenderer();

	void RenderAudioFile(AudioFile *a, const Range &range, BufferBox &buf);
	//BufferBox RenderAudioFilePart(AudioFile *a, const Range &range);
	virtual int read(BufferBox &buf);
	void Prepare(AudioFile *a, const Range &range, bool alllow_loop);

	void Seek(int pos);

private:
	void read_basic(BufferBox &buf, int pos, int size);
	void bb_render_audio_track_no_fx(BufferBox &buf, Track *t);
	void bb_render_time_track_no_fx(BufferBox &buf, Track *t);
	void bb_render_midi_track_no_fx(BufferBox &buf, Track *t, int ti);
	void bb_render_track_no_fx(BufferBox &buf, Track *t, int ti);
	void make_fake_track(Track *t, BufferBox &buf);
	void bb_apply_fx(BufferBox &buf, Track *t, Array<Effect*> &fx_list);
	void bb_render_track_fx(BufferBox &buf, Track *t, int ti);
	void bb_render_audio_no_fx(BufferBox &buf);

	AudioFile *audio;
	Range range_cur;
	int pos;
	Array<MidiData> midi;

public:
	Effect *effect;
	Range range;
	bool loop;
	bool loop_if_allowed;
};

#endif /* AUDIORENDERER_H_ */
