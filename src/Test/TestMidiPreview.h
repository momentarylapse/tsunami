/*
 * TestMidiPreview.h
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef SRC_TEST_TESTMIDIPREVIEW_H_
#define SRC_TEST_TESTMIDIPREVIEW_H_

#include "UnitTest.h"

class TestMidiPreview: public UnitTest
{
public:
	TestMidiPreview();

	Array<Test> tests() override;

	static void test_preview();
};

#endif /* SRC_TEST_TESTMIDIPREVIEW_H_ */
