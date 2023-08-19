/*
 * PeakThread.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_PEAKTHREAD_H_
#define SRC_VIEW_HELPER_PEAKTHREAD_H_

#include "../../lib/threads/Thread.h"
#include "../../lib/pattern/Observable.h"


class Song;
class Track;
class AudioBuffer;
class PeakDatabase;

class InterThreadMessenger : public obs::Node<VirtualBase> {
public:
	~InterThreadMessenger();
	void notify_x();
	void flush();
	std::atomic<int> counter{0};
	std::atomic<bool> flushing{false};
	void unsubscribe(VirtualBase *observer);
};


class PeakThread : public Thread {
public:
	PeakDatabase *db;
	int perf_channel;
	std::atomic<bool> allow_running;

	PeakThread(PeakDatabase *db);
	~PeakThread();
	void on_run() override;

	// interface from outside
	void start_update();
	void stop_update();
	void hard_stop();

	InterThreadMessenger messenger;

private:
	std::atomic<bool> updating;

	void update_buffer(AudioBuffer &buf);

	void notify();
};



#endif /* SRC_VIEW_HELPER_PEAKTHREAD_H_ */
