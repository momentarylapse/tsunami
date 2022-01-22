/*
 * TestMidiPreview.h
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#ifndef SRC_TEST_TESTMIDIPREVIEW_H_
#define SRC_TEST_TESTMIDIPREVIEW_H_

#include "UnitTest.h"

class TestMidiPreview: public UnitTest {
public:
	TestMidiPreview();

	Array<Test> tests() override;

	static void test_preview();
	static void test_preview_source();
};

#endif /* SRC_TEST_TESTMIDIPREVIEW_H_ */

#endif
