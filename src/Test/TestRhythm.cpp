/*
 * TestRhythm.cpp
 *
 *  Created on: 16.01.2019
 *      Author: michi
 */

#include "TestRhythm.h"
#include "../Data/Rhythm/Bar.h"
#include "../Data/Rhythm/Beat.h"
#include "../lib/file/msg.h"

TestRhythm::TestRhythm() : UnitTest("rhythm")
{
}

Array<UnitTest::Test> TestRhythm::tests()
{
	Array<Test> list;
	list.add(Test("bar-simple", TestRhythm::test_bar_simple));
	list.add(Test("bar-complex", TestRhythm::test_bar_complex));
	return list;
}

static Array<int> beat_offsets(const Array<Beat> &beats)
{
	Array<int> off;
	for (Beat &b: beats)
		off.add(b.range.offset);
	return off;

}

void TestRhythm::test_bar_simple()
{
	Bar bar = Bar(1000, 4, 1);

	auto beats = bar.get_beats(0);
	if (beat_offsets(beats) != Array<int>{0, 250, 500, 750})
		throw Failure("4/4, no subs");

	beats = bar.get_beats(0, true, 2);
	if (beat_offsets(beats) != Array<int>{0, 125, 250, 375, 500, 625, 750, 875})
		throw Failure("4/4, partition 2");
}

void TestRhythm::test_bar_complex()
{
	Bar bar = Bar(100, 4, 2);
	bar.set_pattern({3,3,2,2});

	auto beats = bar.get_beats(0);
	if (beat_offsets(beats) != Array<int>{0, 30, 60, 80})
		throw Failure("4/4, no subs");

	beats = bar.get_beats(0, true, 1);
	if (beat_offsets(beats) != Array<int>{0, 10, 20, 30, 40, 50, 60, 70, 80, 90})
		throw Failure("4/4, partition 1");

	beats = bar.get_beats(0, true, 2);
	if (beat_offsets(beats) != Array<int>{0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95})
		throw Failure("4/4, partition 2");
}
