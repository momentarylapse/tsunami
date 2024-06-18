/*
 * Bar.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_DATA_RHYTHM_BAR_H_
#define SRC_DATA_RHYTHM_BAR_H_

#include "../../lib/base/pointer.h"
#include "../Range.h"

class BarCollection;
class Beat;

class BarPattern {
public:
	BarPattern();
	BarPattern(int length, int num_beats, int divisor);
	int length;
	int divisor;
	Array<int> beats;
	void set_pattern(const Array<int> &p);
	int total_sub_beats;

	float bpm(float sample_rate);
	void set_bpm(float bpm, float sample_rate);

	bool is_uniform() const;
	void update_total();
	string pat_str() const;
	bool operator==(const BarPattern &o) const;
	bool operator!=(const BarPattern &o) const;

	enum class Type {
		Bar,
		Pause
	};
};

class Bar : public Sharable<BarPattern> {
public:
	Bar(const BarPattern &b);
	Bar(int length, int num_beats, int divisor);
	void __init__(int length, int num_beats, int divisor);
	bool is_pause();

	string format_beats(bool fancy=true) const;
	string format_bpm(float sample_rate) const;

	Array<Beat> get_beats(int offset, bool include_sub_beats = false, int sub_beat_partition = 1) const;

	const BarPattern &pattern() const;
	Bar *copy() const;


	// filled by BarCollection.getBars()
	int index; // index in the Bar[] array
	int index_text; // index without pause "bars" (well, text = n+1...)
	int offset;
	Range range();
};


// when inserting new bars
enum class BarEditMode {
	Ignore,
	InsertSilence,
	Stretch,
	StretchAndScaleAudio
};



#endif /* SRC_DATA_RHYTHM_BAR_H_ */
