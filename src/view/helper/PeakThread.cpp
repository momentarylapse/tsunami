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
#include "../../data/audio/AudioBuffer.h"
#include "../../stuff/PerformanceMonitor.h"
#include "../../lib/hui/hui.h"
#include "../../lib/os/time.h"
#include "../../processing/audio/Spectrogram.h"

namespace tsunami {

InterThreadMessenger::~InterThreadMessenger() {
	flush();
}

void InterThreadMessenger::unsubscribe(VirtualBase *o) {
	flush();
	obs::Node<VirtualBase>::unsubscribe(o);
}

void InterThreadMessenger::flush() {
	if (flushing)
		return;
	flushing = true;
	while (counter > 0)
		hui::Application::do_single_main_loop();
		//os::sleep(0.01f);
}

// call from any thread, will notify in main/GUI thread
void InterThreadMessenger::notify_x() {
	if (flushing)
		return;
	counter.fetch_add(1);
	hui::run_later(0.001f, [this] {
		if (!flushing)
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


void prepare_spectrum(PeakDataRequest *r, float sample_rate) {
	//if (p.spectrogram.num > 0)
	//	return false;

	const float DB_RANGE = 50;
	const float DB_BOOST = 10;

	auto pspectrum = Spectrogram::log_spectrogram(r->buffer, sample_rate, PeakData::SPECTRUM_CHUNK, PeakData::SPECTRUM_MIN_FREQ, PeakData::SPECTRUM_MAX_FREQ, PeakData::SPECTRUM_N, WindowFunction::HANN);
	//auto pspectrum = Spectrogram::spectrogram(b, SPECTRUM_CHUNK, SPECTRUM_N, WindowFunction::HANN);
	bytes qspectrum = Spectrogram::quantize(Spectrogram::to_db(pspectrum, DB_RANGE, DB_BOOST));

	r->peak_data.spectrogram.exchange(qspectrum);
}

void PeakThread::on_run() {
	while (allow_running) {

		os::sleep(0.05f);
		Thread::cancelation_point();

		if (!db->requests.has_data())
			continue;

		auto r = db->requests.pop();

		if (r->peak_data.mode == AudioViewMode::Peaks) {
			int n = r->peak_data._update_peaks_prepare(r->buffer);

			Thread::cancelation_point();

			for (int i=0; i<n; i++) {
				if (r->peak_data.peaks_chunk_state(r->buffer, i) == PeakData::State::OutOfSync) {
					PerformanceMonitor::start_busy(perf_channel);
					r->peak_data._update_peaks_chunk(r->buffer, i);
					PerformanceMonitor::end_busy(perf_channel);
					Thread::cancelation_point();
				}
			}
		} else if (r->peak_data.mode == AudioViewMode::Spectrum) {

			PerformanceMonitor::start_busy(perf_channel);
			prepare_spectrum(r, db->sample_rate);

			int n = r->peak_data.spectrogram.num / PeakData::SPECTRUM_N;
			auto& im = r->image;
			im.resize(n, PeakData::SPECTRUM_N);

			for (int i=0; i<n; i++) {
				for (int k=0; k<PeakData::SPECTRUM_N; k++) {
					float f = Spectrogram::dequantize(r->peak_data.spectrogram[i * PeakData::SPECTRUM_N + k]);
					im.set_pixel(i, PeakData::SPECTRUM_N - 1 - k, color_heat_map(f));
				}
				if ((i & 0xff) == 0)
					Thread::cancelation_point();
			}
			PerformanceMonitor::end_busy(perf_channel);
		}
		db->replies.add(r);
		notify();
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

}

