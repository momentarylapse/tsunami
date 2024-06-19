/*
 * TestTrackVersion.cpp
 *
 *  Created on: 16.07.2019
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestTrackVersion.h"
#include "../data/base.h"
#include "../data/Song.h"
#include "../data/Track.h"
#include "../data/TrackLayer.h"
#include "../data/CrossFade.h"
#include "../data/audio/AudioBuffer.h"
#include "../Session.h"

namespace tsunami {

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
	Track *t = s->add_track(SignalType::AudioMono);
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
	assert_equal(s->tracks[0]->layers[0]->active_version_ranges(), {Range::to(Range::BEGIN,7), Range::to(10,17), Range::to(20,Range::END)});
}

void TestTrackVersion::test_inactive_version_ranges_base() {
	auto s = create_layer_example_data(true);
	assert_equal(s->tracks[0]->layers[0]->inactive_version_ranges(), {Range::to(7,10), Range::to(17,20)});
}

void TestTrackVersion::test_active_version_ranges_second() {
	auto s = create_layer_example_data(true);
	assert_equal(s->tracks[0]->layers[1]->active_version_ranges(), {Range::to(Range::BEGIN,2), Range::to(5,12), Range::to(15,22), Range::to(25, Range::END)});
}

void TestTrackVersion::test_inactive_version_ranges_second() {
	auto s = create_layer_example_data(true);
	assert_equal(s->tracks[0]->layers[1]->inactive_version_ranges(), {Range::to(2,5), Range::to(12,15), Range::to(22,25)});
}

void TestTrackVersion::test_dominant() {
	auto s = create_layer_example_data(false);
	Track *t = s->tracks[0].get();
	Range r = Range::to(0, 100000);
	t->mark_dominant({t->layers[0].get()}, r);
	assert_equal(t->layers[0]->fades.num, 0);
	assert_equal(t->layers[1]->fades.num, 2);
	t->mark_dominant({t->layers[1].get()}, r);
	assert_equal(t->layers[0]->fades.num, 2);
	assert_equal(t->layers[1]->fades.num, 0);
}

}

#endif
