/*
 * TsunamiPlugin.h
 *
 *  Created on: 24.05.2016
 *      Author: michi
 */

#ifndef SRC_PLUGINS_TSUNAMIPLUGIN_H_
#define SRC_PLUGINS_TSUNAMIPLUGIN_H_

#include "../Stuff/Observable.h"

class HuiWindow;
class AudioView;
class Song;

class TsunamiPlugin : public Observable
{
public:
	TsunamiPlugin();
	virtual ~TsunamiPlugin();

	static const string MESSAGE_START;
	static const string MESSAGE_STOP;
	static const string MESSAGE_STOP_REQUEST;

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual void _cdecl onStart(){}
	virtual void _cdecl onStop(){}

	void _cdecl start();
	void _cdecl stop();

	void _cdecl stop_request();

	HuiWindow *win;
	AudioView *view;
	Song *song;

	string name;
	bool active;
};

#endif /* SRC_PLUGINS_TSUNAMIPLUGIN_H_ */
