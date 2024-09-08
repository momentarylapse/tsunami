/*
 * TestRhythm.cpp
 *
 *  Created on: 16.01.2019
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestRhythm.h"
#include "../data/rhythm/Bar.h"
#include "../data/rhythm/Beat.h"
#include "../lib/os/msg.h"

namespace tsunami {

TestRhythm::TestRhythm() : UnitTest("rhythm") {
}

Array<UnitTest::Test> TestRhythm::tests() {
	Array<Test> list;
	list.add({"bar-simple-no-partition", TestRhythm::test_bar_simple_no_partition});
	list.add({"bar-simple-partition-2", TestRhythm::test_bar_simple_partition_2});
	list.add({"bar-complex-no-partition", TestRhythm::test_bar_complex_no_partition});
	list.add({"bar-complex-partition-1", TestRhythm::test_bar_complex_partition_1});
	list.add({"bar-complex-partition-2", TestRhythm::test_bar_complex_partition_2});
	return list;
}

static Array<int> beat_offsets(const Array<Beat> &beats) {
	Array<int> off;
	for (Beat &b: beats)
		off.add(b.range.offset);
	return off;
}

void TestRhythm::test_bar_simple_no_partition() {
	Bar bar = {1000, 4, 1};

	auto beats = bar.get_beats(0);
	assert_equal(beat_offsets(beats), {0, 250, 500, 750});
}

void TestRhythm::test_bar_simple_partition_2() {
	Bar bar = {1000, 4, 1};

	auto beats = bar.get_beats(0, 2);
	assert_equal(beat_offsets(beats), {0, 125, 250, 375, 500, 625, 750, 875});
}

void TestRhythm::test_bar_complex_no_partition() {
	Bar bar = {100, 4, 2};
	bar.set_pattern({3,3,2,2});

	auto beats = bar.get_beats(0);
	assert_equal(beat_offsets(beats), {0, 30, 60, 80});
}

void TestRhythm::test_bar_complex_partition_1() {
	Bar bar = {100, 4, 2};
	bar.set_pattern({3,3,2,2});

	auto beats = bar.get_beats(0, 1);
	assert_equal(beat_offsets(beats), {0, 10, 20, 30, 40, 50, 60, 70, 80, 90});
}
void TestRhythm::test_bar_complex_partition_2() {
	Bar bar = {100, 4, 2};
	bar.set_pattern({3,3,2,2});

	auto beats = bar.get_beats(0, 2);
	assert_equal(beat_offsets(beats), {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95});
}

}

#endif
