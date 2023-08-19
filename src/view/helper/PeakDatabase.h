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
#include "../../data/audio/AudioBuffer.h"
#include <atomic>
#include <shared_mutex>

class AudioBuffer;
class Range;
class PeakThread;
class Song;

#define NUM_PEAK_LEVELS		24
#define PEAK_FACTOR			2


class PeakData {
public:
	std::shared_timed_mutex mtx;
	AudioBuffer* buffer;
	Array<bytes> peaks;
	int version;

	enum class State {
		OK,
		OUT_OF_SYNC,
		UPDATE_REQUESTED,
		UPDATE_FINISHED
	};
	std::atomic<State> state;

	PeakData(AudioBuffer& b);

	static const int PEAK_CHUNK_EXP;
	static const int PEAK_CHUNK_SIZE;
	static const int PEAK_OFFSET_EXP;
	static const int PEAK_FINEST_SIZE;
	static const int PEAK_MAGIC_LEVEL2;

//	void _cdecl invalidate_peaks(const Range &r);
	bool _peaks_chunk_needs_update(int index);

//	void _truncate_peaks(int length);

	// PeakThread
	struct Temp {
		AudioBuffer buffer;
		Array<bytes> peaks;

		void _ensure_peak_size(int level4, int n, bool set_invalid = false);
		int _update_peaks_prepare();
		void _update_peaks_chunk(int index);
		bool _peaks_chunk_needs_update(int index);
	} temp;
};

class SpectrogramData {
public:
	std::shared_timed_mutex mtx;
	AudioBuffer* buffer;
	bytes spectrogram;
	int version;

	enum class State {
		OK,
		OUT_OF_SYNC,
		UPDATE_REQUESTED,
		UPDATE_FINISHED
	};
	State state;

	SpectrogramData(AudioBuffer& b);

	// PeakThread
	struct Temp {
		AudioBuffer buffer;
		bytes spectrogram;
	} temp;
};

template<class T>
class InterThreadFifoBuffer {
	Array<T> buffer;
	std::shared_timed_mutex mtx;

public:
	void add(const T& t) {
		mtx.lock();
		buffer.add(t);
		mtx.unlock();
	}
	bool has_data() {
		mtx.lock();
		bool h = buffer.num > 0;
		mtx.unlock();
		return h;
	}
	T pop() {
		mtx.lock();
		T t;
		if (buffer.num > 0) {
			t = buffer[0];
			buffer.erase(0);
		}
		mtx.unlock();
		return t;
	}
};

class PeakDatabase : public obs::Node<VirtualBase> {
public:
	PeakDatabase(Song *song);
	~PeakDatabase();

	//bytes& get_peaks(AudioBuffer &b, int level4);
	//bytes& get_spectrogram(AudioBuffer &b);

	void invalidate_all();

	std::shared_timed_mutex mtx;

	//owned_array<PeakData> peak_data;
	//base::map<int, owned<PeakData>> peak_data;
	base::map<int, PeakData*> peak_data;
	base::map<int, SpectrogramData*> spectrogram_data;

	PeakData& acquire_peaks(AudioBuffer &b);
	SpectrogramData& acquire_spectrogram(AudioBuffer &b);
	void release(PeakData& p);
	void release(SpectrogramData& p);


	void update_peaks_now(AudioBuffer &buf);


	owned<PeakThread> peak_thread;

	void stop_update();

	void iterate();

	struct Request {
		PeakData *p;
		SpectrogramData *s;
	};

	InterThreadFifoBuffer<Request> requests;
};

#endif /* SRC_VIEW_HELPER_PEAKDATABASE_H_ */
