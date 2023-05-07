/*
 * Playback.h
 *
 *  Created on: 7 May 2023
 *      Author: michi
 */

#ifndef SRC_PLAYBACK_H_
#define SRC_PLAYBACK_H_

#include "lib/base/pointer.h"
#include "lib/pattern/Observable.h"

class Session;
class SignalChain;
class SongRenderer;
class PeakMeter;
class AudioOutput;
class Range;
class AudioView;

class Playback : public Observable<VirtualBase> {
public:
	Playback(Session *s);
	~Playback();

	static const string MESSAGE_TICK;
	static const string MESSAGE_STATE_CHANGE;


	Session *session;
	AudioView *view();
	shared<SignalChain> signal_chain;
	shared<SongRenderer> renderer;
	shared<PeakMeter> peak_meter;
	shared<AudioOutput> output_stream;
	void set_loop(bool loop);
	void play();
	void stop();
	void pause(bool pause);
	void prepare(const Range &range, bool allow_loop);
	bool is_active();
	bool is_paused();
	int get_pos();
	bool looping();
	void _sync_pos();
	int _sync_counter = 0;
	int64 _stream_offset = 0;
	void set_pos(int pos);
	void seek_relative(float dt);

	void update_range(const Range& r);
};

#endif /* SRC_PLAYBACK_H_ */
