/*
 * TsunamiPlugin.h
 *
 *  Created on: 24.05.2016
 *      Author: michi
 */

#ifndef SRC_PLUGINS_TSUNAMIPLUGIN_H_
#define SRC_PLUGINS_TSUNAMIPLUGIN_H_

#include "../Module/Module.h"

class Painter;

class TsunamiPlugin : public Module {
public:
	TsunamiPlugin();
	virtual ~TsunamiPlugin();

	static const string MESSAGE_STOP_REQUEST;

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual void _cdecl on_start() {};
	virtual void _cdecl on_stop() {};
	virtual void _cdecl on_draw_post(Painter *p) {};

	void _cdecl stop_request();

	Array<string> args;
};

TsunamiPlugin *CreateTsunamiPlugin(Session *session, const string &name);

#endif /* SRC_PLUGINS_TSUNAMIPLUGIN_H_ */
