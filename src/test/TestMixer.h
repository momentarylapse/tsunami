/*
 * TestMixer.h
 *
 *  Created on: 15.05.2019
 *      Author: michi
 */

#ifndef NDEBUG

#ifndef SRC_TEST_TESTMIXER_H_
#define SRC_TEST_TESTMIXER_H_

#include "UnitTest.h"

namespace tsunami {

class TestMixer : public UnitTest {
public:
	TestMixer();

	Array<Test> tests() override;

	static void test_mix_stereo_1track_simple();
	static void test_mix_stereo_1track_volume();
	static void test_mix_stereo_1track_balance_right();
	static void test_mix_stereo_1track_balance_left();
	static void test_mix_stereo_2track_simple();
	static void test_mix_mono_1track_simple();
	static void test_mix_mono_1track_panning_right();
	static void test_mix_mono_1track_panning_left();
};

}

#endif /* SRC_TEST_TESTMIXER_H_ */

#endif
