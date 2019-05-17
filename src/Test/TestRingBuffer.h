/*
 * TestRingBuffer.h
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#ifndef SRC_TEST_TESTRINGBUFFER_H_
#define SRC_TEST_TESTRINGBUFFER_H_

#include "UnitTest.h"

class TestRingBuffer : public UnitTest
{
public:
	TestRingBuffer();

	Array<Test> tests() override;

	static void test_read_write();
	static void test_read_write_wrap();
	static void test_write_too_much_middle();
	static void test_write_too_much_end();
	static void test_read_too_much();
	static void test_read_write_ref();
	static void test_thread_safety();
};

#endif /* SRC_TEST_TESTRINGBUFFER_H_ */

#endif
