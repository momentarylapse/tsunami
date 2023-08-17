/*
 * PeakThread.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "PeakThread.h"
#include "PeakDatabase.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/Sample.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../stuff/PerformanceMonitor.h"
#include "../../lib/hui/hui.h"
#include "../../lib/os/time.h"


InterThreadMessager::~InterThreadMessager() {
	flush();
}

void InterThreadMessager::unsubscribe(VirtualBase *o) {
	flush();
	obs::Node<VirtualBase>::unsubscribe(o);
}

void InterThreadMessager::flush() {
	if (flushing.load())
		return;
	flushing = true;
	while (counter.load() > 0)
		hui::Application::do_single_main_loop();
		//os::sleep(0.01f);
}

// call from any thread, will notify in main/GUI thread
void InterThreadMessager::notify_x() {
	if (flushing.load())
		return;
	counter.fetch_add(1);
	hui::run_later(0.001f, [this] {
		if (!flushing.load())
			this->out_changed.notify();
		counter.fetch_sub(1);
	});
}




PeakThread::PeakThread(Song *s, PeakDatabase *_db) {
	song = s;
	db = _db;
	allow_running = true;
	updating = false;
	perf_channel = PerformanceMonitor::create_channel("peakthread", this);
}

PeakThread::~PeakThread() {
	PerformanceMonitor::delete_channel(perf_channel);
}

void PeakThread::on_run() {
	while (allow_running) {
		if (updating) {
			try {
				update_song();
				updating = false;
				notify();
				//msg_write(":D");
			} catch(Exception &e) {
				//msg_write(":(    " + e.message());
			}
		}
		os::sleep(0.05f);
		Thread::cancelation_point();
	}
}

void PeakThread::start_update() {
	if (updating)
		stop_update();
	//msg_write("PT START");
	updating = true;
}

void PeakThread::stop_update() {
	//msg_write("PT STOP");
	updating = false;
}

void PeakThread::hard_stop() {
	//msg_write("PT HARD STOP");
	updating = false;
	allow_running = false;
	kill();
}

void PeakThread::update_buffer(AudioBuffer &buf) {
	buf.mtx.lock();
	if (!updating) {
		buf.mtx.unlock();
		throw Exception("aaa4");
	}
	auto &p = db->get_data(buf);
	int n = p._update_peaks_prepare();
	buf.mtx.unlock();

	Thread::cancelation_point();
	if (!updating)
		throw Exception("aaa3");

	for (int i=0; i<n; i++) {
		if (p._peaks_chunk_needs_update(i)) {
			while (!buf.mtx.try_lock()) {
				Thread::cancelation_point();
				os::sleep(0.01f);
				if (!updating)
					throw Exception("aaa");
			}
			PerformanceMonitor::start_busy(perf_channel);
			p._update_peaks_chunk(i);
			PerformanceMonitor::end_busy(perf_channel);
			buf.mtx.unlock();
			notify();
			Thread::cancelation_point();
		}
		if (!updating)
			throw Exception("aaa2");
	}
}

// thread safe!
Array<AudioBuffer*> enum_buffers(Song *song) {
	Array<AudioBuffer*> buffers;
	song->lock_shared();
	for (Track *t: weak(song->tracks))
		for (TrackLayer *l: weak(t->layers))
			for (AudioBuffer &b: l->buffers)
				buffers.add(&b);
	for (Sample *s: weak(song->samples))
		if (s->buf)
			buffers.add(s->buf);
	song->unlock_shared();
	return buffers;
}

void PeakThread::update_song() {
	auto buffers = enum_buffers(song);
	for (auto b: buffers)
		update_buffer(*b);
}

void PeakThread::notify() {
	messanger.notify_x();
	//hui::run_later(0.01f, [this] { view->force_redraw(); });
}

