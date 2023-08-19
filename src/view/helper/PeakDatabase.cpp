/*
 * PeakDatabase.cpp
 *
 *  Created on: 15 Aug 2023
 *      Author: michi
 */

#include "PeakDatabase.h"
#include "PeakThread.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../lib/os/msg.h"
#include <math.h>
#include <atomic>

// AudioBuffer is only guaranteed to exist between acquire() and release()!

const int PeakData::PEAK_CHUNK_EXP = 15;
const int PeakData::PEAK_CHUNK_SIZE = 1<<PEAK_CHUNK_EXP;
const int PeakData::PEAK_OFFSET_EXP = 3;
const int PeakData::PEAK_FINEST_SIZE = 1<<PEAK_OFFSET_EXP;
const int PeakData::PEAK_MAGIC_LEVEL2 = (PEAK_CHUNK_EXP - PEAK_OFFSET_EXP) * 2;


BasePeakDatabaseItem::BasePeakDatabaseItem(AudioBuffer& b) : buffer(&b) {
	version = b.version;
	state = State::OUT_OF_SYNC;
	ticks_since_last_usage = 0;
}


unsigned char inline _shrink_mean(unsigned char a, unsigned char b) {
	return (unsigned char)(sqrt(((float)a * (float)a + (float)b * (float)b) / 2));
}

static bool _shrink_table_created = false;
static unsigned char _shrink_mean_table[256][256];

static void update_shrink_table() {
	for (int a=0; a<256; a++)
		for (int b=0; b<256; b++)
			_shrink_mean_table[a][b] = _shrink_mean(a, b);
	_shrink_table_created = true;
}

#define shrink_max(a, b)	max((a), (b))

unsigned char inline shrink_mean(unsigned char a, unsigned char b) {
	return _shrink_mean_table[a][b];
}

bool PeakData::_peaks_chunk_needs_update(int index) {
	if (state != State::OK)
		return true;
	int pm = PEAK_MAGIC_LEVEL2 * buffer->channels;
	if (peaks.num <= pm)
		return true;
	if (index >= peaks[pm].num)
		return true;
	return (peaks[pm][index] == 255);
}

/*void PeakData::Temp::_truncate_peaks(int _length) {
	int level4 = 0;
	int n = 2*buffer.channels;
	_length /= PEAK_FINEST_SIZE;
	while (level4 < peaks.num) {
		for (int k=0; k<n; k++)
			peaks[level4 + k].resize(_length);
		level4 += n;
		_length /= 2;
	}
}*/

/*void PeakData::Temp::invalidate_peaks(const Range &_range) {
	Range r = buffer.range() and _range;

	int pm = PEAK_MAGIC_LEVEL2 * buffer.channels;
	if (peaks.num < pm)
		return;

	int i0 = (r.start() - buffer.offset) / PEAK_CHUNK_SIZE;
	int i1 = min((r.end() - buffer.offset) / PEAK_CHUNK_SIZE + 1, peaks[pm].num);

	for (int i=i0; i<i1; i++)
		peaks[pm][i] = 255;
}*/

inline float fabsmax(float *p) {
	float a = fabs(*p ++);
	float b = fabs(*p ++);
	float c = fabs(*p ++);
	float d = fabs(*p ++);
	float e = fabs(*p ++);
	float f = fabs(*p ++);
	float g = fabs(*p ++);
	float h = fabs(*p ++);
	return max(max(max(a, b), max(c, d)), max(max(e, f), max(g, h)));
}

void PeakData::Temp::_ensure_peak_size(int level4, int n, bool set_invalid) {
	int dl = 2 * buffer.channels;
	if (peaks.num < level4 + dl)
		peaks.resize(level4 + dl);
	if (peaks[level4].num < n) {
		int n0 = peaks[level4].num;
		for (int k=0; k<dl; k++) {
			peaks[level4 + k].resize(n);
			if (set_invalid)
				memset(&peaks[level4 + k][n0], 255, (n - n0));
		}
	}/*else if (peaks[level4].num < n) {
		for (int k=0; k<4; k++)
			peaks[level4 + k].resize(n);
	}*/
}

