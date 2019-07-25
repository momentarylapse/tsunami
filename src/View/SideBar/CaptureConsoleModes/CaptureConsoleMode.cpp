/*
 * CaptureConsoleMode.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleMode.h"
#include "../CaptureConsole.h"
#include "../../../Module/SignalChain.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"

CaptureConsoleMode::CaptureConsoleMode(CaptureConsole *_cc)
{
	cc = _cc;
	song = cc->song;
	view = cc->view;
	session = cc->session;
	chain = nullptr;
}

void CaptureConsoleMode::start_sync_before() {
	for (auto &d: view->mode_capture->data)
		d.start_sync_before(view->output_stream);
}

void CaptureConsoleMode::start_sync_after() {
	for (auto &d: view->mode_capture->data)
		d.start_sync_after();
}

void CaptureConsoleMode::end_sync() {
	for (auto &d: view->mode_capture->data)
		d.end_sync();
}

void CaptureConsoleMode::sync() {
	for (auto &d: view->mode_capture->data)
		d.sync(view->output_stream);
}

