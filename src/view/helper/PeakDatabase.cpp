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
#include "../../lib/math/rect.h"
#include "../../lib/math/vec2.h"
#include "../../lib/image/Painter.h"
#include "../audioview/graph/AudioViewTrack.h" // AudioViewMode...
#include <math.h>
#include <atomic>

// AudioBuffer is only guaranteed to exist between acquire() and release()!

const int PeakData::PEAK_CHUNK_EXP = 15;
const int PeakData::PEAK_CHUNK_SIZE = 1<<PEAK_CHUNK_EXP;
const int PeakData::PEAK_OFFSET_EXP = 3;
const int PeakData::PEAK_FINEST_SIZE = 1<<PEAK_OFFSET_EXP;
const int PeakData::PEAK_MAGIC_LEVEL2 = (PEAK_CHUNK_EXP - PEAK_OFFSET_EXP) * 2;

const int PeakData::SPECTRUM_CHUNK = 1024;
const int HorizontallyChunkedImage::CHUNK_SIZE = 4096;
const int PeakData::SPECTRUM_N = 256;
const float PeakData::SPECTRUM_MIN_FREQ = 60.0f;
const float PeakData::SPECTRUM_MAX_FREQ = 3000.0f;


PeakData::PeakData(AudioBuffer& b, AudioViewMode m) : buffer(&b) {
	version = b.version;
	state = State::OUT_OF_SYNC;
	mode = m;
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

bool PeakData::has_spectrum() {
	if (state != State::OK)
		return false;
	return true;
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



void HorizontallyChunkedImage::resize(int w, int h) {
	width = w;
	height = h;
	chunks.clear();
	while (w > 0) {
		chunks.add(Image(min(w, CHUNK_SIZE), h, Black));
		w -= CHUNK_SIZE;
	}
}

void HorizontallyChunkedImage::set_pixel(int x, int y, const color &c) {
	chunks[x / CHUNK_SIZE].set_pixel(x % CHUNK_SIZE, y, c);
}

void HorizontallyChunkedImage::draw(Painter *p, const rect &area) {

	float scale[] = {area.width() / width, 0.0f, 0.0f, area.height() / height};

	p->set_transform(scale, vec2(area.x1, area.y1));

	for (int i=0; i<chunks.num; i++) {
		p->draw_image(vec2(i*CHUNK_SIZE, 0), &chunks[i]);
	}

	scale[0] = 1.0f;
	scale[3] = 1.0f;
	p->set_transform(scale, {0,0});
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

PeakData& PeakDatabase::acquire(AudioBuffer &b, AudioViewMode mode) {
	auto *map = &peak_data;
	if (mode == AudioViewMode::SPECTRUM)
		map = &spectrogram_data;

	int index = map->find(b.uid);
	if (index >= 0) {
		auto p = map->by_index(index);
		p->mtx.lock();
		p->ticks_since_last_usage = 0;
		p->buffer = &b;
		if (p->version != b.version) {
			msg_write(format("%x   V  %x != %x", b.uid, p->version, b.version));
			p->version = b.version;
			p->peaks.clear();
			p->spectrogram.clear();
			p->state = PeakData::State::OUT_OF_SYNC;
		}
		return *p;
	}

	msg_write(format("++ NEW BUFFER %x  %x", b.uid, b.version));
	auto p = new PeakData(b, mode);
	mtx.lock();
	map->set(b.uid, p);
	mtx.unlock();

	p->mtx.lock();
	return *p;
}

void PeakDatabase::release(PeakData& p) {
	p.mtx.unlock();
}

void PeakDatabase::update_peaks_now(AudioBuffer &buf) {
	auto &p = acquire(buf, AudioViewMode::PEAKS);
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

void PeakDatabase::iterate_items(Map& map) {
	for (auto&& [uid,p]: map) {
		p->ticks_since_last_usage ++;
		if (p->ticks_since_last_usage > 10) {
			msg_write("DELETING OLD PEAKS...");
			delete p;
			map.drop(uid);
			// NOTE: we're abusing that map<> does not reallocate.
			// we will only skip 1 element
			//return;
		} else if (p->state == PeakData::State::OUT_OF_SYNC) {
			p->temp.buffer = *p->buffer;
			p->state = PeakData::State::UPDATE_REQUESTED;
			msg_write(format("+ peak request  %x  %x", p->buffer->uid, p->version));
			requests.add({p});
		}
	}
}

void PeakDatabase::iterate() {
	mtx.lock();

	// new requests?
	iterate_items(peak_data);
	iterate_items(spectrogram_data);

	// collect old requests?
	/*for (auto &r: requests)
		if (r.p and r.p->state == PeakData::State::UPDATE_FINISHED) {
			r.p->state = PeakData::State::OK;
		}*/

	mtx.unlock();

}

