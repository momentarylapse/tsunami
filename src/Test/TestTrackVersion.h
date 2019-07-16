/*
 * TestTrackVersion.h
 *
 *  Created on: 16.07.2019
 *      Author: michi
 */

#ifndef NDEBUG

#ifndef SRC_TEST_TESTTRACKVERSION_H_
#define SRC_TEST_TESTTRACKVERSION_H_

#include "UnitTest.h"

class TestTrackVersion : public UnitTest {
public:
	TestTrackVersion();

	Array<Test> tests() override;

	static void test_active_version_ranges_base();
	static void test_inactive_version_ranges_base();
	static void test_active_version_ranges_second();
	static void test_inactive_version_ranges_second();
};

#endif /* SRC_TEST_TESTTRACKVERSION_H_ */

#endif
