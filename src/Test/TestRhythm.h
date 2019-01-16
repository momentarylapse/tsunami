/*
 * TestRhythm.h
 *
 *  Created on: 16.01.2019
 *      Author: michi
 */

#ifndef SRC_TEST_TESTRHYTHM_H_
#define SRC_TEST_TESTRHYTHM_H_

#include "UnitTest.h"

class TestRhythm : public UnitTest
{
public:
	TestRhythm();

	Array<Test> tests() override;

	static void test_bar_simple();
	static void test_bar_complex();
};

#endif /* SRC_TEST_TESTRHYTHM_H_ */
