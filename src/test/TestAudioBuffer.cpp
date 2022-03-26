/*
 * TestAudioBuffer.cpp
 *
 *  Created on: 18.05.2019
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestAudioBuffer.h"
#include "../data/audio/AudioBuffer.h"

TestAudioBuffer::TestAudioBuffer() : UnitTest("audio-buffer") {
}


Array<UnitTest::Test> TestAudioBuffer::tests() {
	Array<Test> list;
	list.add({"resize", TestAudioBuffer::test_resize});
	list.add({"append", TestAudioBuffer::test_append});
	list.add({"set", TestAudioBuffer::test_set});
	list.add({"scale", TestAudioBuffer::test_scale});
	list.add({"ref", TestAudioBuffer::test_ref});
	list.add({"ref-write", TestAudioBuffer::test_ref_write});
	return list;
}

void TestAudioBuffer::test_resize() {
	AudioBuffer buf;
	buf.resize(4);
	assert_equal(buf, make_buf({0,0,0,0}, {0,0,0,0}));

	buf.resize(2);
	assert_equal(buf, make_buf({0,0}, {0,0}));

	buf = make_buf({1,1,1}, {-1,0,-1});
	buf.resize(2);
	assert_equal(buf, make_buf({1,1}, {-1,0}));
}

void TestAudioBuffer::test_scale() {
	auto buf = make_buf({1,-1}, {0, 2});
	buf.mix_stereo(0.5f);

	assert_equal(buf, make_buf({0.5f, -0.5f}, {0, 1}));
}

void TestAudioBuffer::test_ref() {
	auto buf = make_buf({1,2,3,4,5}, {-1,-2,-3,-4,-5});

	AudioBuffer r = buf.ref(2, 4);
	if (!r.is_ref())
		throw Failure("ref is not a reference...");

	if (&r.c[0][0] != &buf.c[0][2])
		throw Failure("ref has wrong addresses...?");

	assert_equal(r, make_buf({3,4}, {-3,-4}));
}

void TestAudioBuffer::test_append() {
	auto buf = make_buf({1,2,3}, {-1,-2,-3});
	buf.append(make_buf({0,1}, {2,3}));

	assert_equal(buf, make_buf({1,2,3,0,1}, {-1,-2,-3,2,3}));
}

void TestAudioBuffer::test_set() {
	auto buf = make_buf({1,2,3,4}, {-1,-2,-3,-4});

	buf.set(make_buf({10,11},{12,13}), -1, 1.0f);
	assert_equal(buf, make_buf({11,2,3,4}, {13,-2,-3,-4}));

	buf.set(make_buf({10,11},{12,13}), 3, 1.0f);
	assert_equal(buf, make_buf({11,2,3,10}, {13,-2,-3,12}));
}

void TestAudioBuffer::test_ref_write() {
	auto buf = make_buf({1,2,3,4,5}, {-1,-2,-3,-4,-5});

	AudioBuffer r;
	r.set_as_ref(buf, 2, 2);
	assert_equal(r, make_buf({3,4}, {-3,-4}));

	r.c[0][0] = 70;
	r.c[1][1] = 17;
	assert_equal(buf, make_buf({1,2,70,4,5}, {-1,-2,-3,17,-5}));

	buf.ref(1, 2).set_zero();
	assert_equal(buf, make_buf({1,0,70,4,5}, {-1,0,-3,17,-5}));
}

#endif
