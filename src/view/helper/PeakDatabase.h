/*
 * PeakDatabase.h
 *
 *  Created on: 15 Aug 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_PEAKDATABASE_H_
#define SRC_VIEW_HELPER_PEAKDATABASE_H_

#include "../../lib/base/base.h"
#include "../../lib/base/pointer.h"
#include "../../lib/base/map.h"
#include "../../lib/pattern/Observable.h"
#include "../../lib/threads/util.h"
#include "../../lib/image/image.h"
#include "../../data/audio/AudioBuffer.h"
#include <atomic>

class AudioBuffer;
class Range;
class rect;
class PeakThread;
enum class AudioViewMode;

#define NUM_PEAK_LEVELS		24
#define PEAK_FACTOR			2


// gtk/cairo does not like drawing huge images - let's split them into smaller chunks
struct HorizontallyChunkedImage {
	Array<Image> chunks;
	int width = 0, height = 0;
	static const int CHUNK_SIZE;

	void resize(int w, int h);
	void set_pixel(int x, int y, const color &c);
	void draw(Painter *p, const rect &area);
};


class PeakData {
public:
	PeakData() {}
	PeakData(AudioBuffer& b, AudioViewMode mode);

	AudioViewMode mode;
	AudioBuffer* buffer;
	int version;
	int ticks_since_last_usage;

	enum class State {
		Ok,
		OutOfSync,
		UpdateRequested,
		UpdateFinished,
		Overflow
	};
	std::atomic<State> state;

	Array<bytes> peaks;
	bytes spectrogram;
	HorizontallyChunkedImage image;
	int peak_length;

	static const int PEAK_CHUNK_EXP;
	static const int PEAK_CHUNK_SIZE;
	static const int PEAK_OFFSET_EXP;
	static const int PEAK_FINEST_SIZE;
	static const int PEAK_MAGIC_LEVEL2;

	static const int PEAK_VALUE_UNKNOWN;
	static const int PEAK_VALUE_OVERFLOW;
	static const int PEAK_VALUE_MAX;


	static const int SPECTRUM_CHUNK;
	static const int SPECTRUM_N;
	static const float SPECTRUM_MIN_FREQ;
	static const float SPECTRUM_MAX_FREQ;

//	void _cdecl invalidate_peaks(const Range &r);
	State peaks_chunk_state(const AudioBuffer& buffer, int index);

	bool has_spectrum();

//	void _truncate_peaks(int length);


	void _ensure_peak_size(const AudioBuffer& buf, int level4, int n, bool set_invalid = false);
	int _update_peaks_prepare(const AudioBuffer& buf);
	void _update_peaks_chunk(const AudioBuffer& buf, int index);

};

// PeakThread
struct PeakDataRequest {
	int uid;
	AudioBuffer buffer;
	PeakData peak_data;
	HorizontallyChunkedImage image;
};

// single threaded (apart from requests)
// access to PeakData needs to lock!
class PeakDatabase : public obs::Node<VirtualBase> {
public:
	PeakDatabase();
	~PeakDatabase();

	//bytes& get_peaks(AudioBuffer &b, int level4);
	//bytes& get_spectrogram(AudioBuffer &b);

	void invalidate_all();

	//owned_array<PeakData> peak_data;
	//base::map<int, owned<PeakData>> peak_data;
	using Map = base::map<int, PeakData*>;
	Map peak_data;
	Map spectrogram_data;

	PeakData& _get(AudioBuffer &b, AudioViewMode mode);
	PeakData& acquire(AudioBuffer &b, AudioViewMode mode);
	void release(PeakData& p);


	void update_peaks_now(AudioBuffer &buf);


	owned<PeakThread> peak_thread;
	std::atomic<float> sample_rate;

	void stop_update();

	void iterate(float sample_rate);
	void iterate_items(Map& map);
	void process_replies();

	// shared between main thread and PeakThread:
	InterThreadFifoBuffer<PeakDataRequest*> requests;
	InterThreadFifoBuffer<PeakDataRequest*> replies;
};

#endif /* SRC_VIEW_HELPER_PEAKDATABASE_H_ */