bool PeakData::Temp::_peaks_chunk_needs_update(int index) {
	int pm = PEAK_MAGIC_LEVEL2 * buffer.channels;
	if (peaks.num <= pm)
		return true;
	if (index >= peaks[pm].num)
		return true;
	return (peaks[pm][index] == 255);
}

void PeakData::Temp::_update_peaks_chunk(int index) {
	// first level
	int i0 = index * (PEAK_CHUNK_SIZE / PEAK_FINEST_SIZE);
	int i1 = min(i0 + PEAK_CHUNK_SIZE / PEAK_FINEST_SIZE, buffer.length / PEAK_FINEST_SIZE);
	int n = i1 - i0;
	int dl = 2 * buffer.channels;

	_ensure_peak_size(0, i1, true);

	//msg_write(format("lvl0:  %d  %d     %d  %d", i0, n, peaks[0].num, index));

	for (int j=0; j<buffer.channels; j++) {
		for (int i=i0; i<i1; i++)
			peaks[j][i] = (unsigned char)(fabsmax(&buffer.c[j][i * PEAK_FINEST_SIZE]) * 254.0f);
		memcpy(&peaks[buffer.channels + j][i0], &peaks[j][i0], n);
	}

	// medium levels
	int level4 = 0;
	while (n >= 2) {
		level4 += dl;
		n = n / 2;
		i0 = i0 / 2;
		i1 = i0 + n;
		_ensure_peak_size(level4, i1);

		for (int j=0; j<buffer.channels; j++)
			for (int i=i0; i<i1; i++) {
				peaks[level4 + j][i] = shrink_max(peaks[level4 - dl + j][i * 2], peaks[level4 - dl + j][i * 2 + 1]);
				//peaks[level4 + 1][i] = shrink_max(peaks[level4 - 3][i * 2], peaks[level4 - 3][i * 2 + 1]);
				peaks[level4 + buffer.channels + j][i] = shrink_mean(peaks[level4 - buffer.channels + j][i * 2], peaks[level4 - buffer.channels + j][i * 2 + 1]);
				//peaks[level4 + 3][i] = shrink_mean(peaks[level4 - 1][i * 2], peaks[level4 - 1][i * 2 + 1]);
			}
	}

	//	msg_write(format("%d  %d  %d", level4 / 4, peaks.num / 4 - 1, n));
	if (n == 0)
		return;

	// high levels
	for (int k=0; k<32; k++) {
		if ((index & (1<<k)) == 0)
			break;

		if (peaks[level4].num <= (i0 | 1))
			break;

		level4 += dl;
		i0 = i0 / 2;

		_ensure_peak_size(level4, i0 + 1);

		for (int j=0; j<buffer.channels; j++) {
			peaks[level4 + j][i0] = shrink_max(peaks[level4 - dl + j][i0 * 2], peaks[level4 - dl + j][i0 * 2 + 1]);
			//peaks[level4 + 1][i0] = shrink_max(peaks[level4 - 3][i0 * 2], peaks[level4 - 3][i0 * 2 + 1]);
			peaks[level4 + buffer.channels + j][i0] = shrink_mean(peaks[level4 - buffer.channels + j][i0 * 2], peaks[level4 - buffer.channels + j][i0 * 2 + 1]);
			//peaks[level4 + 3][i0] = shrink_mean(peaks[level4 - 1][i0 * 2], peaks[level4 - 1][i0 * 2 + 1]);
		}
	}
}

int PeakData::Temp::_update_peaks_prepare() {
	if (!_shrink_table_created)
		update_shrink_table();

	int n = (int)ceil((float)buffer.length / (float)PEAK_CHUNK_SIZE);

	for (int i=PEAK_OFFSET_EXP; i<=PEAK_CHUNK_EXP; i++)
		_ensure_peak_size((i - PEAK_OFFSET_EXP) * 2 * buffer.channels, buffer.length >> i, true);
	//_ensure_peak_size(PEAK_MAGIC_LEVEL4, n, true);

	return n;
}



