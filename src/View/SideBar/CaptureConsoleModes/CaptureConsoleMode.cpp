/*
 * CaptureConsoleMode.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleMode.h"
#include "../CaptureConsole.h"

CaptureConsoleMode::CaptureConsoleMode(CaptureConsole *_cc)
{
	cc = _cc;
	song = cc->song;
	view = cc->view;
	session = cc->session;
}

