/*
 * TestMixer.cpp
 *
 *  Created on: 15.05.2019
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestMixer.h"
#include "../Session.h"
#include "../data/base.h"
#include "../data/Song.h"
#include "../data/Track.h"
#include "../data/TrackLayer.h"
#include "../data/audio/AudioBuffer.h"
#include "../module/audio/SongRenderer.h"
#include "../lib/os/msg.h"
#include <math.h>

const float sqrt2 = (float)sqrt(2.0);

TestMixer::TestMixer() : UnitTest("mixer") {}


Array<UnitTest::Test> TestMixer::tests() {
	Array<Test> list;
	list.add({"mix-stereo-1track-simple", TestMixer::test_mix_stereo_1track_simple});
	list.add({"mix-stereo-1track-volume", TestMixer::test_mix_stereo_1track_volume});
	list.add({"mix-stereo-1track-balance-right", TestMixer::test_mix_stereo_1track_balance_right});
	list.add({"mix-stereo-1track-balance-left", TestMixer::test_mix_stereo_1track_balance_left});
	//list.add({"mix-stereo-2track-simple", TestMixer::test_mix_stereo_2track_simple});
	list.add({"mix-mono-1track-simple", TestMixer::test_mix_mono_1track_simple});
	list.add({"mix-mono-1track-panning-right", TestMixer::test_mix_mono_1track_panning_right});
	list.add({"mix-mono-1track-panning-left", TestMixer::test_mix_mono_1track_panning_left});
	return list;
}

Song* make_example_song(int num_tracks, int channels) {
	Song *s = new Song(Session::GLOBAL, DEFAULT_SAMPLE_RATE);
	for (int ti=0; ti<num_tracks; ti++) {
		Track *t1 = s->add_track((channels == 1) ? SignalType::AUDIO_MONO : SignalType::AUDIO_STEREO);
		AudioBuffer buf;
		t1->layers[0]->get_buffers(buf, Range(0, 4));
		for (int i=0; i<4; i++)
			buf.c[0][i] = 1.0f;
		if (channels > 1)
			for (int i=0; i<4; i+=2)
				buf.c[1][i] = 1.0f;
	}

	//Track *t2 = s->add_track(SignalType::AUDIO);
	return s;
}

void TestMixer::test_mix_stereo_1track_simple() {
	Song *s = make_example_song(1, 2);
	auto* sr = new SongRenderer(s);
	sr->set_range(Range(0,4));
	AudioBuffer buf;
	buf.resize(4);
	sr->read(buf);
	assert_equal(buf, make_buf({1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 0.0f}));
	delete sr;
	delete s;
}

void TestMixer::test_mix_stereo_1track_volume() {
	Song *s = make_example_song(1, 2);
	s->tracks[0]->set_volume(0.5);

	auto* sr = new SongRenderer(s);
	sr->set_range(Range(0,4));
	AudioBuffer buf;
	buf.resize(4);
	sr->read(buf);
	assert_equal(buf, make_buf({0.5f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.0f, 0.5f, 0.0f}));
	delete sr;
	delete s;
}

void TestMixer::test_mix_stereo_1track_balance_right() {
	Song *s = make_example_song(1, 2);
	s->tracks[0]->set_panning(1.0f);

	auto* sr = new SongRenderer(s);
	sr->set_range(Range(0,4));
	AudioBuffer buf;
	buf.resize(4);
	sr->read(buf);
	assert_equal(buf, make_buf({0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 1.0f, 0.0f}));

	delete sr;
	delete s;
}

void TestMixer::test_mix_stereo_1track_balance_left() {
	Song *s = make_example_song(1, 2);
	s->tracks[0]->set_panning(-1.0f);

	auto* sr = new SongRenderer(s);
	sr->set_range(Range(0,4));
	AudioBuffer buf;
	buf.resize(4);
	sr->read(buf);
	assert_equal(buf, make_buf({1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}));

	delete sr;
	delete s;
}

void TestMixer::test_mix_stereo_2track_simple() {
	/*Song *s = make_stereo_song();
	auto* sr = new SongRenderer(s);
	sr->set_range(Range(0,4), false);
	AudioBuffer buf;
	buf.resize(4);
	sr->read(buf);
	assert_equal(buf.c[0], {1.0f, 1.0f, 1.0f, 1.0f});
	assert_equal(buf.c[1], {1.0f, 0.0f, 1.0f, 0.0f});
	delete sr;
	delete s;*/
}

void TestMixer::test_mix_mono_1track_simple() {
	Song *s = make_example_song(1, 1);
	auto* sr = new SongRenderer(s);
	sr->set_range(Range(0,4));
	AudioBuffer buf;
	buf.resize(4);
	sr->read(buf);

	// maximum volume... not 1/sqrt(2)
	assert_equal(buf, make_buf({1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}));
	delete sr;
	delete s;
}

void TestMixer::test_mix_mono_1track_panning_right() {
	Song *s = make_example_song(1, 1);
	s->tracks[0]->set_panning(1.0f);

	auto* sr = new SongRenderer(s);
	sr->set_range(Range(0,4));
	AudioBuffer buf;
	buf.resize(4);
	sr->read(buf);

	// overdriving... by design
	assert_equal(buf, make_buf({0.0f, 0.0f, 0.0f, 0.0f}, {sqrt2, sqrt2, sqrt2, sqrt2}));
	delete sr;
	delete s;
}

void TestMixer::test_mix_mono_1track_panning_left() {
	Song *s = make_example_song(1, 1);
	s->tracks[0]->set_panning(-1.0f);

	auto* sr = new SongRenderer(s);
	sr->set_range(Range(0,4));
	AudioBuffer buf;
	buf.resize(4);
	sr->read(buf);

	// overdriving... by design
	assert_equal(buf, make_buf({sqrt2, sqrt2, sqrt2, sqrt2}, {0.0f, 0.0f, 0.0f, 0.0f}));
	delete sr;
	delete s;
}

#endif