PeakData::PeakData(AudioBuffer &b) : BasePeakDatabaseItem(b) {
}

SpectrogramData::SpectrogramData(AudioBuffer &b) : BasePeakDatabaseItem(b) {
}

/*-----------------------------------------------------------
 * PeakDatabase
 */

PeakDatabase::PeakDatabase() {
	peak_thread = new PeakThread(this);
	peak_thread->messenger.out_changed >> create_sink([this] { out_changed(); });
	peak_thread->run();
}

PeakDatabase::~PeakDatabase() {
	peak_thread->messenger.unsubscribe(this);
	peak_thread->hard_stop();
}

void PeakDatabase::invalidate_all() {
	peak_data.clear();
}

PeakData& PeakDatabase::acquire_peaks(AudioBuffer &b) {
	int index = peak_data.find(b.uid);
	if (index >= 0) {
		auto p = peak_data.by_index(index);
		p->mtx.lock();
		p->ticks_since_last_usage = 0;
		p->buffer = &b;
		if (p->version != b.version) {
			msg_write(format("%x   V  %x != %x", b.uid, p->version, b.version));
			p->version = b.version;
			p->peaks.clear();
			p->state = PeakData::State::OUT_OF_SYNC;
		}
		return *p;
	}

	msg_write(format("++ NEW BUFFER %x  %x", b.uid, b.version));
	auto p = new PeakData(b);
	mtx.lock();
	peak_data.set(b.uid, p);
	mtx.unlock();

	p->mtx.lock();
	return *p;
}

SpectrogramData& PeakDatabase::acquire_spectrogram(AudioBuffer &b) {
	int index = spectrogram_data.find(b.uid);
	if (index >= 0) {
		auto p = spectrogram_data.by_index(index);
		p->mtx.lock();
		p->ticks_since_last_usage = 0;
		p->buffer = &b;
		if (p->version != b.version) {
			p->version = b.version;
			p->spectrogram.clear();
			p->state = SpectrogramData::State::OUT_OF_SYNC;
		}
		return *p;
	}

	auto p = new SpectrogramData(b);
	mtx.lock();
	spectrogram_data.set(b.uid, p);
	mtx.unlock();

	p->mtx.lock();
	return *p;
}

void PeakDatabase::release(PeakData& p) {
	p.mtx.unlock();
}

void PeakDatabase::release(SpectrogramData& p) {
	p.mtx.unlock();
}

void PeakDatabase::update_peaks_now(AudioBuffer &buf) {
	auto &p = acquire_peaks(buf);
	int n = p.temp._update_peaks_prepare();

	for (int i=0; i<n; i++)
		if (p.temp._peaks_chunk_needs_update(i))
			p.temp._update_peaks_chunk(i);
	p.peaks = p.temp.peaks;
	release(p);
}

void PeakDatabase::stop_update() {
	peak_thread->stop_update();
}

template<class T>
bool iterate_peak_db_item(T& p) {
	p.ticks_since_last_usage ++;
	if (p.ticks_since_last_usage > 10) {
		return false;
	}
	if (p.state == PeakData::State::OUT_OF_SYNC) {
		p.temp.buffer = *p.buffer;
		p.state = PeakData::State::UPDATE_REQUESTED;
		msg_write(format("+ peak request  %x  %x", p.buffer->uid, p.version));
		return true;
	}
	return false;
}

void PeakDatabase::iterate() {
	mtx.lock();

	// new requests?
	for (auto&& [uid,p]: peak_data)
		if (iterate_peak_db_item(*p))
			requests.add({p, nullptr});
	for (auto&& [uid,p]: spectrogram_data)
		if (iterate_peak_db_item(*p))
			requests.add({nullptr, p});

	// collect old requests?
	/*for (auto &r: requests)
		if (r.p and r.p->state == PeakData::State::UPDATE_FINISHED) {
			r.p->state = PeakData::State::OK;
		}*/

	mtx.unlock();

}

