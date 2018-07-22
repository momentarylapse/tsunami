/*
 * TestAudioBuffer.h
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef SRC_TEST_TESTAUDIOBUFFER_H_
#define SRC_TEST_TESTAUDIOBUFFER_H_

#include "UnitTest.h"

class TestAudioBuffer : public UnitTest
{
public:
	TestAudioBuffer();

	Array<Test> tests() override;

	static void test_ring_buffer_thread_safety();
};

#endif /* SRC_TEST_TESTAUDIOBUFFER_H_ */
