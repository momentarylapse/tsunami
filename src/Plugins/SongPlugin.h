/*
 * SongPlugin.h
 *
 *  Created on: 24.05.2016
 *      Author: michi
 */

#ifndef SRC_PLUGINS_SONGPLUGIN_H_
#define SRC_PLUGINS_SONGPLUGIN_H_

#include "../lib/base/base.h"

namespace hui{
	class Window;
}
class TsunamiWindow;
class AudioView;
class Song;

class SongPlugin : public VirtualBase
{
public:
	SongPlugin();
	virtual ~SongPlugin();

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual void _cdecl apply(Song *song){}

	hui::Window *win;
	AudioView *view;
};

SongPlugin *CreateSongPlugin(const string &name, TsunamiWindow *win);

#endif /* SRC_PLUGINS_SONGPLUGIN_H_ */
