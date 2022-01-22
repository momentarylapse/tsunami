/*
 * TestStreams.h
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#ifndef SRC_TEST_TESTSTREAMS_H_
#define SRC_TEST_TESTSTREAMS_H_

#include "UnitTest.h"

class TestStreams : public UnitTest {
public:
	TestStreams();

	Array<Test> tests() override;

	static void test_output_stream();
	static void test_input_stream();
};

#endif /* SRC_TEST_TESTSTREAMS_H_ */

#endif
