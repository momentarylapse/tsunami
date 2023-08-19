/*
 * PeakThread.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "PeakThread.h"
#include "PeakDatabase.h"
#include "../helper/Drawing.h" // color_heat_map
#include "../audioview/graph/AudioViewTrack.h" // AudioViewMode...
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/Sample.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../stuff/PerformanceMonitor.h"
#include "../../lib/hui/hui.h"
#include "../../lib/os/time.h"
#include "../../processing/audio/Spectrogram.h"


InterThreadMessenger::~InterThreadMessenger() {
	flush();
}

void InterThreadMessenger::unsubscribe(VirtualBase *o) {
	flush();
	obs::Node<VirtualBase>::unsubscribe(o);
}

void InterThreadMessenger::flush() {
	if (flushing.load())
		return;
	flushing = true;
	while (counter.load() > 0)
		hui::Application::do_single_main_loop();
		//os::sleep(0.01f);
}

// call from any thread, will notify in main/GUI thread
void InterThreadMessenger::notify_x() {
	if (flushing.load())
		return;
	counter.fetch_add(1);
	hui::run_later(0.001f, [this] {
		if (!flushing.load())
			this->out_changed.notify();
		counter.fetch_sub(1);
	});
}




PeakThread::PeakThread(PeakDatabase *_db) {
	db = _db;
	allow_running = true;
	updating = false;
	perf_channel = PerformanceMonitor::create_channel("peakthread", this);
}

PeakThread::~PeakThread() {
	PerformanceMonitor::delete_channel(perf_channel);
}


void prepare_spectrum(PeakData &p, float sample_rate) {
	//if (p.spectrogram.num > 0)
	//	return false;

	const float DB_RANGE = 50;
	const float DB_BOOST = 10;

	auto pspectrum = Spectrogram::log_spectrogram(p.temp.buffer, sample_rate, PeakData::SPECTRUM_CHUNK, PeakData::SPECTRUM_MIN_FREQ, PeakData::SPECTRUM_MAX_FREQ, PeakData::SPECTRUM_N, WindowFunction::HANN);
	//auto pspectrum = Spectrogram::spectrogram(b, SPECTRUM_CHUNK, SPECTRUM_N, WindowFunction::HANN);
	bytes qspectrum = Spectrogram::quantize(Spectrogram::to_db(pspectrum, DB_RANGE, DB_BOOST));

	p.temp.spectrogram.exchange(qspectrum);
}

void PeakThread::on_run() {
	while (allow_running) {

		os::sleep(0.05f);
		//os::sleep(1.0f);
		Thread::cancelation_point();

		if (!db->requests.has_data())
			continue;

		auto p = db->requests.pop().p;

		printf("==>\n");
		if (p->mode == AudioViewMode::PEAKS) {

			int n = p->temp._update_peaks_prepare();

			Thread::cancelation_point();

			for (int i=0; i<n; i++) {
				if (p->temp._peaks_chunk_needs_update(i)) {
					PerformanceMonitor::start_busy(perf_channel);
					p->temp._update_peaks_chunk(i);
					PerformanceMonitor::end_busy(perf_channel);
					//notify();
					Thread::cancelation_point();
				}
			}

			// write back
			p->mtx.lock();
			PerformanceMonitor::start_busy(perf_channel);
			p->peaks = p->temp.peaks;
			p->temp.peaks.clear();
			p->state = PeakData::State::OK;
			PerformanceMonitor::end_busy(perf_channel);
			p->mtx.unlock();
			notify();
		} else if (p->mode == AudioViewMode::SPECTRUM) {

			PerformanceMonitor::start_busy(perf_channel);
			float sample_rate = 44100;
			prepare_spectrum(*p, sample_rate);

			int n = p->temp.spectrogram.num / PeakData::SPECTRUM_N;
			auto& im = p->temp.image;
			im.resize(n, PeakData::SPECTRUM_N);

			for (int i=0; i<n; i++) {
				for (int k=0; k<PeakData::SPECTRUM_N; k++) {
					float f = Spectrogram::dequantize(p->temp.spectrogram[i * PeakData::SPECTRUM_N + k]);
					im.set_pixel(i, PeakData::SPECTRUM_N - 1 - k, color_heat_map(f));
				}
			}
			PerformanceMonitor::end_busy(perf_channel);


			// write back
			p->mtx.lock();
			PerformanceMonitor::start_busy(perf_channel);
			p->spectrogram = p->temp.spectrogram;
			p->image = p->temp.image;
			p->temp.spectrogram.clear();
			p->temp.image.resize(1, 1);
			p->state = PeakData::State::OK;
			PerformanceMonitor::end_busy(perf_channel);
			p->mtx.unlock();
			notify();
		}
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
}

void PeakThread::notify() {
	messenger.notify_x();
}

