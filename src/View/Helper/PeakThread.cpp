/*
 * PeakThread.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "PeakThread.h"
#include "../AudioView/AudioView.h" // FIXME: get rid!
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Sample.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Stuff/PerformanceMonitor.h"


PeakThread::PeakThread(AudioView *_view) {
	view = _view;
	song = view->song;
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
		hui::Sleep(0.05f);
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
	song->lock();
	if (!updating) {
		song->unlock();
		throw Exception("aaa4");
	}
	int n = buf._update_peaks_prepare();
	song->unlock();

	Thread::cancelation_point();
	if (!updating)
		throw Exception("aaa3");

	for (int i=0; i<n; i++) {
		if (buf._peaks_chunk_needs_update(i)) {
			while (!song->try_lock()) {
				Thread::cancelation_point();
				hui::Sleep(0.01f);
				if (!updating)
					throw Exception("aaa");
			}
			PerformanceMonitor::start_busy(perf_channel);
			buf._update_peaks_chunk(i);
			PerformanceMonitor::end_busy(perf_channel);
			song->unlock();
			notify();
			Thread::cancelation_point();
		}
		if (!updating)
			throw Exception("aaa2");
	}
}

void PeakThread::update_track(Track *t) {
	for (TrackLayer *l: weak(t->layers))
		for (AudioBuffer &b: l->buffers)
			update_buffer(b);
}

void PeakThread::update_song() {
	//msg_write(".");
	for (Track *t: weak(song->tracks))
		update_track(t);
	for (Sample *s: weak(song->samples))
		if (s->buf)
			update_buffer(*s->buf);
}

void PeakThread::notify() {
	hui::run_later(0.01f, [=]{ view->force_redraw(); });
}

