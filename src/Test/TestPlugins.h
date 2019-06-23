/*
 * TestPlugins.h
 *
 *  Created on: 24.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#ifndef SRC_TEST_TESTPLUGINS_H_
#define SRC_TEST_TESTPLUGINS_H_

#include "UnitTest.h"

enum class ModuleType;

class TestPlugins : public UnitTest
{
public:
	TestPlugins();

	Array<Test> tests() override;

	static void test_compile(ModuleType type, const string &filename);
	static void test_audio_effect(const string &name);
	static void test_audio_source(const string &name);
	static void test_midi_effect(const string &name);
	static void test_midi_source(const string &name);
	static void test_synthesizer(const string &name);
	static void test_tsunami_plugin(const string &name);
};

#endif /* SRC_TEST_TESTPLUGINS_H_ */

#endif
