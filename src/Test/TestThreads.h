/*
 * TestThreads.h
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#ifndef SRC_TEST_TESTTHREADS_H_
#define SRC_TEST_TESTTHREADS_H_

#include "UnitTest.h"

class TestThreads: public UnitTest
{
public:
	TestThreads();

	Array<Test> tests() override;

	static void test_thread_safety();
};

#endif /* SRC_TEST_TESTTHREADS_H_ */

#endif
