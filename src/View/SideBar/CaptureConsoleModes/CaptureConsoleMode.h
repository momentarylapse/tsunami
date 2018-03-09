/*
 * CaptureConsoleMode.h
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODE_H_
#define SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODE_H_

class CaptureConsole;
class Song;
class AudioView;
class Session;

class CaptureConsoleMode
{
public:
	CaptureConsoleMode(CaptureConsole *cc);
	virtual ~CaptureConsoleMode(){};
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void enter() = 0;
	virtual void leave() = 0;
	virtual void enterParent(){};
	virtual void leaveParent(){};
	virtual bool insert() = 0;
	virtual void pause() = 0;
	virtual void dump() = 0;
	virtual int getSampleCount() = 0;
	virtual bool isCapturing() = 0;

	CaptureConsole *cc;
	Session *session;
	Song *song;
	AudioView *view;
};


#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODE_H_ */
