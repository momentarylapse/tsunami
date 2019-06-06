/*
 * TestInterpolator.h
 *
 *  Created on: 06.06.2019
 *      Author: michi
 */

#ifndef SRC_TEST_TESTINTERPOLATOR_H_
#define SRC_TEST_TESTINTERPOLATOR_H_

#include "UnitTest.h"

class TestInterpolator : public UnitTest
{
public:
	TestInterpolator();

	Array<Test> tests() override;

	static void test_linear();
	static void test_linear_short();
	static void test_cubic();
	static void test_fourier();
};

#endif /* SRC_TEST_TESTINTERPOLATOR_H_ */
