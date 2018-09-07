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
	virtual void enter_parent(){};
	virtual void leave_parent(){};
	virtual bool insert() = 0;
	virtual void pause() = 0;
	virtual void dump() = 0;
	virtual int get_sample_count() = 0;
	virtual bool is_capturing() = 0;

	CaptureConsole *cc;
	Session *session;
	Song *song;
	AudioView *view;
};


#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODE_H_ */
