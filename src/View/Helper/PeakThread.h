/*
 * PeakThread.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_PEAKTHREAD_H_
#define SRC_VIEW_HELPER_PEAKTHREAD_H_

#include "../../lib/threads/Thread.h"


class AudioView;
class Song;
class Track;
class AudioBuffer;

class PeakThread : public Thread {
public:
	AudioView *view;
	Song *song;
	int perf_channel;
	std::atomic<bool> allow_running;
	PeakThread(AudioView *view);
	~PeakThread();
	void on_run() override;
	void update_buffer(AudioBuffer &buf);
	void update_track(Track *t);
	void update_song();

	void reset();
};



#endif /* SRC_VIEW_HELPER_PEAKTHREAD_H_ */
