/*
 * PeakThread.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "PeakThread.h"
#include "../AudioView.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Sample.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Stuff/PerformanceMonitor.h"


PeakThread::PeakThread(AudioView *_view) {
	view = _view;
	song = view->song;
	perf_channel = PerformanceMonitor::create_channel("peak", this);
}

PeakThread::~PeakThread() {
	PerformanceMonitor::delete_channel(perf_channel);
}

void PeakThread::on_run() {
	try {
		update_song();
	} catch(...) {
	}
}

void PeakThread::update_buffer(AudioBuffer &buf) {
	song->lock();
	if (!allow_running) {
		song->unlock();
		throw "";
	}
	int n = buf._update_peaks_prepare();
	song->unlock();

	Thread::cancelation_point();

	if (!allow_running)
		throw "";

	for (int i=0; i<n; i++) {
		if (buf._peaks_chunk_needs_update(i)) {
			while (!song->try_lock()) {
				Thread::cancelation_point();
				hui::Sleep(0.01f);
				if (!allow_running)
					throw "";
			}
			PerformanceMonitor::start_busy(perf_channel);
			buf._update_peaks_chunk(i);
			PerformanceMonitor::end_busy(perf_channel);
			song->unlock();
			Thread::cancelation_point();
		}
		if (!allow_running)
			throw "";
	}
}

void PeakThread::update_track(Track *t) {
	for (TrackLayer *l: t->layers)
		for (AudioBuffer &b: l->buffers)
			update_buffer(b);
}

void PeakThread::update_song() {
	for (Track *t: song->tracks)
		update_track(t);
	for (Sample *s: song->samples)
		if (s->buf)
			update_buffer(*s->buf);
}

