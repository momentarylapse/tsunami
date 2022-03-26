/*
 * TestSignalChain.h
 *
 *  Created on: 22 Jan 2022
 *      Author: michi
 */

#ifndef NDEBUG

#ifndef SRC_TEST_TESTSIGNALCHAIN_H_
#define SRC_TEST_TESTSIGNALCHAIN_H_

#include "UnitTest.h"

class TestSignalChain : public UnitTest {
public:
	TestSignalChain();

	Array<Test> tests() override;

	static void test_synth();
};

#endif /* SRC_TEST_TESTSIGNALCHAIN_H_ */

#endif
