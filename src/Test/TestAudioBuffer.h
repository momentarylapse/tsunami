/*
 * TestAudioBuffer.h
 *
 *  Created on: 18.05.2019
 *      Author: michi
 */

#ifndef NDEBUG

#ifndef SRC_TEST_TESTAUDIOBUFFER_H_
#define SRC_TEST_TESTAUDIOBUFFER_H_

#include "UnitTest.h"

class TestAudioBuffer : public UnitTest
{
public:
	TestAudioBuffer();

	Array<Test> tests() override;

	static void test_resize();
	static void test_append();
	static void test_set();
	static void test_scale();
	static void test_ref();
	static void test_ref_write();
};

#endif /* SRC_TEST_TESTAUDIOBUFFER_H_ */

#endif
