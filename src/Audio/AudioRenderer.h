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
	AudioRendererInterface();
	virtual ~AudioRendererInterface(){}

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual int read(BufferBox &buf){ return 0; }
	virtual void reset(){}
	virtual Range range(){ return Range(0, 0); }
	virtual int offset(){ return 0; }
	virtual int getSampleRate(){ return DEFAULT_SAMPLE_RATE; }
};

class AudioRenderer : public AudioRendererInterface
{
public:
	AudioRenderer();
	virtual ~AudioRenderer();

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	void renderAudioFile(AudioFile *a, const Range &range, BufferBox &buf);
	//BufferBox RenderAudioFilePart(AudioFile *a, const Range &range);
	virtual int read(BufferBox &buf);
	virtual void reset();
	void prepare(AudioFile *a, const Range &range, bool alllow_loop);

	void seek(int pos);

	void setRange(const Range &r){ _range = r; }
	virtual Range range(){ return _range; }
	virtual int offset(){ return _offset; }

	virtual int getSampleRate();

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
	Range _range;
	Range range_cur;
	int pos;
	int _offset;
	Array<MidiData> midi;

public:
	Effect *effect;
	bool loop;
	bool loop_if_allowed;
};

#endif /* AUDIORENDERER_H_ */
