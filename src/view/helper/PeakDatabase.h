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
#include <shared_mutex>

class AudioBuffer;
class Range;

#define NUM_PEAK_LEVELS		24
#define PEAK_FACTOR			2


class PeakData {
public:
	AudioBuffer &buffer;
	Array<bytes> peaks;
	bytes spectrum;
	int version;

	PeakData(AudioBuffer &b);

	static const int PEAK_CHUNK_EXP;
	static const int PEAK_CHUNK_SIZE;
	static const int PEAK_OFFSET_EXP;
	static const int PEAK_FINEST_SIZE;
	static const int PEAK_MAGIC_LEVEL2;

	void _cdecl invalidate_peaks(const Range &r);

	void _ensure_peak_size(int level4, int n, bool set_invalid = false);
	int _update_peaks_prepare();
	void _update_peaks_chunk(int index);
	bool _peaks_chunk_needs_update(int index);
	void _truncate_peaks(int length);
};

class PeakDatabase {
public:
	PeakDatabase();

	bytes& get_peaks(AudioBuffer &b, int level4);
	bytes& get_spectrogram(AudioBuffer &b);

	void invalidate_all();

	std::shared_timed_mutex mtx;

	owned_array<PeakData> peak_data;

	PeakData& get_data(AudioBuffer &b);


	void update_peaks_now(AudioBuffer &buf);

};

#endif /* SRC_VIEW_HELPER_PEAKDATABASE_H_ */
