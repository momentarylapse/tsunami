/*
 * TestSignalChain.cpp
 *
 *  Created on: 22 Jan 2022
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestSignalChain.h"
#include "../module/synthesizer/DebugSynthesizer.h"

namespace tsunami {

TestSignalChain::TestSignalChain() : UnitTest("signal-chain") {
}

Array<UnitTest::Test> TestSignalChain::tests() {
	Array<Test> list;
	list.add({"synth", TestSignalChain::test_synth});
	return list;
}

void TestSignalChain::test_synth() {
	DebugSynthesizer synth;
}

}

#endif
