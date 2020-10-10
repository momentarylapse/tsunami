/*
 * TestTrackVersion.cpp
 *
 *  Created on: 16.07.2019
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestTrackVersion.h"
#include "../Data/base.h"
#include "../Data/Song.h"
#include "../Data/Track.h"
#include "../Data/TrackLayer.h"
#include "../Data/CrossFade.h"
#include "../Data/Audio/AudioBuffer.h"
#include "../Session.h"

TestTrackVersion::TestTrackVersion() : UnitTest("version") {
}

Array<UnitTest::Test> TestTrackVersion::tests() {
	return {
		{"active-version-ranges-base", TestTrackVersion::test_active_version_ranges_base},
		{"inactive-version-ranges-base", TestTrackVersion::test_inactive_version_ranges_base},
		{"active-version-ranges-second", TestTrackVersion::test_active_version_ranges_second},
		{"inactive-version-ranges-second", TestTrackVersion::test_inactive_version_ranges_second},
		{"dominant", TestTrackVersion::test_dominant}
	};
}

shared<Song> create_layer_example_data(bool with_fades) {
	Song *s = new Song(Session::GLOBAL, DEFAULT_SAMPLE_RATE);
	Track *t = s->add_track(SignalType::AUDIO_MONO);
	auto *l1 = t->layers[0].get();
	auto *l2 = t->add_layer();
	if (with_fades) {
		l1->fades = {{5, CrossFade::OUTWARD, 2}, {10, CrossFade::INWARD, 2}, {15, CrossFade::OUTWARD, 2}, {20, CrossFade::INWARD, 2}};
		l2->fades = {{0, CrossFade::OUTWARD, 2}, {5, CrossFade::INWARD, 2}, {10, CrossFade::OUTWARD, 2}, {15, CrossFade::INWARD, 2}, {20, CrossFade::OUTWARD, 2}, {25, CrossFade::INWARD, 2}};
	}
	return s;
}

void TestTrackVersion::test_active_version_ranges_base() {
	auto s = create_layer_example_data(true);
	assert_equal(s->tracks[0]->layers[0]->active_version_ranges(), {RangeTo(Range::BEGIN,7), RangeTo(10,17), RangeTo(20,Range::END)});
}

void TestTrackVersion::test_inactive_version_ranges_base() {
	auto s = create_layer_example_data(true);
	assert_equal(s->tracks[0]->layers[0]->inactive_version_ranges(), {RangeTo(7,10), RangeTo(17,20)});
}

void TestTrackVersion::test_active_version_ranges_second() {
	auto s = create_layer_example_data(true);
	assert_equal(s->tracks[0]->layers[1]->active_version_ranges(), {RangeTo(Range::BEGIN,2), RangeTo(5,12), RangeTo(15,22), RangeTo(25, Range::END)});
}

void TestTrackVersion::test_inactive_version_ranges_second() {
	auto s = create_layer_example_data(true);
	assert_equal(s->tracks[0]->layers[1]->inactive_version_ranges(), {RangeTo(2,5), RangeTo(12,15), RangeTo(22,25)});
}

void TestTrackVersion::test_dominant() {
	auto s = create_layer_example_data(false);
	Track *t = s->tracks[0].get();
	Range r = RangeTo(0, 100000);
	t->mark_dominant({t->layers[0].get()}, r);
	assert_equal(t->layers[0]->fades.num, 0);
	assert_equal(t->layers[1]->fades.num, 2);
	t->mark_dominant({t->layers[1].get()}, r);
	assert_equal(t->layers[0]->fades.num, 2);
	assert_equal(t->layers[1]->fades.num, 0);
}

#endif
