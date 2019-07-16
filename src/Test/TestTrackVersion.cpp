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
		{"inactive-version-ranges-second", TestTrackVersion::test_inactive_version_ranges_second}
	};
}

Song *create_layer_example_data() {
	Song *s = new Song(Session::GLOBAL, DEFAULT_SAMPLE_RATE);
	Track *t = s->add_track(SignalType::AUDIO_MONO);
	auto *l1 = t->layers[0];
	auto *l2 = t->add_layer();
	l1->fades = {{5, CrossFade::OUTWARD, 2}, {10, CrossFade::INWARD, 2}, {15, CrossFade::OUTWARD, 2}, {20, CrossFade::INWARD, 2}};
	l2->fades = {{5, CrossFade::INWARD, 2}, {10, CrossFade::OUTWARD, 2}, {15, CrossFade::INWARD, 2}, {20, CrossFade::OUTWARD, 2}};
	return s;
}

void TestTrackVersion::test_active_version_ranges_base() {
	Song *s = create_layer_example_data();
	assert_equal(s->tracks[0]->layers[0]->active_version_ranges(), {RangeTo(Range::BEGIN,7), RangeTo(10,17), RangeTo(20,Range::END)});
	delete s;
}

void TestTrackVersion::test_inactive_version_ranges_base() {
	Song *s = create_layer_example_data();
	assert_equal(s->tracks[0]->layers[0]->inactive_version_ranges(), {RangeTo(7,10), RangeTo(17,20)});
	delete s;
}

void TestTrackVersion::test_active_version_ranges_second() {
	Song *s = create_layer_example_data();
	assert_equal(s->tracks[0]->layers[1]->active_version_ranges(), {RangeTo(5,12), RangeTo(15,22)});
	delete s;
}

void TestTrackVersion::test_inactive_version_ranges_second() {
	Song *s = create_layer_example_data();
	assert_equal(s->tracks[0]->layers[1]->inactive_version_ranges(), {RangeTo(Range::BEGIN,5), RangeTo(12,15), RangeTo(22,Range::END)});
	delete s;
}

#endif
