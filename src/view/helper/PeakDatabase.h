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
#include <shared_mutex>

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
	PeakData(AudioBuffer& b, AudioViewMode mode);

	std::shared_timed_mutex mtx;
	AudioViewMode mode;
	AudioBuffer* buffer;
	int version;
	int ticks_since_last_usage;

	enum class State {
		OK,
		OUT_OF_SYNC,
		UPDATE_REQUESTED,
		UPDATE_FINISHED
	};
	std::atomic<State> state;

	Array<bytes> peaks;
	bytes spectrogram;
	HorizontallyChunkedImage image;

	static const int PEAK_CHUNK_EXP;
	static const int PEAK_CHUNK_SIZE;
	static const int PEAK_OFFSET_EXP;
	static const int PEAK_FINEST_SIZE;
	static const int PEAK_MAGIC_LEVEL2;


	static const int SPECTRUM_CHUNK;
	static const int SPECTRUM_N;
	static const float SPECTRUM_MIN_FREQ;
	static const float SPECTRUM_MAX_FREQ;

//	void _cdecl invalidate_peaks(const Range &r);
	bool _peaks_chunk_needs_update(int index);

	bool has_spectrum();

//	void _truncate_peaks(int length);

	// PeakThread
	struct Temp {
		AudioBuffer buffer;
		Array<bytes> peaks;
		bytes spectrogram;
		HorizontallyChunkedImage image;

		void _ensure_peak_size(int level4, int n, bool set_invalid = false);
		int _update_peaks_prepare();
		void _update_peaks_chunk(int index);
		bool _peaks_chunk_needs_update(int index);
	} temp;
};

class PeakDatabase : public obs::Node<VirtualBase> {
public:
	PeakDatabase();
	~PeakDatabase();

	//bytes& get_peaks(AudioBuffer &b, int level4);
	//bytes& get_spectrogram(AudioBuffer &b);

	void invalidate_all();

	std::shared_timed_mutex mtx;

	//owned_array<PeakData> peak_data;
	//base::map<int, owned<PeakData>> peak_data;
	using Map = base::map<int, PeakData*>;
	Map peak_data;
	Map spectrogram_data;

	PeakData& acquire(AudioBuffer &b, AudioViewMode mode);
	void release(PeakData& p);


	void update_peaks_now(AudioBuffer &buf);


	owned<PeakThread> peak_thread;
	std::atomic<float> sample_rate;

	void stop_update();

	void iterate(float sample_rate);
	void iterate_items(Map& map);

	struct Request {
		PeakData *p;
	};

	InterThreadFifoBuffer<Request> requests;
};

#endif /* SRC_VIEW_HELPER_PEAKDATABASE_H_ */
