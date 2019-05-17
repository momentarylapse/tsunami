/*
 * TestRhythm.h
 *
 *  Created on: 16.01.2019
 *      Author: michi
 */

#ifndef NDEBUG

#ifndef SRC_TEST_TESTRHYTHM_H_
#define SRC_TEST_TESTRHYTHM_H_

#include "UnitTest.h"

class TestRhythm : public UnitTest
{
public:
	TestRhythm();

	Array<Test> tests() override;

	static void test_bar_simple_no_partition();
	static void test_bar_simple_partition_2();
	static void test_bar_complex_no_partition();
	static void test_bar_complex_partition_1();
	static void test_bar_complex_partition_2();
};

#endif /* SRC_TEST_TESTRHYTHM_H_ */

#endif
