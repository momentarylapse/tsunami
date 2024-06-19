/*
 * TestPointer.h
 *
 *  Created on: Oct 4, 2020
 *      Author: michi
 */

#ifndef NDEBUG

#ifndef SRC_TEST_TESTPOINTER_H_
#define SRC_TEST_TESTPOINTER_H_


#include "UnitTest.h"

namespace tsunami {

class TestPointer : public UnitTest {
public:
	TestPointer();

	Array<Test> tests() override;

	static void test_owned();
	static void test_shared();
	static void test_owned_array();
	static void test_shared_array();
	static void test_shared_array_set_shared_array();
	static void test_shared_array_set_array();
};

}

#endif /* SRC_TEST_TESTPOINTER_H_ */

#endif
