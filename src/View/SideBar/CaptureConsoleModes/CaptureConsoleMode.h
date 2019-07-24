/*
 * CaptureConsoleMode.h
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODE_H_
#define SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODE_H_

#include "../../../lib/base/base.h"

class CaptureConsole;
class Song;
class AudioView;
class Session;
class SignalChain;

class CaptureConsoleMode : public VirtualBase {
public:
	CaptureConsoleMode(CaptureConsole *cc);
	virtual ~CaptureConsoleMode(){};
	virtual void enter() = 0;
	virtual void leave() = 0;
	
	virtual void allow_change_device(bool allow) = 0;

	void start_sync_before();
	void start_sync_after();
	void sync();

	CaptureConsole *cc;
	Session *session;
	Song *song;
	AudioView *view;
	SignalChain *chain;
};


#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODE_H_ */
