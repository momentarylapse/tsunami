/*
 * Bar.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_DATA_RHYTHM_BAR_H_
#define SRC_DATA_RHYTHM_BAR_H_

#include "../Range.h"

#ifdef IGNORE
#undef IGNORE
#endif

class BarCollection;
class Beat;

class BarPattern
{
public:
	BarPattern();
	BarPattern(int length, int num_beats, int num_sub_beats);
	int length;
	int num_beats;
	int num_sub_beats;
	Array<int> pattern;
	void set_pattern(const Array<int> &p);
	int total_sub_beats;
	bool is_uniform() const;
	void update_total();
	string pat_str() const;
	bool operator==(const BarPattern &o) const;
	bool operator!=(const BarPattern &o) const;

	enum Type{
		BAR,
		PAUSE
	};
};

class Bar : public BarPattern
{
public:
	Bar(){}
	Bar(const BarPattern &b);
	Bar(int length, int num_beats, int num_sub_beats);
	bool is_pause();
	float bpm(float sample_rate);

	string format_beats(bool fancy=true) const;
	string format_bpm(float sample_rate) const;

	Array<Beat> get_beats(int offset, bool include_sub_beats = false, int sub_beat_partition = 1) const;


	// filled by BarCollection.getBars()
	int index; // index in the Bar[] array
	int index_text; // index without pause "bars" (well, text = n+1...)
	int offset;
	Range range();

	// when inserting new bars
	enum EditMode{
		IGNORE,
		INSERT_SILENCE,
		STRETCH,
		STRETCH_AND_SCALE_AUDIO
	};
};



#endif /* SRC_DATA_RHYTHM_BAR_H_ */
