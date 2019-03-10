/*
 * CaptureConsoleMode.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleMode.h"
#include "../CaptureConsole.h"
#include "../../../Module/SignalChain.h"

CaptureConsoleMode::CaptureConsoleMode(CaptureConsole *_cc)
{
	cc = _cc;
	song = cc->song;
	view = cc->view;
	session = cc->session;
	chain = nullptr;
}

void CaptureConsoleMode::dump()
{
	chain->command(ModuleCommand::ACCUMULATION_STOP, 0);
	chain->command(ModuleCommand::ACCUMULATION_CLEAR, 0);
}

int CaptureConsoleMode::get_sample_count()
{
	return chain->command(ModuleCommand::ACCUMULATION_GET_SIZE, 0);
}

